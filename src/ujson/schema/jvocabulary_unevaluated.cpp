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
#include <ujson/schema/jvocabulary_unevaluated.hpp>
#include <ujson/jschema.hpp>
#include <regex>


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
    jvocabulary_unevaluated::jvocabulary_unevaluated (jschema& schema_arg)
        : jvocabulary (schema_arg, "https://json-schema.org/draft/2020-12/vocab/unevaluated")
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_unevaluated::load (jvalue& schema, jvalue& load_ctx)
    {
        if (schema.type() == j_bool)
            return;

        auto& ui_value = schema.get ("unevaluatedItems");
        if (ui_value.valid()) {
            push_load_ctx_path (load_ctx, "unevaluatedItems");
            load_subschema (ui_value);
            pop_load_ctx_path (load_ctx);
        }

        auto& up_value = schema.get ("unevaluatedProperties");
        if (up_value.valid()) {
            push_load_ctx_path (load_ctx, "unevaluatedProperties");
            load_subschema (up_value);
            pop_load_ctx_path (load_ctx);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_unevaluated::validate (validation_context& ctx, jvalue& schema, jvalue& instance)
    {
        bool valid = true;
        if (instance.type() == j_array) {
            auto& schema_value = schema.get ("unevaluatedItems");
            if (schema_value.valid()) {
                ctx.push_schema_path ("unevaluatedItems");
                auto indexes = collect_unevaluatedItems_annotations (ctx, instance);
                if (indexes.empty() == false) {
                    if (validate_unevaluatedItems (ctx, indexes, schema, schema_value, instance) == false) {
                        ctx.set_valid (false);
                        valid = false;
                    }
                }
                ctx.pop_schema_path ();
            }
        }
        else if (instance.type() == j_object) {
            auto& schema_value = schema.get ("unevaluatedProperties");
            if (schema_value.valid()) {
                ctx.push_schema_path ("unevaluatedProperties");
                if (validate_unevaluatedProperties (ctx, schema, schema_value, instance) == false) {
                    ctx.set_valid (false);
                    valid = false;
                }
                ctx.pop_schema_path ();
            }
        }
        return valid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jvocabulary_unevaluated::get_unevaluatedItems_annotation (validation_context& ctx,
                                                                     const std::string& instance_path)
    {
        jvalue* a = ctx.annotation ("unevaluatedItems", instance_path);
        if (a && a->is_boolean())
            return jvalue (true);

        for (auto& annotations : ctx.in_place_annotations) {
            auto entry = annotations.find (std::make_pair("unevaluatedItems", instance_path));
            if (entry != annotations.end()  &&  entry->second.is_boolean())
                return jvalue (true);
        }
        return jvalue (j_invalid);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jvocabulary_unevaluated::get_prefixItems_annotation (validation_context& ctx,
                                                                const std::string& instance_path)
    {
        size_t max_index = (ssize_t)-1;
        bool have_max_index = false;

        jvalue* a = ctx.annotation ("prefixItems", instance_path);
        if (a) {
            if (a->is_boolean())
                return jvalue (true);
            max_index = a->num ();
            have_max_index = true;
        }

        for (auto& annotations : ctx.in_place_annotations) {
            auto entry = annotations.find (std::make_pair("prefixItems", instance_path));
            if (entry != annotations.end()) {
                if (entry->second.is_boolean()) {
                    return jvalue (true);
                }
                if (have_max_index) {
                    if (entry->second.num() > max_index)
                        max_index = entry->second.num();
                }else{
                    max_index = entry->second.num();
                }
                have_max_index = true;
            }
        }

        return have_max_index ? jvalue((double)max_index) : jvalue(j_invalid);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jvocabulary_unevaluated::get_items_annotation (validation_context& ctx,
                                                          const std::string& instance_path)
    {
        jvalue* a = ctx.annotation ("items", instance_path);
        if (a && a->is_boolean())
            return jvalue (true);

        for (auto& annotations : ctx.in_place_annotations) {
            auto entry = annotations.find (std::make_pair("items", instance_path));
            if (entry != annotations.end()  &&  entry->second.is_boolean())
                return jvalue (true);
        }

        return jvalue (j_invalid);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jvocabulary_unevaluated::get_contains_annotation (validation_context& ctx,
                                                             const std::string& instance_path)
    {
        std::set<size_t> indexes;

        jvalue* a = ctx.annotation ("contains", instance_path);
        if (a) {
            if (a->is_boolean()) {
                return jvalue (true);
            }else{
                for (auto& val : a->array())
                    indexes.emplace ((size_t)val.num());
            }
        }


        for (auto& annotations : ctx.in_place_annotations) {
            auto entry = annotations.find (std::make_pair("contains", instance_path));
            if (entry != annotations.end()) {
                if (entry->second.is_boolean()) {
                    return jvalue (true);
                }else{
                    for (auto& val : entry->second.array())
                        indexes.emplace ((size_t)val.num());
                }
            }
        }

        jvalue retval (j_array);
        for (auto i : indexes)
            retval.append ((double)i);
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::set<size_t> jvocabulary_unevaluated::collect_unevaluatedItems_annotations (
            validation_context& ctx, jvalue& instance)
    {
        std::set<size_t> indexes;
        std::string instance_path = ctx.instance_path().str ();
        size_t max_index = (ssize_t)-1;

        // Check annotation result of "unevaluatedItems"
        //
        if (get_unevaluatedItems_annotation(ctx, instance_path).is_boolean())
            return indexes;

        // Check annotation result of "prefixItems"
        //
        auto prefixItems = get_prefixItems_annotation (ctx, instance_path);
        if (prefixItems.valid()) {
            if (prefixItems.is_boolean())
                return indexes;
            else
                max_index = prefixItems.num ();
        }

        // Check annotation result of "items"
        //
        if (get_items_annotation(ctx, instance_path).is_boolean())
            return indexes;

        // Check annotation result of "contains"
        //
        auto contains = get_contains_annotation (ctx, instance_path);
        if (contains.is_boolean())
            return indexes;

        // Collect all available index numbers
        for (size_t i=max_index+1; i<instance.size(); ++i)
            indexes.emplace (i);

        // Remove indexes from keyword "contains"
        for (auto& val : contains.array()) {
            size_t index_value = val.num ();
            indexes.erase (index_value);
        }

        return indexes;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_unevaluated::validate_unevaluatedItems (validation_context& ctx,
                                                             std::set<size_t>& indexes,
                                                             jvalue& schema,
                                                             jvalue& schema_value,
                                                             jvalue& instance)
    {
        validation_context sub_ctx (ctx);

        bool all_valid = true;
        for (auto i : indexes) {
            sub_ctx.push_instance_path (std::to_string(i));
            if (validate_subschema(sub_ctx, schema_value, instance[i]) == false)
                all_valid = false;
            sub_ctx.pop_instance_path ();
        }

        sub_ctx.set_valid (all_valid);
        if (all_valid) {
            ctx.collect_annotations (sub_ctx);
            sub_ctx.output_unit["annotation"] = true;
        }else{
            ctx.set_valid (false);
        }

        ctx.annotate (jvalue(true));
        ctx.add_output_unit (std::move(sub_ctx.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_unevaluated::get_properties_annotations (validation_context& ctx,
                                                              const std::string& instance_path,
                                                              const std::string& keyword,
                                                              std::set<std::string>& names)
    {
        jvalue* a = ctx.annotation (keyword, instance_path);
        if (a) {
            for (auto& name : a->array())
                names.emplace (name.str());
        }

        for (auto& annotations : ctx.in_place_annotations) {
            auto entry = annotations.find (std::make_pair(keyword, instance_path));
            if (entry != annotations.end()) {
                for (auto& name : entry->second.array())
                    names.emplace (name.str());
            }
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::set<std::string> jvocabulary_unevaluated::collect_unevaluatedProperties_annotations (
            validation_context& ctx)
    {
        std::set<std::string> names;
        std::string instance_path = ctx.instance_path().str ();

        get_properties_annotations (ctx, instance_path, "properties", names);
        get_properties_annotations (ctx, instance_path, "patternProperties", names);
        get_properties_annotations (ctx, instance_path, "additionalProperties", names);
        get_properties_annotations (ctx, instance_path, "unevaluatedProperties", names);

        return names;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_unevaluated::validate_unevaluatedProperties (validation_context& ctx,
                                                                  jvalue& schema,
                                                                  jvalue& schema_value,
                                                                  jvalue& instance)
    {
        if (instance.obj().empty())
            return true;

        bool none_evaluated = true;
        bool all_valid = true;
        auto evaluated_names = collect_unevaluatedProperties_annotations (ctx);

        std::string ctx_instance_path = ctx.instance_path().str ();
        jvalue annotation (j_array);
        validation_context sub_ctx (ctx);

        for (auto& property : instance.obj()) {
            auto& property_name = property.first;
            auto& sub_instance = property.second;

            // Skip property names already evaluated
            if (evaluated_names.find(property_name) != evaluated_names.end())
                continue;

            none_evaluated = false;

            sub_ctx.push_instance_path (property_name);
            if (validate_subschema(sub_ctx, schema_value, sub_instance) == false)
                all_valid = false;
            else
                annotation.append (property_name);
            sub_ctx.pop_instance_path ();
        }

        if (none_evaluated)
            return true;

        sub_ctx.set_valid (all_valid);
        if (all_valid) {
            sub_ctx.output_unit["annotation"] = annotation;
            ctx.annotate (annotation);
            ctx.collect_annotations (sub_ctx);
        }else{
            ctx.set_valid (false);
        }
        ctx.add_output_unit (std::move(sub_ctx.output_unit));

        return all_valid;
    }


}
