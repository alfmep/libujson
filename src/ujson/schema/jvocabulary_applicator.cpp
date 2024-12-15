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
#include <ujson/schema/jvocabulary_applicator.hpp>
#include <ujson/invalid_schema.hpp>
#include <ujson/jschema.hpp>
#include <algorithm>
#include <functional>
#include <regex>
#include <set>

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


    const jvocabulary_applicator::keywords_t jvocabulary_applicator::keywords = {{
            // Any instance type
            {"allOf", std::make_tuple(
                        j_array, &jvocabulary_applicator::load_allOf,
                        j_invalid, &jvocabulary_applicator::validate_allOf)},
            {"anyOf", std::make_tuple(
                        j_array, &jvocabulary_applicator::load_anyOf,
                        j_invalid,  &jvocabulary_applicator::validate_anyOf)},
            {"oneOf", std::make_tuple(
                        j_array, &jvocabulary_applicator::load_oneOf,
                        j_invalid,  &jvocabulary_applicator::validate_oneOf)},
            {"not", std::make_tuple(
                        j_invalid, nullptr,
                        j_invalid,  &jvocabulary_applicator::validate_not)},
            {"if", std::make_tuple(
                        j_invalid, nullptr,
                        j_invalid,  nullptr)},
            {"then", std::make_tuple(
                        j_invalid, nullptr,
                        j_invalid,  nullptr)},
            {"else", std::make_tuple(
                        j_invalid, nullptr,
                        j_invalid,  nullptr)},

            // Array instances
            {"prefixItems", std::make_tuple(
                        j_array, &jvocabulary_applicator::load_prefixItems,
                        j_array,  &jvocabulary_applicator::validate_prefixItems)},
            {"items", std::make_tuple(
                        j_invalid, nullptr,
                        j_array,  nullptr)},
            {"contains", std::make_tuple(
                        j_invalid, nullptr,
                        j_array,  &jvocabulary_applicator::validate_contains)},

            // Object instances
            {"dependentSchemas", std::make_tuple(
                        j_object, nullptr,
                        j_object,  &jvocabulary_applicator::validate_dependentSchemas)},
            {"properties", std::make_tuple(
                        j_object, nullptr,
                        j_object,  &jvocabulary_applicator::validate_properties)},
            {"patternProperties", std::make_tuple(
                        j_object, nullptr,
                        j_object,  &jvocabulary_applicator::validate_patternProperties)},
            {"additionalProperties", std::make_tuple(
                        j_invalid, nullptr,
                        j_object,  nullptr)},
            {"propertyNames", std::make_tuple(
                        j_invalid, nullptr,
                        j_object,  &jvocabulary_applicator::validate_propertyNames)},
        }};


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvocabulary_applicator::jvocabulary_applicator (jschema& schema_arg)
        : jvocabulary (schema_arg, "https://json-schema.org/draft/2020-12/meta/applicator")
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_applicator::load (jvalue& schema, jvalue& load_ctx)
    {
        if (schema.type() == j_bool)
            return;

        for (auto& member : schema.obj()) {
            const std::string& keyword = member.first;
            jvalue& kw_value = member.second;

            auto entry = keywords.find (keyword);
            if (entry == keywords.end())
                continue;
            auto kw_jtype = std::get<0> (entry->second);
            auto kw_loader = std::get<1> (entry->second);

            if (kw_loader) {
                push_load_ctx_path (load_ctx, keyword);
                (this->*kw_loader) (kw_value, load_ctx);
                pop_load_ctx_path (load_ctx);
                continue;
            }

            push_load_ctx_path (load_ctx, keyword);

            if (kw_jtype == j_invalid) {
                // Load single subschema
                load_subschema (kw_value);
            }
            else if (kw_jtype == j_array) {
                // Load subschemas from an array
                for (size_t i=0; i<kw_value.size(); ++i) {
                    push_load_ctx_path (load_ctx, std::to_string(i));
                    load_subschema (kw_value[i]);
                    pop_load_ctx_path (load_ctx);
                }
            }
            else if (kw_jtype == j_object) {
                // Load subschemas from an object
                for (auto& sub_schema : kw_value.obj()) {
                    push_load_ctx_path (load_ctx, sub_schema.first);
                    load_subschema (sub_schema.second);
                    pop_load_ctx_path (load_ctx);
                }
            }

            pop_load_ctx_path (load_ctx);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_applicator::load_allOf (jvalue& schema_value, jvalue& load_ctx)
    {
        if (schema_value.type() != j_array  ||  schema_value.array().empty()) {
            // 10.2.1.1. This keyword's value MUST be a non-empty array.
            //           Each item of the array MUST be a valid JSON Schema.
            throw invalid_schema ("Keyword 'allOf' is not a non-empty array of subschemas.");
        }

        auto& array = schema_value.array ();
        for (unsigned i=0; i<array.size(); ++i) {
            push_load_ctx_path (load_ctx, std::to_string(i));
            load_subschema (array[i]);
            pop_load_ctx_path (load_ctx);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_applicator::load_anyOf (jvalue& schema_value, jvalue& load_ctx)
    {
        if (schema_value.type() != j_array  ||  schema_value.array().empty()) {
            // 10.2.1.2. This keyword's value MUST be a non-empty array.
            //           Each item of the array MUST be a valid JSON Schema.
            throw invalid_schema ("Keyword 'anyOf' is not a non-empty array of subschemas.");
        }

        auto& array = schema_value.array ();
        for (unsigned i=0; i<array.size(); ++i) {
            push_load_ctx_path (load_ctx, std::to_string(i));
            load_subschema (array[i]);
            pop_load_ctx_path (load_ctx);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_applicator::load_oneOf (jvalue& schema_value, jvalue& load_ctx)
    {
        // Sanity check
        if (schema_value.type() != j_array  ||  schema_value.array().empty()) {
            // 10.2.1.3. This keyword's value MUST be a non-empty array.
            //           Each item of the array MUST be a valid JSON Schema.
            throw invalid_schema ("Keyword 'oneOf' is not a non-empty array of subschemas.");
        }

        auto& array = schema_value.array ();
        for (unsigned i=0; i<array.size(); ++i) {
            push_load_ctx_path (load_ctx, std::to_string(i));
            load_subschema (array[i]);
            pop_load_ctx_path (load_ctx);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvocabulary_applicator::load_prefixItems (jvalue& schema_value, jvalue& load_ctx)
    {
        if (schema_value.type() != j_array  ||  schema_value.array().empty()) {
            // 10.3.1.1. The value of "prefixItems" MUST be a non-empty array of valid JSON Schemas.
            throw invalid_schema ("Keyword 'prefixItems' is not a non-empty array of subschemas.");
        }

        auto& array = schema_value.array ();
        for (unsigned i=0; i<array.size(); ++i) {
            push_load_ctx_path (load_ctx, std::to_string(i));
            load_subschema (array[i]);
            pop_load_ctx_path (load_ctx);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate (validation_context& ctx,
                                           jvalue& schema,
                                           jvalue& instance,
                                           const bool quit_on_first_error)
    {
        auto instance_type = instance.type ();
        bool keyword_if_handled = false;
        bool is_if_true = false;
        bool valid = true;

        for (auto& member : schema.obj()) {

            if (quit_on_first_error && valid==false)
                break;

            const std::string& schema_keyword = member.first;
            ujson::jvalue& schema_value = member.second;

            // Find a validator for this keyword
            auto entry = keywords.find (schema_keyword);
            if (entry != keywords.end()) {
                auto kw_validator = std::get<3> (entry->second);
                if (kw_validator) {
                    // Check if the keyword handles this type of instance
                    auto kw_jtype = std::get<2> (entry->second);
                    if (kw_jtype==instance_type || kw_jtype==j_invalid) {
                        std::string error_msg;
                        ctx.push_schema_path (schema_keyword);
                        if ((this->*kw_validator)(ctx, schema, schema_value, instance, quit_on_first_error) == false)
                            valid = false;
                        ctx.pop_schema_path ();
                        continue;
                    }
                }
            }

            // Handle if-then-else
            //
            if (!keyword_if_handled && schema_keyword == "if") {
                keyword_if_handled = true;
                ctx.push_schema_path ("if");
                is_if_true = validate_if (ctx, schema, schema_value, instance, quit_on_first_error);
                ctx.pop_schema_path ();

                if (is_if_true) {
                    auto& schema_value = schema.get ("then");
                    if (schema_value.valid()) {
                        ctx.push_schema_path ("then");
                        if (validate_then(ctx, schema, schema_value, instance, quit_on_first_error) == false)
                            valid = false;
                        ctx.pop_schema_path ();
                    }
                }else{
                    auto& schema_value = schema.get ("else");
                    if (schema_value.valid()) {
                        ctx.push_schema_path ("else");
                        if (validate_else(ctx, schema, schema_value, instance, quit_on_first_error) == false)
                            valid = false;
                        ctx.pop_schema_path ();
                    }
                }
            }
        }

        if (quit_on_first_error && valid==false)
            return false;

        if (instance_type == j_array) {
            // Handle 'items' after other array-keywords
            // items depends on 'prefixItems'
            auto& schema_value = schema.get ("items");
            if (schema_value.valid()) {
                ctx.push_schema_path ("items");
                if (validate_items(ctx, schema, schema_value, instance, quit_on_first_error) == false)
                    valid = false;
                ctx.pop_schema_path ();
            }
        }
        else if (instance_type == j_object) {
            // Handle 'additionalProperties' after other property-keywords
            // additionalProperties depends on 'properties' and 'patternProperties'
            auto& schema_value = schema.get ("additionalProperties");
            if (schema_value.valid()) {
                ctx.push_schema_path ("additionalProperties");
                if (validate_additionalProperties(ctx, schema, schema_value, instance, quit_on_first_error) == false)
                    valid = false;
                ctx.pop_schema_path ();
            }
        }

        return valid;
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for any instance
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_allOf (validation_context& ctx,
                                                 jvalue& schema,
                                                 jvalue& schema_value,
                                                 jvalue& instance,
                                                 const bool quit_on_first_error)
    {
        bool all_valid = true;
        validation_context ctx_allOf (ctx);

        for (size_t i=0; i < schema_value.array().size(); ++i) {
            ctx_allOf.push_schema_path (std::to_string(i));
            if (validate_subschema(ctx_allOf, schema_value[i], instance, quit_on_first_error) == false)
                all_valid = false;
            ctx_allOf.pop_schema_path ();
            if (quit_on_first_error && all_valid==false)
                break;
        }

        ctx_allOf.set_valid (all_valid);
        if (all_valid) {
            ctx.collect_annotations (ctx_allOf);
        }else{
            ctx_allOf.set_error ("Not all subschema evaluated true.");
            ctx.set_valid (false);
        }

        ctx.add_output_unit (std::move(ctx_allOf.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for any instance
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_anyOf (validation_context& ctx,
                                                 jvalue& schema,
                                                 jvalue& schema_value,
                                                 jvalue& instance,
                                                 const bool quit_on_first_error)
    {
        bool some_valid = false;
        validation_context ctx_anyOf (ctx);

        for (size_t i=0; i < schema_value.array().size(); ++i) {
            ctx_anyOf.push_schema_path (std::to_string(i));
            if (validate_subschema(ctx_anyOf, schema_value[i], instance, quit_on_first_error) == true)
                some_valid = true;
            ctx_anyOf.pop_schema_path ();
        }

        ctx_anyOf.set_valid (some_valid);
        if (some_valid) {
            ctx.collect_annotations (ctx_anyOf);
        }else{
            ctx_anyOf.set_error ("No subschema evaluated true.");
            ctx.set_valid (false);
        }
        ctx.add_output_unit (std::move(ctx_anyOf.output_unit));

        return some_valid;
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for any instance
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_oneOf (validation_context& ctx,
                                                 jvalue& schema,
                                                 jvalue& schema_value,
                                                 jvalue& instance,
                                                 const bool quit_on_first_error)
    {
        size_t num_valid = 0;
        size_t num_subschemas = 0;
        bool retval = true;
        validation_context ctx_oneOf (ctx);

        for (size_t i=0; i < schema_value.array().size(); ++i) {
            ++num_subschemas;
            ctx_oneOf.push_schema_path (std::to_string(i));

            if (validate_subschema (ctx_oneOf, schema_value[i], instance, quit_on_first_error))
                ++num_valid;

            ctx_oneOf.pop_schema_path ();

            if (quit_on_first_error && num_valid>1)
                break;
        }

        ctx_oneOf.set_valid (num_valid == 1);
        if (num_valid == 1) {
            ctx.collect_annotations (ctx_oneOf);
        }else{
            retval = false;
            if (num_valid == 0)
                ctx_oneOf.set_error ("No subschema evaluated true.");
            else if (num_valid == num_subschemas && !quit_on_first_error)
                ctx_oneOf.set_error ("All subschemas evaluated true.");
            else
                ctx_oneOf.set_error ("More than one subschema evaluated true.");
            ctx.set_valid (false);
        }
        ctx.add_output_unit (std::move(ctx_oneOf.output_unit));

        return retval;
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for any instance
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_not (validation_context& ctx,
                                               jvalue& schema,
                                               jvalue& schema_value,
                                               jvalue& instance,
                                               const bool quit_on_first_error)
    {
        validation_context sub_ctx (ctx);

        bool is_valid = validate_subschema (sub_ctx, schema_value, instance, quit_on_first_error, false, true);
        sub_ctx.set_valid (!is_valid);
        if (is_valid) {
            sub_ctx.set_error ("Subschema evaluated true.");
            ctx.set_valid (false);
        }
        ctx.add_output_unit (std::move(sub_ctx.output_unit));

        return !is_valid;
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for any instance
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_if (validation_context& ctx,
                                              jvalue& schema,
                                              jvalue& schema_value,
                                              jvalue& instance,
                                              const bool quit_on_first_error)
    {
        return validate_subschema (ctx, schema_value, instance, quit_on_first_error);
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for any instance
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_then (validation_context& ctx,
                                                jvalue& schema,
                                                jvalue& schema_value,
                                                jvalue& instance,
                                                const bool quit_on_first_error)
    {
        return validate_subschema (ctx, schema_value, instance, quit_on_first_error, true, false, true);
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for any instance
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_else (validation_context& ctx,
                                                jvalue& schema,
                                                jvalue& schema_value,
                                                jvalue& instance,
                                                const bool quit_on_first_error)
    {
        return validate_subschema (ctx, schema_value, instance, quit_on_first_error, true, false, true);
    }


    //--------------------------------------------------------------------------
    // 10.2. Keywords for Applying Subschemas in Place
    //
    // Applicator keywords for objects
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_dependentSchemas (validation_context& ctx,
                                                            jvalue& schema,
                                                            jvalue& schema_value,
                                                            jvalue& instance,
                                                            const bool quit_on_first_error)
    {
        bool all_valid = true;
        validation_context ctx_ds (ctx);

        for (auto& schema_property : schema_value.obj()) {
            const std::string& property_name = schema_property.first;
            if (! instance.has(property_name))
                continue;

            jvalue& subschema = schema_property.second;

            ctx_ds.push_schema_path (property_name);
            if (validate_subschema(ctx_ds, subschema, instance, quit_on_first_error) == false)
                all_valid = false;
            ctx_ds.pop_schema_path ();

            if (quit_on_first_error && all_valid==false)
                break;
        }

        ctx_ds.set_valid (all_valid);
        if (all_valid) {
            ctx.collect_annotations (ctx_ds);
        }else{
            ctx_ds.set_error ("Not all subschema evaluated true.");
            ctx.set_valid (false);
        }
        ctx.add_output_unit (std::move(ctx_ds.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.3. Keywords for Applying Subschemas to Child Instances
    //
    // Applicator keywords for arrays
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_prefixItems (validation_context& ctx,
                                                       jvalue& schema,
                                                       jvalue& schema_value,
                                                       jvalue& instance,
                                                       const bool quit_on_first_error)
    {
        bool all_valid = true;
        size_t items_in_schema_array = schema_value.size ();
        size_t items_in_instance = instance.size ();
        size_t items_to_test = std::min (items_in_schema_array, items_in_instance);

        validation_context ctx_pi (ctx);

        for (size_t i=0; i<items_to_test; ++i) {
            jvalue& sub_schema = schema_value[i];
            jvalue& sub_instance = instance[i];
            auto index_str = std::to_string(i);

            ctx_pi.push_schema_path (index_str);
            ctx_pi.push_instance_path (index_str);

            if (validate_subschema(ctx_pi, sub_schema, sub_instance, quit_on_first_error) == false)
                all_valid = false;

            ctx_pi.pop_schema_path ();
            ctx_pi.pop_instance_path ();

            if (quit_on_first_error && all_valid==false)
                break;
        }

        ctx_pi.set_valid (all_valid);
        if (!all_valid) {
            ctx_pi.set_error ("Not all subschemas evaluated true.");
            ctx.set_valid (false);
        }

        if (items_to_test >= items_in_instance) {
            ctx.annotate (jvalue(true));
            ctx_pi.output_unit["annotation"] = true;
        }else{
            ctx.annotate (jvalue((long int)items_to_test-1));
            ctx_pi.output_unit["annotation"] = (long) (items_to_test-1);
        }

        ctx.add_output_unit (std::move(ctx_pi.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.3. Keywords for Applying Subschemas to Child Instances
    //
    // Applicator keywords for arrays
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_items (validation_context& ctx,
                                                 jvalue& schema,
                                                 jvalue& schema_value,
                                                 jvalue& instance,
                                                 const bool quit_on_first_error)
    {
        bool all_valid = true;

        jvalue* annotation = nullptr;
        annotation = ctx.annotation ("prefixItems", ctx.instance_path().str());

        size_t i = 0;
        if (annotation) {
            if (annotation->is_boolean() && annotation->boolean())
                return true;
            else if (annotation->is_number())
                i = (size_t) annotation->num ();
            ++i;
        }
        size_t num_items_in_instance = instance.size ();
        bool schema_applied = false;

        validation_context ctx_items (ctx);

        for (; i<num_items_in_instance; ++i) {
            schema_applied = true;
            ctx_items.push_instance_path (std::to_string(i));
            if (validate_subschema(ctx_items, schema_value, instance[i], quit_on_first_error) == false)
                all_valid = false;
            ctx_items.pop_instance_path ();

            if (quit_on_first_error && all_valid==false)
                break;
        }

        ctx_items.set_valid (all_valid);
        if (!all_valid) {
            ctx_items.set_error ("Not all array items evaluated true.");
            ctx.set_valid (false);
        }

        if (schema_applied) {
            ctx.annotate (jvalue(true));
            ctx_items.output_unit["annotation"] = true;
        }
        ctx.add_output_unit (std::move(ctx_items.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.3. Keywords for Applying Subschemas to Child Instances
    //
    // Applicator keywords for arrays
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_contains (validation_context& ctx,
                                                    jvalue& schema,
                                                    jvalue& schema_value,
                                                    jvalue& instance,
                                                    const bool quit_on_first_error)
    {
        jvalue annotation_value (j_array);
        bool all_valid = true;
        bool some_valid = false;
        size_t items_in_instance = instance.size ();

        validation_context ctx_contains (ctx);

        auto& minContains_value = schema.get ("minContains");
        if (minContains_value.is_number()  &&  minContains_value.num()==0) {
            // 10.3.1.3
            // ...when "minContains" is present and has a value of 0,
            // in which case an array instance MUST be considered valid
            // against the "contains" keyword, even if none of its
            // elements is valid against the given schema.
            all_valid = true;
        }else{
            if (items_in_instance == 0) {
                // Instance array is empty
                all_valid = false;
                some_valid = false;
            }else{
                for (size_t i=0; i<items_in_instance; ++i) {
                    ctx_contains.push_instance_path (std::to_string(i));
                    validation_context sub_ctx (ctx_contains);

                    if (validate_subschema(sub_ctx, schema_value, instance[i], quit_on_first_error, false, true)) {
                        some_valid = true;
                        annotation_value.append ((int)i);
                    }else{
                        all_valid = false;
                    }

                    ctx_contains.add_output_unit (std::move(sub_ctx.output_unit));
                    ctx_contains.pop_instance_path ();
                }
            }
        }

        if (some_valid || all_valid) {
            if (all_valid  &&  items_in_instance > 0)
                annotation_value = true;
            ctx_contains.set_valid (true);
        }else{
            ctx_contains.set_error ("No array item evaluated true.");
            ctx.set_valid (false);
        }

        ctx.annotate (annotation_value);
        ctx_contains.output_unit["annotation"] = annotation_value;
        ctx.add_output_unit (std::move(ctx_contains.output_unit));

        return some_valid || all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.3. Keywords for Applying Subschemas to Child Instances
    //
    // Applicator keywords for objects
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_properties (validation_context& ctx,
                                                      jvalue& schema,
                                                      jvalue& schema_value,
                                                      jvalue& instance,
                                                      const bool quit_on_first_error)
    {
        bool all_valid = true;
        std::string ctx_instance_path = ctx.instance_path().str ();
        jvalue annotation (j_array);

        validation_context ctx_props (ctx);

        for (auto& property : schema_value.obj()) {

            if (quit_on_first_error && all_valid==false)
                break;

            auto& property_name = property.first;
            auto& sub_schema = property.second;
            auto& sub_instance = instance.get (property_name);
            if (sub_instance.invalid())
                continue;

            ctx_props.push_schema_path (property_name);
            ctx_props.push_instance_path (property_name);

            /*
            if (validate_subschema(ctx_props, sub_schema, sub_instance, quit_on_first_error))
                annotation.append (property_name);
            else
                all_valid = false;
            */
            if (!validate_subschema(ctx_props, sub_schema, sub_instance, quit_on_first_error))
                all_valid = false;
            annotation.append (property_name);

            ctx_props.pop_schema_path ();
            ctx_props.pop_instance_path ();
        }

        ctx_props.set_valid (all_valid);

        ctx.annotate (annotation);

        if (all_valid) {
            ctx.collect_annotations (ctx_props);
            //ctx.annotate (annotation);
            ctx_props.output_unit["annotation"] = annotation;
        }else{
            ctx.set_valid (false);
            ctx_props.set_error ("Not all properties evaluated true.");
        }
        ctx.add_output_unit (std::move(ctx_props.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.3. Keywords for Applying Subschemas to Child Instances
    //
    // Applicator keywords for objects
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_patternProperties (validation_context& ctx,
                                                             jvalue& schema,
                                                             jvalue& schema_value,
                                                             jvalue& instance,
                                                             const bool quit_on_first_error)
    {
        std::cmatch cm;
        bool all_valid = true;
        std::string ctx_instance_path = ctx.instance_path().str ();
        jvalue annotation (j_array);

        validation_context ctx_props (ctx);

        for (auto& schema_property : schema_value.obj()) {
            if (quit_on_first_error && all_valid==false)
                break;

            auto& property_pattern = schema_property.first;
            auto& sub_schema = schema_property.second;

            std::regex re (property_pattern, std::regex::ECMAScript);

            for (auto& instance_property : instance.obj()) {
                if (quit_on_first_error && all_valid==false)
                    break;
                auto& property_name = instance_property.first;
                if (! std::regex_search(property_name.c_str(), cm, re))
                    continue;

                ctx_props.push_schema_path (property_pattern);
                ctx_props.push_instance_path (property_name);

                auto& sub_instance = instance_property.second;
                /*
                if (validate_subschema(ctx_props, sub_schema, sub_instance, quit_on_first_error))
                    annotation.append (property_name);
                else
                    all_valid = false;
                */
                if (!validate_subschema(ctx_props, sub_schema, sub_instance, quit_on_first_error))
                    all_valid = false;
                annotation.append (property_name);

                ctx_props.pop_schema_path ();
                ctx_props.pop_instance_path ();
            }
        }

        ctx_props.set_valid (all_valid);

        ctx.annotate (annotation);

        if (all_valid) {
            ctx.collect_annotations (ctx_props);
            //ctx.annotate (annotation);
            ctx_props.output_unit["annotation"] = annotation;
        }else{
            ctx.set_valid (false);
            ctx_props.set_error ("Not all properties evaluated true.");
        }
        ctx.add_output_unit (std::move(ctx_props.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.3. Keywords for Applying Subschemas to Child Instances
    //
    // Applicator keywords for objects
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_additionalProperties (validation_context& ctx,
                                                                jvalue& schema,
                                                                jvalue& schema_value,
                                                                jvalue& instance,
                                                                const bool quit_on_first_error)
    {
        bool all_valid = true;
        std::string ctx_instance_path = ctx.instance_path().str ();
        jvalue annotation (j_array);

        bool all_checked = false;
        std::set<std::string> checked_props;
        auto props_pos = ctx.annotations.find (std::make_pair("properties", ctx_instance_path));
        if (props_pos != ctx.annotations.end()) {
            if (props_pos->second.type() == j_bool) {
                all_checked = true;
            }
            else if (props_pos->second.type() == j_array) {
                for (auto item : props_pos->second.array()) {
                    checked_props.emplace (item.str());
                }
            }
        }
        auto pprops_pos = ctx.annotations.find (std::make_pair("patternProperties", ctx_instance_path));
        if (pprops_pos != ctx.annotations.end()) {
            if (pprops_pos->second.type() == j_bool) {
                all_checked = true;
            }
            else if (pprops_pos->second.type() == j_array) {
                for (auto item : pprops_pos->second.array()) {
                    checked_props.emplace (item.str());
                }
            }
        }

        if (all_checked)
            return true;

        validation_context ctx_props (ctx);
        unsigned num_checked = 0;

        for (auto& property : instance.obj()) {

            if (quit_on_first_error && all_valid==false)
                break;

            auto& property_name = property.first;
            auto& sub_instance = property.second;

            auto pos = checked_props.find (property_name);
            if (pos != checked_props.end())
                continue;

            ++num_checked;
            ctx_props.push_instance_path (property_name);

            if (validate_subschema(ctx_props, schema_value, sub_instance, quit_on_first_error))
                annotation.append (property_name);
            else
                all_valid = false;

            ctx_props.pop_instance_path ();
        }

        if (num_checked == 0)
            return true;

        ctx_props.set_valid (all_valid);
        if (all_valid) {
            ctx.collect_annotations (ctx_props);
            ctx.annotate (annotation);
            ctx_props.output_unit["annotation"] = annotation;
        }else{
            ctx.set_valid (false);
            ctx_props.set_error ("Not all properties evaluated true.");
        }
        ctx.add_output_unit (std::move(ctx_props.output_unit));

        return all_valid;
    }


    //--------------------------------------------------------------------------
    // 10.3. Keywords for Applying Subschemas to Child Instances
    //
    // Applicator keywords for objects
    //--------------------------------------------------------------------------
    bool jvocabulary_applicator::validate_propertyNames (validation_context& ctx,
                                                         jvalue& schema,
                                                         jvalue& schema_value,
                                                         jvalue& instance,
                                                         const bool quit_on_first_error)
    {
        bool all_valid = true;

        validation_context ctx_props (ctx);

        for (auto& entry : instance.obj()) {

            if (quit_on_first_error && all_valid==false)
                break;

            auto& property_name = entry.first;
            jvalue name_instance (property_name);

            ctx_props.push_instance_path (property_name);
            if (validate_subschema(ctx_props, schema_value, name_instance, quit_on_first_error) == false)
                all_valid = false;
            ctx_props.pop_instance_path ();
        }

        ctx_props.set_valid (all_valid);
        if (all_valid == false) {
            ctx.set_valid (false);
            ctx_props.set_error ("Not all property names evaluated true.");
        }
        ctx.add_output_unit (std::move(ctx_props.output_unit));

        return all_valid;
    }


}
