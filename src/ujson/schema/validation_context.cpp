/*
 * Copyright (C) 2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <ujson/schema/validation_context.hpp>

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


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    validation_context::validation_context ()
        : parent (nullptr),
          output_unit (j_object)
    {
        validation_path_ptr.reset (new jpointer);
        instance_path_ptr.reset (new jpointer);

        // Always assume the validation is
        // ok until it explicitly fails.
        output_unit["valid"] = true;
        output_unit["instanceLocation"] = "";
        output_unit["keywordLocation"] = "";
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    validation_context::validation_context (validation_context& parent_arg)
        : parent (&parent_arg),
          output_unit (j_object),
          validation_path_ptr (parent_arg.validation_path_ptr),
          instance_path_ptr (parent_arg.instance_path_ptr)
    {
        // Always assume the validation is
        // ok until it explicitly fails.
        output_unit["valid"] = true;

        base_uri = parent_arg.base_uri;
        abs_keyword_path = parent_arg.abs_keyword_path;

        // Initialize the output unit
        output_unit["instanceLocation"] = instance_path().str ();
        output_unit["keywordLocation"] = validation_path().str ();
        if (base_uri.empty() == false) {
            for (auto& kw : validation_path()) {
                if (kw == "$ref" || kw == "$dynamicRef") {
                    std::string abs_keyword_location;
                    abs_keyword_location = std::string("#") + abs_keyword_path.str();
                    output_unit["absoluteKeywordLocation"] = base_uri + abs_keyword_location;
                }
            }
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::push_schema_path (const std::string& entry)
    {
        validation_path().push_back (entry);
        abs_keyword_path.push_back (entry);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::pop_schema_path ()
    {
        if (!validation_path().empty())
            validation_path().pop_back ();

        if (!abs_keyword_path.empty())
            abs_keyword_path.pop_back ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::push_instance_path (const std::string& entry)
    {
        instance_path().push_back (entry);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::pop_instance_path ()
    {
        if (!instance_path().empty())
            instance_path().pop_back ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::set_error (const std::string& err_msg)
    {
        set_valid (false);

        auto& ou = output_unit.obj ();

        auto pos = ou.find ("error");
        if (pos != ou.end()) {
            pos->second = err_msg;
        }else{
            pos = ou.find ("errors");
            if (pos == ou.end())
                ou.emplace_back ("error", jvalue(err_msg));
            else
                ou.emplace (pos, "error", jvalue(err_msg));
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::append_error (const std::string& err_msg)
    {
        //set_valid (false);

        validation_context sub_ctx (*this);
        sub_ctx.set_error (err_msg);
        add_output_unit (std::move(sub_ctx.output_unit));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::append_sub_ou ()
    {
        validation_context sub_ctx (*this);
        add_output_unit (std::move(sub_ctx.output_unit));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::annotate (const jvalue& value)
    {
        // Store annotation for later usage
        //
#if (DEVEL_DEBUG)
        cerr << PREFIX << "Annotate: ("
             << (validation_path().empty() ? "" : validation_path().back()) << ", "
             << instance_path().str() << "): "
             << value.describe(fmt_color) << endl;
#endif
        annotations.emplace (
                std::make_pair(validation_path().empty() ? "" : validation_path().back(),
                               instance_path().str()),
                value);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue* validation_context::annotation (const std::string& keyword, const std::string& instance_path)
    {
        auto entry = annotations.find (std::make_pair(keyword, instance_path));
        if (entry != annotations.end())
            return &entry->second;
        else
            return nullptr;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::collect_annotations (validation_context& sub_ctx)
    {
        if (sub_ctx.annotations.empty() == false)
            in_place_annotations.emplace_back (std::move(sub_ctx.annotations));
        for (auto& annotations : sub_ctx.in_place_annotations)
            in_place_annotations.emplace_back (std::move(annotations));
    }


/*
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::debug_print ()
    {
        if (parent)
            cerr << "Root context         : No" << endl;
        else
            cerr << "Root context         : Yes" << endl;
        cerr << "Base URI             : " << base_uri << endl;
        cerr << "Absolute keyword path: " << abs_keyword_path.str() << endl;
        cerr << "Validation path      : " << validation_path().str() << endl;
        cerr << "Instance path        : " << instance_path().str() << endl;
        cerr << "Output unit          : " << output_unit.describe(fmt_color) << endl;
        if (annotations.empty()) {
            cerr << "Annotations          : " << endl;
        }else{
            bool first = true;
            for (auto& a : annotations) {
                if (first) {
                    cerr << "Annotations          : ";
                    first = false;
                }else{
                    cerr << "                       ";
                }
                cerr << a.first.first << ", " << a.first.second << ": " << a.second.describe() << endl;
            }
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::debug_print_annotations ()
    {
        if (annotations.empty() && in_place_annotations.empty()) {
            cerr << __FUNCTION__ << ": No annotations" << endl;
            return;
        }

        for (auto& a : annotations) {
            cerr << __FUNCTION__ << ": Annotation - ";
            cerr << "<\"" << a.first.first << "\", \"" << a.first.second << "\">: ";
            cerr << a.second.describe() << endl;
        }
        for (auto& list : in_place_annotations) {
            for (auto& a : list) {
                cerr << __FUNCTION__ << ": In-place   - ";
                cerr << "<\"" << a.first.first << "\", \"" << a.first.second << "\">: ";
                cerr << a.second.describe() << endl;
            }
        }

    }
*/


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::set_valid (const bool is_valid)
    {
        output_unit["valid"] = is_valid;

        if (is_valid) {
            output_unit.remove ("error");
            output_unit.remove ("errors");
        }else{
            output_unit.remove ("annotation");
            output_unit.remove ("annotations");
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void validation_context::add_output_unit (jvalue&& sub_output_unit, output_unit_placement_t where)
    {
        if (where == place_automatic)
            where = sub_output_unit["valid"].boolean() ? place_annotation : place_error;

        if (where == place_annotation) {
            output_unit["annotations"].type (j_array); // Create array if needed
            output_unit["annotations"].append (std::forward<jvalue>(sub_output_unit));
        }else{
            output_unit["errors"].type (j_array); // Create array if needed
            output_unit["errors"].append (std::forward<jvalue>(sub_output_unit));
        }
    }


}
