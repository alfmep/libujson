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
#include <ujson.hpp>
#include <ujson/jschema.hpp>
#include <ujson/schema/jvocabulary_core.hpp>
#include <ujson/schema/jvocabulary_applicator.hpp>
#include <ujson/schema/jvocabulary_validation.hpp>
#include <ujson/schema/jvocabulary_unevaluated.hpp>


#define DEVEL_DEBUG 0

#if (DEVEL_DEBUG)
#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
#endif


namespace ujson {


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static std::string carefully_get_schema_id (jvalue& schema,
                                                const std::string& default_id="")
    {
        std::string id (default_id);
        if (schema.type() == j_object) {
            auto& value = schema.get ("$id");
            if (value.type() == j_string)
                id = value.str ();
        }
        return id;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jschema::jschema ()
        : jschema (true)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jschema::jschema (const jvalue& root_arg)
    {
        initialize (root_arg);
/*
#if (DEVEL_DEBUG)
        cerr << "--------------------------------------------------------------------------------" << endl;
        dynamic_cast<schema::jvocabulary_core&>(*vocabularies.front().second).print_maps (cerr);
        cerr << "--------------------------------------------------------------------------------" << endl;
#endif
*/
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jschema::jschema (const jvalue& root_arg, const std::list<jvalue>& referenced_schemas)
        : jschema (root_arg)
    {
        // Load referenced schemas
        for (const auto& schema : referenced_schemas)
            add_referenced_schema (schema);

#if (DEVEL_DEBUG)
        cerr << "--------------------------------------------------------------------------------" << endl;
        dynamic_cast<schema::jvocabulary_core&>(*vocabularies.front().second).print_maps (cerr);
        cerr << "--------------------------------------------------------------------------------" << endl;
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jschema::reset (const jvalue& root_arg)
    {
        // Clear old attributes
        vocabularies.clear ();
        ref_schemas.clear ();
        id_alias.clear ();
        load_ctx.obj().clear ();

        initialize (root_arg);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jschema::initialize (const jvalue& root_arg)
    {
        // Initialize load context
        load_ctx.type (j_object);
        load_ctx["base_uri"].type (j_array);        // Array of base_uri's
        load_ctx["absolute_path"].type (j_array);   // Array of absolute paths relative to the corresponding base_uri
        load_ctx["validation_path"].type (j_array); // Path relative to the root of the schema

        // Initialize vocabularies
        init_vocabularies ();

        // Load root schema
        root = root_arg;
        try {
            if (root.type()==j_object && !root.has("$id")) {
                root["$id"] = default_base_uri;
            }
            load (root);
            load_ctx["base_uri"].array().clear ();
            load_ctx["validation_path"].array().clear ();
            load_ctx["absolute_path"].array().clear ();
        }
        catch (std::exception& e) {
            throw invalid_schema (carefully_get_schema_id(root, default_base_uri),
                                  jpointer(load_ctx["validation_path"].array()),
                                  std::string("Error loading root schema: ") + e.what());
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jschema::add_referenced_schema (const jvalue& referenced_schema)
    {
        add_referenced_schema (referenced_schema, "");
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jschema::add_referenced_schema (const jvalue& referenced_schema, const std::string& alias)
    {
        load_ctx["base_uri"].array().clear ();
        load_ctx["validation_path"].array().clear (); // Path relative to the root of the schema
        load_ctx["absolute_path"].array().clear ();   // Absolute path relative to current base URI

        auto ref_schema = referenced_schema;

        std::string ref_schema_id = carefully_get_schema_id (const_cast<jvalue&>(ref_schema));
        bool add_alias = false;
        if (ref_schema_id.empty()) {
            if (alias.empty()) {
                throw invalid_schema ("Referenced schema missing '$id'");
            }else{
                ref_schema_id = alias;
                ref_schema["$id"] = ref_schema_id;
            }
        }else if (!alias.empty()) {
            add_alias = true;
        }

        try {
            ref_schemas.emplace_back (std::move(ref_schema));
            load (ref_schemas.back());
            if (add_alias)
                id_alias.emplace (alias, ref_schema_id);
        }
        catch (std::exception& e) {
            throw invalid_schema (ref_schema_id, //carefully_get_schema_id(ref_schemas.back()),
                                  jpointer(load_ctx["validation_path"].array()),
                                  std::string("Error loading referenced schema: ") + e.what());
        }

        load_ctx["base_uri"].array().clear ();
        load_ctx["validation_path"].array().clear ();
        load_ctx["absolute_path"].array().clear ();

#if (DEVEL_DEBUG)
        cerr << "--------------------------------------------------------------------------------" << endl;
        dynamic_cast<schema::jvocabulary_core&>(*vocabularies.front().second).print_maps (cerr);
        cerr << "--------------------------------------------------------------------------------" << endl;
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jschema::set_invalid_ref_cb (schema::jvocabulary_core::invalid_ref_cb_t cb)
    {
        dynamic_cast<schema::jvocabulary_core&>(*vocabularies.front().second).set_invalid_ref_cb (cb);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jschema::init_vocabularies ()
    {
        vocabularies.emplace_back ("core", std::make_shared<schema::jvocabulary_core>(*this));
        vocabularies.emplace_back ("applicator", std::make_shared<schema::jvocabulary_applicator>(*this));
        vocabularies.emplace_back ("validation",  std::make_shared<schema::jvocabulary_validation>(*this));
        vocabularies.emplace_back ("unevaluated", std::make_shared<schema::jvocabulary_unevaluated>(*this));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::shared_ptr<schema::jvocabulary> jschema::vocabulary (const std::string& name)
    {
        auto entry = vocabularies.find (name);
        return entry==vocabularies.end() ? std::shared_ptr<schema::jvocabulary>(nullptr) : entry->second;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jschema::load (jvalue& schema)
    {
        if (schema.type() == j_bool)
            return;

        size_t id_size = load_ctx["base_uri"].array().size ();
        size_t abs_paths = load_ctx["absolute_path"].array().size ();

        for (auto& entry : vocabularies)
            entry.second->load (schema, load_ctx);

        if (load_ctx["base_uri"].array().size() > id_size)
            load_ctx["base_uri"].array().pop_back ();
        if (load_ctx["absolute_path"].array().size() > abs_paths)
            load_ctx["absolute_path"].array().pop_back ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jschema::validate (jvalue& instance)
    {
        return validate (instance, true);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jschema::validate (jvalue& instance, bool quit_on_first_error)
    {
        schema::validation_context ctx;
        validate (ctx, root, instance, quit_on_first_error);

        if (ctx.output_unit["valid"].boolean()) {
            ctx.output_unit.remove ("error");
            ctx.output_unit.remove ("errors");
        }else{
            ctx.output_unit.remove ("annotation");
            ctx.output_unit.remove ("annotations");
        }

        return ctx.output_unit;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jschema::validate (schema::validation_context& ctx,
                            jvalue& schema,
                            jvalue& instance,
                            bool quit_on_first_error)
    {
        // Handle boolean schemas
        if (schema.type() == j_bool) {
            ctx.output_unit["valid"] = schema.boolean ();
            return schema.boolean ();
        }

        bool valid = true;
        for (auto& entry : vocabularies) {
            if (entry.second->validate(ctx, schema, instance, quit_on_first_error) == false) {
                valid = false;
                if (quit_on_first_error)
                    break;
            }
        }

        return valid;
    }


}
