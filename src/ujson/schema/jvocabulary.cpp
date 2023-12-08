/*
 * Copyright (C) 2022,2023 Dan Arrhenius <dan@ultramarin.se>
 *
 * This file is part of ujson.
 *
 * ujson is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <ujson/schema/jvocabulary.hpp>
#include <ujson/jschema.hpp>
#include <memory>
#include <regex>
#include <string_view>


#define DEVEL_DEBUG 0

#if (DEVEL_DEBUG)
#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
#define PREFIX __FUNCTION__ << ',' << __LINE__ << ": "
#endif


namespace ujson::schema {


    static const std::regex uri_regex ("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?", std::regex::awk);


    // Functor used when we create a std::shared_ptr<validation_context>
    // but we don't want it to delete the pointer when it's done.
    struct nulldeleter {
        void operator() (validation_context*) {
            ; // Do nothing here
        }
    };


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvocabulary::jvocabulary (jschema& root_schema_arg)
        : root_schema (root_schema_arg)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvocabulary::jvocabulary (jschema& root_schema_arg, const std::string& id_arg)
        : vocabulary_id (id_arg),
          root_schema (root_schema_arg)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    const std::string& jvocabulary::id () const
    {
        return vocabulary_id;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary::load_subschema (jvalue& sub_schema)
    {
        root_schema.load (sub_schema);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary::validate_subschema (validation_context& ctx,
                                          jvalue& sub_schema,
                                          jvalue& instance,
                                          const bool create_subcontext,
                                          const bool ignore_annotations,
                                          const bool invalidate_parent_if_invalid)
    {
        std::shared_ptr<validation_context> sub_ctx;
        if (create_subcontext)
            sub_ctx.reset (new validation_context(ctx));
        else
            sub_ctx.reset (&ctx, nulldeleter()); // Don't actually delete anything

        // Validate the subschema
        bool is_valid = root_schema.validate (*sub_ctx, sub_schema, instance);
        sub_ctx->set_valid (is_valid);

        if (sub_ctx->parent) {
            validation_context* parent_ctx = sub_ctx->parent;

            if (ignore_annotations == false)
                parent_ctx->add_output_unit (std::move(sub_ctx->output_unit));

            if (is_valid) {
                if (ignore_annotations == false)
                    parent_ctx->collect_annotations (*sub_ctx);
            }else{
                if (invalidate_parent_if_invalid)
                    parent_ctx->set_valid (false);
            }
        }

        return is_valid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary::push_load_ctx_path (jvalue& load_ctx, const std::string& entry)
    {
        if (!load_ctx["absolute_path"].array().empty())
            load_ctx["absolute_path"].array().back().append (entry);
        load_ctx["validation_path"].append (entry);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary::pop_load_ctx_path (jvalue& load_ctx)
    {
        load_ctx["validation_path"].array().pop_back ();
        if (!load_ctx["absolute_path"].array().empty())
            load_ctx["absolute_path"].array().back().array().pop_back ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::map<std::string, std::string>& jvocabulary::id_aliases ()
    {
        return root_schema.id_alias;
    }


    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    static inline char hex_to_ch (char hex)
    {
        if (hex <= '9')
            return hex - '0';
        else if (hex <= 'F')
            return hex - 'A';
        else
            return hex - 'a';
    }


    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    static std::string uri_unescape (const std::string& uri)
    {
        std::string retval;

        for (unsigned i=0; i<uri.size(); ++i) {
            if (uri[i] == '%'  &&  i<(uri.size()-2) && isxdigit(uri[i+1]) && isxdigit(uri[i+2])) {
                char ch = hex_to_ch (uri[++i]);
                ch <<= 4;
                ch |= hex_to_ch (uri[++i]);
                retval.push_back (ch);
            }else{
                retval.push_back (uri[i]);
            }
        }
        return retval;
    }


    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    int jvocabulary::split_uri (const std::string& full_uri,
                                std::string& uri,
                                std::string& fragment)
    {
        std::cmatch cm;

        if (! std::regex_search(full_uri.c_str(), cm, uri_regex)) {
            uri = "";
            fragment = "";
            return -1;
        }

        const char* first = nullptr;
        if (cm[2].length())      // scheme
            first = cm[2].first;
        else if (cm[4].length()) // host
            first = cm[4].first;
        else if (cm[5].length()) // path
            first = cm[5].first;
        else if (cm[7].length()) // query
            first = cm[7].first;

        if (first)
            uri = std::string (first, (cm[8].first - first));
        else
            uri = "";

        fragment = uri_unescape (cm[9]);

        return 0;
    }


    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    std::string jvocabulary::resolve_id (const std::string& base_uri_arg,
                                         const std::string& uri_arg,
                                         std::string& err_msg,
                                         bool allow_fragment)
    {
        // Parse uri_arg
        //
        std::cmatch cm_uri;
        if (! std::regex_search(uri_arg.c_str(), cm_uri, uri_regex)) {
            err_msg = "Invalid '$id', invalid URI.";
            return "";
        }

        // 8.2.1. "$id" MUST NOT contain a non-empty fragment, and SHOULD
        //        NOT contain an empty fragment.
        if (!allow_fragment && cm_uri[8].length()) {
            err_msg = "Invalid '$id', fragment not allowed.";
            return "";
        }

        if (cm_uri[2].length() != 0) {
            // uri_arg is an absolute URI
            return uri_arg;
        }
        if (base_uri_arg.empty()) {
            // base_uri_arg empty, and uri_arg not an absolute URI.
            // 8.2.1.1 The root schema of a JSON Schema document SHOULD contain an "$id"
            //         keyword with an absolute-URI
            err_msg = "Invalid '$id', not an absolute URI.";
            return "";
        }

        // Parse base URI, should never fail
        std::cmatch cm_base_uri;
        if (! std::regex_search(base_uri_arg.c_str(), cm_base_uri, uri_regex)) {
            err_msg = "Invalid '$id', invalid base URI."; // Should never happen
            return "";
        }

        // Make resulting URI
        //
        std::string result_uri (cm_base_uri[2].first, (cm_base_uri[5].first - cm_base_uri[2].first));

        if (cm_uri[5].length()) {
            if (cm_uri[5].first[0] == '/') {
                // The path in URI is an absolute path, put in in Base URI
                result_uri.append (std::string_view(cm_uri[5].first, cm_uri[5].length()));
            }else{
                if (cm_base_uri[5].length() == 0) {
                    // Base URI has no path, set it to same as the path in URI
                    result_uri.push_back ('/');
                    result_uri.append (std::string_view(cm_uri[5].first, cm_uri[5].length()));
                }else{
                    // Base URI has a path, replace tha last path element to same as the path in URI
                    const char* pos = cm_base_uri[5].first + cm_base_uri[5].length();
                    while (*--pos != '/'  &&  pos != cm_base_uri[5].first)
                        ;
                    if (*pos == '/')
                        result_uri.append (std::string_view(cm_base_uri[5].first, pos - cm_base_uri[5].first + 1));
                    result_uri.append (std::string_view(cm_uri[5].first, cm_uri[5].length()));
                }
            }
            if (cm_uri[7].length()) // Query
                result_uri.append (std::string_view(cm_uri[6].first, cm_uri[6].length()));
        }else{
            if (cm_base_uri[5].length())
                result_uri.append (std::string_view(cm_base_uri[5].first, cm_base_uri[5].length()));
            if (cm_base_uri[7].length()) // Query
                result_uri.append (std::string_view(cm_base_uri[6].first, cm_base_uri[6].length()));
        }

        if (cm_uri[8].length() != 0) // Fragment
            result_uri.append (std::string_view(cm_uri[8].first, cm_uri[8].length()));

        return result_uri;
    }


}
