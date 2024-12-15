/*
 * Copyright (C) 2022-2024 Dan Arrhenius <dan@ultramarin.se>
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
#include <ujson/schema/jvocabulary_core.hpp>
#include <ujson/jschema.hpp>
#include <ujson/utils.hpp>
#include <regex>


#define DEVEL_DEBUG 0

#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#if (DEVEL_DEBUG)
#define PREFIX __FUNCTION__ << ',' << __LINE__ << ": "
#endif


namespace ujson::schema {


#define get_tuple_base_uri(tuple) std::get<0>(tuple)
#define get_tuple_abs_path(tuple) std::get<1>(tuple)
#define get_tuple_schema(tuple) std::get<2>(tuple).get()



    const jvocabulary_core::keyword_loaders_t jvocabulary_core::keyword_loaders = {{
            // Any instance type
            {"$schema", &jvocabulary_core::load_schema},
            //{"$id", &jvocabulary_core::load_id},
            {"$defs", &jvocabulary_core::load_defs},
            {"$anchor", &jvocabulary_core::load_anchor},
            {"$dynamicAnchor", &jvocabulary_core::load_dynamicAnchor},
            //{"$comment", &jvocabulary_core::load_comment},
        }};



    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvocabulary_core::jvocabulary_core (jschema& schema_arg)
        : jvocabulary (schema_arg, "https://json-schema.org/draft/2020-12/meta/core")
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::print_maps (std::ostream& out)
    {
        out << endl;
        out << "ids:" << endl;
        for (auto entry : ids) {
            out << entry.first << endl;
            //out << entry.first << ": " << entry.second.get().describe() << endl;
        }
        out << endl;

        if (!id_aliases().empty()) {
            out << "id aliases:" << endl;
            for (auto entry : id_aliases()) {
                out << entry.first << ": " << entry.second << endl;
            }
            out << endl;
        }

        out << "anchors:" << endl;
        for (auto entry : anchors) {
            out << entry.first << ", " << get_tuple_base_uri(entry.second) << ", " << get_tuple_abs_path(entry.second) << endl;
            //out << get_tuple_schema(entry.second).describe(true) << endl;
        }
        out << endl;

        out << "dyn_anchors:" << endl;
        for (auto entry : dyn_anchors) {
            out << entry.first << ", " << get_tuple_base_uri(entry.second) << ", " << get_tuple_abs_path(entry.second) << endl;
            //out << get_tuple_schema(entry.second).describe(true) << endl;
        }
        out << endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::set_invalid_ref_cb (invalid_ref_cb_t cb)
    {
        invalid_ref_cb = cb;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::load (jvalue& schema, jvalue& load_ctx)
    {
        if (schema.type() == j_bool)
            return;

        // Check "$id" first
        auto& id_value = schema.get ("$id");
        if (id_value.valid())
            load_id (schema, id_value, load_ctx);

        for (auto& member : schema.obj()) {
            const std::string& schema_keyword = member.first;
            ujson::jvalue& schema_value = member.second;

            // Find a validator for this keyword
            auto entry = keyword_loaders.find (schema_keyword);
            if (entry != keyword_loaders.end()) {
                auto keyword_loader = entry->second;
                (this->*keyword_loader) (schema, schema_value, load_ctx);
            }
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_core::validate (validation_context& ctx,
                                     jvalue& schema,
                                     jvalue& instance,
                                     const bool quit_on_first_error)
    {
        bool is_valid = true;

        auto& id_value = schema.get ("$id");
        if (id_value.valid()) {
            validate_id (ctx, id_value, instance);
        }else{
            if (ctx.base_uri.empty())
                ctx.base_uri = jschema::default_base_uri;
        }

        auto& ref_value = schema.get ("$ref");
        if (ref_value.valid()) {
            ctx.push_schema_path ("$ref");
            if (!validate_ref (ctx, schema, ref_value, instance, quit_on_first_error))
                is_valid = false;
            ctx.pop_schema_path ();
        }

        auto& dynref_value = schema.get ("$dynamicRef");
        if (dynref_value.valid()) {
            ctx.push_schema_path ("$dynamicRef");
            if (!validate_dynamicRef (ctx, schema, dynref_value, instance, quit_on_first_error))
                is_valid = false;
            ctx.pop_schema_path ();
        }

        /*
        auto& comment_value = schema.get ("$comment");
        if (comment_value.valid()) {
            ctx.push_schema_path ("$comment");
            validate_comment (ctx, schema, comment_value, instance);
            ctx.pop_schema_path ();
        }
        */

        return is_valid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_core::validate_id (validation_context& ctx, jvalue& schema_value, jvalue& instance)
    {
        ctx.push_schema_path ("$id");

        std::string uri_error;
        if (ctx.base_uri.empty()) {
            ctx.base_uri = resolve_id ("", schema_value.str(), uri_error);
        }else{
            if (ctx.parent) {
                std::string& base_uri = ctx.parent->base_uri;
                ctx.base_uri = resolve_id (base_uri, schema_value.str(), uri_error);
            }else{
                std::string& base_uri = ctx.base_uri;
                ctx.base_uri = resolve_id (base_uri, schema_value.str(), uri_error);
            }
        }
        ctx.abs_keyword_path.clear (); // New abs_keyword_path relative to the new $id
        ctx.pop_schema_path ();

        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue* jvocabulary_core::resolve_ref (validation_context& ctx,
                                           jvalue& schema,
                                           std::string& ref)
    {
        try {
            std::string err_msg;
            std::string uri = resolve_id (ctx.base_uri, ref, err_msg, true);

            if (uri.empty())
                return nullptr;

            std::string uri_without_fragment;
            std::string fragment;
            split_uri (uri, uri_without_fragment, fragment);

            // Check anchors
            //
            auto anchor_entry = anchors.find (uri);
            if (anchor_entry != anchors.end()) {
                ctx.base_uri = get_tuple_base_uri (anchor_entry->second);
                ctx.abs_keyword_path = get_tuple_abs_path (anchor_entry->second);
                return &(get_tuple_schema(anchor_entry->second));
            }

            // Check dynamic anchors
            //
            auto dyn_anchor_entry = dyn_anchors.find (uri);
            if (dyn_anchor_entry != dyn_anchors.end()) {
                ctx.base_uri = get_tuple_base_uri (dyn_anchor_entry->second);
                ctx.abs_keyword_path = get_tuple_abs_path (dyn_anchor_entry->second);
                return &(get_tuple_schema(dyn_anchor_entry->second));
            }

            // Check pointer
            //
            ids_iter_t id_entry;
            // First check if this is an alias for a "real" id
            auto alias_entry = id_aliases().find (uri_without_fragment);
            if (alias_entry != id_aliases().end())
                id_entry = ids.find (alias_entry->second);
            else
                id_entry = ids.find (uri_without_fragment);
            if (id_entry == ids.end())
                return nullptr;

            auto& subschema = find_jvalue (id_entry->second.get(), fragment);
            if (subschema.invalid())
                return nullptr;

            // Search fragment path backwards for $id
            jpointer tmp_ptr (fragment);
            jpointer result_ptr;
            size_t tmp_ptr_size = tmp_ptr.size ();
            if (tmp_ptr_size > 1) {
                while (tmp_ptr_size--) {
                    auto& jval = find_jvalue (id_entry->second.get(), tmp_ptr);
                    if (jval.type() == j_object) {
                        auto& id = jval.get ("$id");
                        if (id.type() == j_string) {
                            ctx.base_uri = resolve_id (uri_without_fragment, id.str(), err_msg);
                            ctx.abs_keyword_path = result_ptr;
                            return &subschema;
                        }
                    }
                    result_ptr.push_front (tmp_ptr.back());
                    tmp_ptr.pop_back ();
                }
            }

            ctx.base_uri = uri_without_fragment;
            ctx.abs_keyword_path = fragment;
            return &subschema;
        }
        catch (...) {
            return nullptr;
        }

        return nullptr;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_core::validate_ref (validation_context& ctx, jvalue& schema,
                                         jvalue& schema_value, jvalue& instance,
                                         const bool quit_on_first_error)
    {
        jvalue* target_schema = nullptr;
        bool new_ref_schema_loaded = false;
        bool valid = true;

        do {
            validation_context sub_ctx (ctx);

            if (schema_value.type() == j_string)
                target_schema = resolve_ref (sub_ctx,
                                             schema,
                                             schema_value.str());
            if (target_schema) {
                new_ref_schema_loaded = false;

                if (target_schema == &schema) {
                    // Simple recursion check
                    throw invalid_schema (ctx.base_uri, ctx.abs_keyword_path.str(),
                                          "Keyword '$ref' referring to same schema.");
                }else{
                    valid = validate_subschema (sub_ctx, *target_schema, instance,
                                                quit_on_first_error, false, false, true);
                }
            }else{
                if (invalid_ref_cb && !new_ref_schema_loaded) {
                    new_ref_schema_loaded = invalid_ref_cb (root_schema,
                                                            ctx.base_uri,
                                                            schema_value.str());
                    if (!new_ref_schema_loaded) {
                        sub_ctx.set_error ("Invalid reference");
                        ctx.set_valid (false);
                        ctx.add_output_unit (std::move(sub_ctx.output_unit));
                        valid = false;
                    }
                }else{
                    throw invalid_schema (ctx.base_uri, ctx.abs_keyword_path.str(),
                                          "Invalid reference for keyword '$ref'.");
                }
            }
        }while (new_ref_schema_loaded);

        return valid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue* jvocabulary_core::resolve_dynref (validation_context& ctx,
                                              jvalue& schema,
                                              std::string& dynref)
    {
        jvalue* retval = nullptr;
        std::string err_msg;

        std::string uri = resolve_id (ctx.base_uri, dynref, err_msg, true);
        auto entry = dyn_anchors.find (uri);
        if (entry == dyn_anchors.end())
            return resolve_ref (ctx, schema, dynref);

        std::string uri_without_fragment;
        std::string fragment;
        split_uri (uri, uri_without_fragment, fragment);
        fragment.insert (0, 1, '#');

        retval = &(get_tuple_schema(entry->second));
        ctx.base_uri = get_tuple_base_uri (entry->second);
        ctx.abs_keyword_path = get_tuple_abs_path (entry->second);

        validation_context* parent = ctx.parent;
        while (parent) {
            //uri = resolve_id (parent->base_uri, dynref, err_msg, true);
            uri = resolve_id (parent->base_uri, fragment, err_msg, true);
            auto entry = dyn_anchors.find (uri);
            if (entry != dyn_anchors.end()) {
                retval = &(get_tuple_schema(entry->second));
                ctx.base_uri = get_tuple_base_uri (entry->second);
                ctx.abs_keyword_path = get_tuple_abs_path (entry->second);
            }
            parent = parent->parent;
        }

        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_core::validate_dynamicRef (validation_context& ctx, jvalue& schema,
                                                jvalue& schema_value, jvalue& instance,
                                                const bool quit_on_first_error)
    {
        jvalue* target_schema = nullptr;
        validation_context sub_ctx (ctx);
        bool new_ref_schema_loaded = false;
        bool valid = true;

        do {
            if (schema_value.type() == j_string)
                target_schema = resolve_dynref (sub_ctx,
                                                schema,
                                                schema_value.str());
            if (target_schema) {
                /*
                  if (target_schema == &schema) {
                  // Simple recursion check
                  ctx.keywords.erase ("$dynamicRef");
                  throw invalid_schema (ctx.base_uri, ctx.abs_keyword_path.str(),
                  "Keyword '$dynamicRef' referring to same schema.");
                  }
                */
                valid = validate_subschema (sub_ctx, *target_schema, instance,
                                            quit_on_first_error, false, false, true);
            }else{
                if (invalid_ref_cb && !new_ref_schema_loaded) {
                    new_ref_schema_loaded = invalid_ref_cb (root_schema,
                                                            ctx.base_uri,
                                                            schema_value.str());
                    if (!new_ref_schema_loaded) {
                        sub_ctx.set_error ("Invalid reference");
                        ctx.set_valid (false);
                        ctx.add_output_unit (std::move(sub_ctx.output_unit));
                        valid = false;
                    }
                }else{
                    throw invalid_schema (ctx.base_uri, ctx.abs_keyword_path.str(),
                                          "Invalid reference for keyword '$dynamicRef'.");
                }
            }
        }while (new_ref_schema_loaded);

        return valid;
    }


/*
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_core::validate_comment (validation_context& ctx, jvalue& schema,
                                             jvalue& schema_value, jvalue& instance)
    {
        validation_context sub_ctx (ctx);
        sub_ctx.output_unit["annotation"] = schema_value;
        ctx.add_output_unit (std::move(sub_ctx.output_unit));
        return true;
    }
*/

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::load_schema (jvalue& schema, jvalue& schema_value, jvalue& load_ctx)
    {
        push_load_ctx_path (load_ctx, "$schema");

        if (schema_value.str() != "https://json-schema.org/draft/2020-12/schema")
            throw invalid_schema ("Schema not supported, not https://json-schema.org/draft/2020-12/schema");

        pop_load_ctx_path (load_ctx);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::load_id (jvalue& schema, jvalue& schema_value, jvalue& load_ctx)
    {
        push_load_ctx_path (load_ctx, "$id");

        std::string base_uri;
        std::string uri_error;
        if (load_ctx["base_uri"].array().empty())
            base_uri = resolve_id ("", schema_value.str(), uri_error);
        else
            base_uri = resolve_id (load_ctx["base_uri"].array().back().str(), schema_value.str(), uri_error);

        if (uri_error.empty() == false)
            throw invalid_schema (uri_error);

        load_ctx["base_uri"].append (base_uri);

        auto result = ids.try_emplace (base_uri, std::ref(schema));
        if (result.second == false) {
            throw invalid_schema ("Duplicate '$id'");
        }

        pop_load_ctx_path (load_ctx);

        // New base_uri, new absolute path relative to the new base_uri
        load_ctx["absolute_path"].append (jvalue(j_array));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::load_defs (jvalue& schema, jvalue& schema_value, jvalue& load_ctx)
    {
        push_load_ctx_path (load_ctx, "$defs");

        // Load sub-schemas in "$defs"
        for (auto& sub_schema : schema_value.obj()) {
            push_load_ctx_path (load_ctx, sub_schema.first);
            load_subschema (sub_schema.second);
            pop_load_ctx_path (load_ctx);
        }

        pop_load_ctx_path (load_ctx);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::load_anchor (jvalue& schema, jvalue& schema_value, jvalue& load_ctx)
    {
        push_load_ctx_path (load_ctx, "$anchor");

        std::regex re ("^[A-Za-z_][-A-Za-z0-9._]*$", std::regex::ECMAScript);
        std::cmatch cm;
        if (!std::regex_match (schema_value.str().c_str(), cm, re))
            throw invalid_schema ("Invalid '$anchor' value.");

        std::string abs_path;
        if (!load_ctx["absolute_path"].array().empty()) {
            jpointer abs_pointer (load_ctx["absolute_path"].array().back().array());
            if (!abs_pointer.empty())
                abs_pointer.pop_back ();
            abs_path = abs_pointer.str ();
        }

        std::string base_uri = load_ctx["base_uri"].array().back().str();
        std::string full_uri = base_uri;
        full_uri.append ("#");
        full_uri.append (schema_value.str());
        auto result = anchors.try_emplace (full_uri, std::make_tuple(base_uri, abs_path, std::ref(schema)));
        if (result.second == false) {
            throw invalid_schema ("Duplicate '$anchor'");
        }

#if (DEVEL_DEBUG)
        cerr << PREFIX << "base_uri       : " << load_ctx["base_uri"].array().back().str() << endl;
        cerr << PREFIX << "full_uri       : " << full_uri << endl;
        cerr << PREFIX << "validation_path: " << jpointer(load_ctx["validation_path"].array()).str() << endl;
        cerr << PREFIX << "absolute_path  : " << abs_path << endl;
#endif

        pop_load_ctx_path (load_ctx);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::load_dynamicAnchor (jvalue& schema, jvalue& schema_value, jvalue& load_ctx)
    {
        push_load_ctx_path (load_ctx, "$dynamicAnchor");

        std::regex re ("^[A-Za-z_][-A-Za-z0-9._]*$", std::regex::ECMAScript);
        std::cmatch cm;
        if (!std::regex_match (schema_value.str().c_str(), cm, re))
            throw invalid_schema ("Invalid '$dynamicAnchor' value.");

        std::string abs_path;
        if (!load_ctx["absolute_path"].array().empty()) {
            jpointer abs_pointer (load_ctx["absolute_path"].array().back().array());
            if (!abs_pointer.empty())
                abs_pointer.pop_back ();
            abs_path = abs_pointer.str ();
        }

        std::string base_uri = load_ctx["base_uri"].array().back().str();
        std::string full_uri = base_uri;
        full_uri.append ("#");
        full_uri.append (schema_value.str());
        auto result = dyn_anchors.try_emplace (full_uri, std::make_tuple(base_uri, abs_path, std::ref(schema)));
        if (result.second == false) {
            throw invalid_schema ("Duplicate '$dynamicAnchor'.");
        }

#if (DEVEL_DEBUG)
        cerr << PREFIX << "base_uri       : " << load_ctx["base_uri"].array().back().str() << endl;
        cerr << PREFIX << "full_uri       : " << full_uri << endl;
        cerr << PREFIX << "validation_path: " << jpointer(load_ctx["validation_path"].array()).str() << endl;
        cerr << PREFIX << "absolute_path  : " << abs_path << endl;
#endif

        pop_load_ctx_path (load_ctx);
    }


/*
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_core::load_comment (jvalue& schema, jvalue& schema_value, jvalue& load_ctx)
    {
        push_load_ctx_path (load_ctx, "$comment");
        if (schema_value.type() != j_string)
            throw invalid_schema ("Schema keyword '$comment' not a string.");
        pop_load_ctx_path (load_ctx);
    }
*/


}
