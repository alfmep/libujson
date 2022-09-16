/*
 * Copyright (C) 2021,2022 Dan Arrhenius <dan@ultramarin.se>
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
#include <ujson/internal.hpp>
#include <ujson/Schema.hpp>
#include <ujson/utils.hpp>
#include <regex>
#include <cmath>


#if 0
#include <iostream>
#define DBG_ERR_SCHEMA do{std::cerr<<"TRACE ERROR Invalid schema at "<<__FUNCTION__<<':'<<__LINE__<<std::endl;}while(false)
#define DBG_TRACE do{std::cerr<<"TRACE "<<__FUNCTION__<<':'<<__LINE__<<std::endl;}while(false)
#else
#define DBG_ERR_SCHEMA do{;}while(false)
#define DBG_TRACE do{;}while(false)
#endif


namespace ujson {

    //
    // Schema keywords mapped to methods to handle the keyword implementation
    //
    const Schema::schema_handler_map_t Schema::schema_handlers = {
        //
        // Core keywords for all instance types (here, j_invalid means no specific type)
        //
        {"allOf", {j_invalid, &Schema::handle_allOf}},
        {"anyOf", {j_invalid, &Schema::handle_anyOf}},
        {"oneOf", {j_invalid, &Schema::handle_oneOf}},
        {"not",   {j_invalid, &Schema::handle_not}},
        {"if",    {j_invalid, &Schema::handle_if}},
        {"then",  {j_invalid, &Schema::handle_then}},
        {"else",  {j_invalid, &Schema::handle_else}},
        // Validation keywords for all instance types
        {"type",  {j_invalid, &Schema::handle_type_keyword}},
        {"enum",  {j_invalid, &Schema::handle_enum}},
        {"const", {j_invalid, &Schema::handle_const}},
        //
        // Validation Keywords for Numeric Instances
        //
        {"multipleOf",       {j_number, &Schema::handle_multipleOf}},
        {"maximum",          {j_number, &Schema::handle_maximum}},
        {"exclusiveMaximum", {j_number, &Schema::handle_exclusiveMaximum}},
        {"minimum",          {j_number, &Schema::handle_minimum}},
        {"exclusiveMinimum", {j_number, &Schema::handle_exclusiveMinimum}},
        //
        // Validation Keywords for strings
        //
        {"maxLength", {j_string, &Schema::handle_maxLength}},
        {"minLength", {j_string, &Schema::handle_minLength}},
        {"pattern",   {j_string, &Schema::handle_pattern}},
        //
        // Keywords for objects
        //
        {"dependentSchemas",     {j_object, &Schema::handle_dependentSchemas}},
        {"properties",           {j_object, &Schema::handle_properties}},
        {"patternProperties",    {j_object, &Schema::handle_patternProperties}},
        {"additionalProperties", {j_object, &Schema::handle_additionalProperties}},
        {"propertyNames",        {j_object, &Schema::handle_propertyNames}},
        // Validations
        {"maxProperties",        {j_object, &Schema::handle_maxProperties}},
        {"minProperties",        {j_object, &Schema::handle_minProperties}},
        {"required",             {j_object, &Schema::handle_required}},
        {"dependentRequired",    {j_object, &Schema::handle_dependentRequired}},
        //
        // Keywords for arrays
        //
        {"prefixItems", {j_array, &Schema::handle_prefixItems}},
        {"items",       {j_array, &Schema::handle_items}},
        {"contains",    {j_array, &Schema::handle_contains}},
        {"maxItems",    {j_array, &Schema::handle_maxItems}},
        {"minItems",    {j_array, &Schema::handle_minItems}},
        {"uniqueItems", {j_array, &Schema::handle_uniqueItems}},
        {"maxContains", {j_array, &Schema::handle_maxContains}},
        {"minContains", {j_array, &Schema::handle_minContains}},
    };



    Schema::vdata_t::vdata_t (jvalue& s, jvalue& i)
        : schema (s),
          instance (i)
    {
        result = valid;
        have_if = if_result = false;
        have_prefix_items = false;
        prefix_items_size = 0;
        have_properties = false;
        have_pattern_properties = false;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
#if UJSON_HAVE_GMPXX
    static bool is_integer (const mpf_class& value)
    {
        return mpf_integer_p(value.get_mpf_t()) != 0;
    }
#else
    static bool is_integer (const double value)
    {
        double i;
        // We don't count NaN or infinity as an integer
        if (!std::isinf(value) && !std::isnan(value))
            return modf(value, &i) == (double)0.0;
        else
            return false;
    }
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool is_non_negative_integer (jvalue& value)
    {
        if (value.type() != j_number)
            return false;
#if UJSON_HAVE_GMPXX
        return is_integer(value.mpf())  &&  value.mpf() >= 0;
#else
        return is_integer(value.num())  &&  value.num() >= 0;
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Schema::Schema (const jvalue& root)
        : root_schema (root)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Schema::Schema (jvalue&& root)
        : root_schema (std::forward<jvalue&&>(root))
    {
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Schema::result_t Schema::validate (const jvalue& instance)
    {
        return validate_impl (root_schema, const_cast<jvalue&>(instance));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool Schema::add_ref_schema (const Schema& schema)
    {
        auto& id = find_jvalue (const_cast<jvalue&>(schema.root_schema), "/$id");
        if (id.type() != j_string)
            return false;

        ref_schemas.emplace (id.str(), Schema(schema));
        ref_cache.clear ();
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Schema::del_ref_schema (const std::string& id)
    {
        ref_schemas.erase (id);
        ref_cache.clear ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& Schema::root ()
    {
        return root_schema;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool is_schema_type (const jvalue& schema)
    {
        return schema.type()==j_object || schema.type()==j_bool;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Schema::result_t Schema::handle_ref (const std::string& ref, jvalue& instance)
    {
        auto entry = ref_cache.find (ref);
        if (entry != ref_cache.end()) {
            Schema& ref_root = entry->second.first;
            jvalue& ref_schema = entry->second.second;
            return ref_root.validate_impl (ref_schema, instance);
        }

        std::string id;
        std::string pointer;

        auto pos = ref.find ("#");
        if (pos == std::string::npos) {
            pointer = ref;
        }else{
            id = ref.substr (0, pos);
            pointer = ref.substr (pos+1);
        }
        pointer.insert (0, 1, '/'); // Make sure the pointer starts with /

        if (id.empty()) {
            // Reference to pointer in this schema
            auto& ref_schema = find_jvalue (root_schema, pointer);
            if (ref_schema.valid()) {
                ref_cache.emplace (ref,
                                   std::make_pair(std::reference_wrapper<Schema>(*this),
                                                  std::reference_wrapper<jvalue>(ref_schema)));
            }
            return validate_impl (ref_schema, instance);
        }else{
            // Reference to pointer in other schema
            auto entry = ref_schemas.find (id);
            if (entry == ref_schemas.end()) {
                return err_schema;
            }
            auto& ref_root = entry->second;
            auto& ref_schema = find_jvalue (ref_root.root_schema, pointer);
            if (ref_schema.valid()) {
                ref_cache.emplace (ref,
                                   std::make_pair(std::reference_wrapper<Schema>(ref_root),
                                                  std::reference_wrapper<jvalue>(ref_schema)));
            }
            return ref_root.validate_impl (ref_schema, instance);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Schema::result_t Schema::validate_impl (jvalue& schema, jvalue& instance)
    {
        vdata_t vdata (schema, instance);

        // Sanity checks
        //
        if (!instance.valid())
            return err_instance;
        if (!is_schema_type(schema)) {
            DBG_ERR_SCHEMA;
            return err_schema;
        }

        // Boolean schema: true or false
        //
        if (schema.type() == j_bool)
            return schema.boolean() ? valid : not_valid;

        //
        // Handle keyword "$ref" first
        //
        auto& ref = schema.get("$ref");
        if (ref.type() != j_invalid)
            return handle_ref (ref.str(), instance);

        // Handle the schema keywords
        //
        for (auto& member : schema.obj()) {
            auto& keyword = member.first;
            auto& value = member.second;

            const auto& entry = schema_handlers.find (keyword);
            if (entry != schema_handlers.end()) {
                auto handler_type = entry->second.first;
                auto handler_func = entry->second.second;
                if (handler_type==j_invalid || handler_type==instance.type())
                    (this->*handler_func) (vdata, value);
            }
            if (vdata.result != valid)
                return vdata.result;
        }
        return valid;
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //
    // 10.2.1.1.
    // This keyword's value MUST be a non-empty array.  Each item of the
    // array MUST be a valid JSON Schema.
    //
    // An instance validates successfully against this keyword if it
    // validates successfully against all schemas defined by this keyword's
    // value.
    //--------------------------------------------------------------------------
    void Schema::handle_allOf (vdata_t& vdata, jvalue& value)
    {
        if (value.array().empty()) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        for (auto& schema : value.array()) {
            vdata.result = validate_impl (schema, vdata.instance);
            if (vdata.result != valid)
                return;
        }
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    void Schema::handle_anyOf (vdata_t& vdata, jvalue& value)
    {
        // 10.2.1.2. This keyword's value MUST be a non-empty array.
        if (value.array().empty()) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        for (auto& schema : value.array()) {
            vdata.result = validate_impl (schema, vdata.instance);
            if (vdata.result == valid) {
                break;
            }
        }
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    void Schema::handle_oneOf (vdata_t& vdata, jvalue& value)
    {
        // 10.2.1.2. This keyword's value MUST be a non-empty array.
        if (value.array().empty()) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        unsigned ok = 0;
        for (auto& schema : value.array()) {
            if (validate_impl(schema, vdata.instance) == valid) {
                if (++ok > 1)
                    break;
            }
        }
        if (ok != 1)
            vdata.result = not_valid;
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    void Schema::handle_not (vdata_t& vdata, jvalue& value)
    {
        auto result = validate_impl (value, vdata.instance);
        if (result == not_valid) {
            vdata.result = valid;
        }
        else if (result == valid) {
            vdata.result = not_valid;
        }
        else {
            vdata.result = result; // Unexpected error
        }
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    void Schema::handle_if (vdata_t& vdata, jvalue& value)
    {
        if (vdata.have_if)
            return;
        auto result = validate_impl (value, vdata.instance);
        vdata.have_if = true;
        if (result == valid) {
            vdata.if_result = true;
        }
        else if (result == not_valid) {
            vdata.if_result = false;
        }
        else {
            vdata.result = result; // Unexpected error
        }
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    void Schema::handle_then (vdata_t& vdata, jvalue& value)
    {
        // 10.2.2.2. This keyword's value MUST be a valid JSON Schema
        if (!is_schema_type(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        if (!vdata.have_if) {
            if (vdata.schema.have("if")) {
                handle_if (vdata, vdata.schema.get("if"));
                if (vdata.result != valid)
                    return;
            }else{
                return;
            }
        }
        if (vdata.have_if && vdata.if_result)
            vdata.result = validate_impl (value, vdata.instance);
    }


    //--------------------------------------------------------------------------
    // Any instance type
    //--------------------------------------------------------------------------
    void Schema::handle_else (vdata_t& vdata, jvalue& value)
    {
        // 10.2.2.3. This keyword's value MUST be a valid JSON Schema
        if (!is_schema_type(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        if (!vdata.have_if) {
            if (vdata.schema.have("if")) {
                handle_if (vdata, vdata.schema.get("if"));
                if (vdata.result != valid)
                    return;
            }else{
                return;
            }
        }
        if (vdata.have_if && !vdata.if_result)
            vdata.result = validate_impl (value, vdata.instance);
    }


    //--------------------------------------------------------------------------
    // Only object instances
    //--------------------------------------------------------------------------
    void Schema::handle_dependentSchemas (vdata_t& vdata, jvalue& value)
    {
        // 10.2.2.4. This keyword's value MUST be an object
        if (value.type() != j_object) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        for (auto& i : value.obj()) {
            auto& property  = i.first;
            auto& sub_schema = i.second;
            if (vdata.instance.have(property)) {
                vdata.result = validate_impl (sub_schema, vdata.instance);
                if (vdata.result != valid)
                    return;
            }
        }
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_prefixItems (vdata_t& vdata, jvalue& value)
    {
        if (vdata.have_prefix_items)
            return;

        // 10.3.1.1. The value of "prefixItems" MUST be a non-empty array of valid JSON Schemas.
        if (value.array().empty()) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        json_array& sub_schema = value.array ();
        json_array& sub_instance = vdata.instance.array ();
        if (sub_instance.size() < sub_schema.size()) {
            // Each element of the instance cannot validate against
            // each element of the keyword values since there aren't
            // enough instance elements.
            vdata.result = not_valid;
            return;
        }

        vdata.have_prefix_items = true;
        vdata.prefix_items_size = sub_schema.size ();
        for (size_t i=0; i<vdata.prefix_items_size; ++i) {
            vdata.result = validate_impl (sub_schema[i], sub_instance[i]);
            if (vdata.result != valid)
                return;
        }
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_items (vdata_t& vdata, jvalue& value)
    {
        // 10.3.1.2. The value of "items" MUST be a valid JSON Schema.
        if (!is_schema_type(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        if (!vdata.have_prefix_items) {
            auto& prefix_items = vdata.schema.get("prefixItems");
            if (prefix_items.type() == j_array)
                vdata.prefix_items_size = prefix_items.size ();
        }

        json_array& sub_instance = vdata.instance.array ();
        for (size_t i=vdata.prefix_items_size; i<sub_instance.size(); ++i) {
            vdata.result = validate_impl (value, sub_instance[i]);
            if (vdata.result != valid)
                return;
        }
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_contains (vdata_t& vdata, jvalue& value)
    {
        // 10.3.1.3. The value of this keyword MUST be a valid JSON Schema.
        if (!is_schema_type(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        json_array& sub_instance = vdata.instance.array ();

        for (size_t i=0; i<sub_instance.size(); ++i) {
            vdata.result = validate_impl (value, sub_instance[i]);
            if (vdata.result != not_valid)
                return;
        }
        vdata.result = not_valid;
    }


    //--------------------------------------------------------------------------
    // Only object instances
    //--------------------------------------------------------------------------
    void Schema::handle_properties (vdata_t& vdata, jvalue& value)
    {
        if (vdata.have_properties)
            return;

        // 10.3.2.1. The value of "properties" MUST be an object.
        if (value.type() != j_object) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        vdata.have_properties = true;
        if (!vdata.have_pattern_properties) {
            for (auto& i : vdata.instance.obj())
                vdata.additional_properties.emplace (i.first);
        }

        for (auto& member : value.obj()) {
            auto& name = member.first;
            auto& sub_schema = member.second;
            auto& sub_instance = vdata.instance.get(name);
            vdata.additional_properties.erase (name);
            if (sub_instance.type() != j_invalid) {
                auto result = validate_impl (sub_schema, sub_instance);
                if (result != valid) {
                    vdata.result = result;
                }
            }
        }
    }


    //--------------------------------------------------------------------------
    // Only object instances
    //--------------------------------------------------------------------------
    void Schema::handle_patternProperties (vdata_t& vdata, jvalue& value)
    {
        if (vdata.have_pattern_properties)
            return;

        try {
            // 10.3.2.2. The value of "patternProperties" MUST be an object.
            if (value.type() != j_object) {
                DBG_ERR_SCHEMA;
                vdata.result = err_schema;
                return;
            }

            vdata.have_pattern_properties = true;
            if (!vdata.have_properties) {
                for (auto& i : vdata.instance.obj())
                    vdata.additional_properties.emplace (i.first);
            }

            for (auto& member : value.obj()) {
                auto& expr = member.first;
                auto& sub_schema = member.second;

                std::regex re (expr, std::regex::ECMAScript);
                std::cmatch cm;

                for (auto& instance_member : vdata.instance.obj()) {
                    auto& sub_instance_name = instance_member.first;
                    auto& sub_instance = instance_member.second;

                    if (std::regex_match(sub_instance_name.c_str(), cm, re)) {
                        vdata.additional_properties.erase (sub_instance_name);
                        auto result = validate_impl (sub_schema, sub_instance);
                        if (result != valid)
                            vdata.result = result;
                    }
                }
            }
        }
        catch (std::regex_error& re) {
            vdata.result = err_schema;
        }
    }


    //--------------------------------------------------------------------------
    // Only object instances
    //--------------------------------------------------------------------------
    void Schema::handle_additionalProperties (vdata_t& vdata, jvalue& value)
    {
        // 10.3.2.3. The value of "additionalProperties" MUST be a valid JSON Schema.
        if (!is_schema_type(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        if (!vdata.have_properties) {
            //
            // Handle keyword 'properties' first, if any
            //
            auto& v = vdata.schema.get("properties");
            if (v.type() == j_object) {
                handle_properties (vdata, v);
                if (vdata.result != valid)
                    return;
            }
        }
        if (!vdata.have_pattern_properties) {
            //
            // Handle keyword 'patternProperties' first, if any
            //
            auto& v = vdata.schema.get("patternProperties");
            if (v.type() == j_object) {
                handle_patternProperties (vdata, v);
                if (vdata.result != valid)
                    return;
            }
        }

        if (vdata.have_properties || vdata.have_pattern_properties) {
            // Valdate instance members that are left after
            // 'properties' and 'patternProperties' is done
            for (auto& property : vdata.additional_properties) {
                vdata.result = validate_impl (value, vdata.instance.get(property));
                if (vdata.result != valid) {
                    return;
                }
            }
        }else{
            // We have neither 'properties' nor 'patternProperties',
            // check all instance members
            for (auto& i : vdata.instance.obj()) {
                vdata.result = validate_impl (value, i.second);
                if (vdata.result != valid) {
                    return;
                }
            }
        }
    }


    //--------------------------------------------------------------------------
    // Only object instances
    //--------------------------------------------------------------------------
    void Schema::handle_propertyNames (vdata_t& vdata, jvalue& value)
    {
        // 10.3.2.4. The value of "propertyNames" MUST be a valid JSON Schema.
        if (!is_schema_type(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        for (auto& member : vdata.instance.obj()) {
            jvalue name_instance (member.first);
            vdata.result = validate_impl (value, name_instance);
            if (vdata.result != valid)
                return;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static Schema::result_t check_instance_type (const std::string& type_name, jvalue& instance)
    {
        if (type_name == "object" && instance.type() == j_object)
            return Schema::valid;

        if (type_name == "array" && instance.type() == j_array)
            return Schema::valid;

        if (type_name == "string" && instance.type() == j_string)
            return Schema::valid;

        if (type_name == "number" && instance.type() == j_number)
            return Schema::valid;

        if (type_name == "boolean" && instance.type() == j_bool)
            return Schema::valid;

        if (type_name == "null" && instance.type() == j_null)
            return Schema::valid;

        if (type_name == "integer" && instance.type() == j_number) {
#if UJSON_HAVE_GMPXX
            if (is_integer(instance.mpf()))
                return Schema::valid;
#else
            if (is_integer(instance.num()))
                return Schema::valid;
#endif
        }

        return Schema::not_valid;
    }


    //--------------------------------------------------------------------------
    // Any instance
    //--------------------------------------------------------------------------
    void Schema::handle_type_keyword (vdata_t& vdata, jvalue& value)
    {
        if (value.type() == j_string) {
            vdata.result = check_instance_type (value.str(), vdata.instance);
        }
        else if (value.type() == j_array) {
            for(auto& i : value.array()) {
                if (i.type() != j_string) {
                    DBG_ERR_SCHEMA;
                    vdata.result = err_schema;
                    break;
                }
                vdata.result = check_instance_type (i.str(), vdata.instance);
                if (vdata.result == valid)
                    break;
            }
        }
        else {
            // 6.1.1. The value of this keyword MUST be either a string or an array.
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
        }
    }


    //--------------------------------------------------------------------------
    // Any instance
    //--------------------------------------------------------------------------
    void Schema::handle_enum (vdata_t& vdata, jvalue& value)
    {
        // 6.1.2. The value of this keyword MUST be an array.
        if (value.type() != j_array) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        for (auto& element : value.array()) {
            if (element == vdata.instance) {
                vdata.result = valid;
                return;
            }
        }
        vdata.result = not_valid;
    }


    //--------------------------------------------------------------------------
    // Any instance
    //--------------------------------------------------------------------------
    void Schema::handle_const (vdata_t& vdata, jvalue& value)
    {
        if (value == vdata.instance)
            vdata.result = valid;
        else
            vdata.result = not_valid;
    }


    //--------------------------------------------------------------------------
    // Only numeric instances
    //--------------------------------------------------------------------------
    void Schema::handle_multipleOf (vdata_t& vdata, jvalue& value)
    {
        // 6.2.1. The value of "multipleOf" MUST be a number, strictly greater than 0.
        if (value.type() != j_number) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
#if UJSON_HAVE_GMPXX
        auto& vdata_num = vdata.instance.mpf ();
        auto& value_num = value.mpf ();
#else
        auto vdata_num = vdata.instance.num ();
        auto& value_num = value.num ();
#endif
        if (value_num <= 0) {
            // 6.2.1. The value of "multipleOf" MUST be a number, strictly greater than 0.
            vdata.result = err_schema;
        }
        else if (is_integer(vdata_num / value_num)) {
            vdata.result = valid;
        }else{
            vdata.result = not_valid;
        }
    }


    //--------------------------------------------------------------------------
    // Only numeric instances
    //--------------------------------------------------------------------------
    void Schema::handle_maximum (vdata_t& vdata, jvalue& value)
    {
        // 6.2.2. The value of "maximum" MUST be a number.
        if (value.type() != j_number) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
#if UJSON_HAVE_GMPXX
        if (vdata.instance.mpf() <= value.mpf())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#else
        if (vdata.instance.num() <= value.num())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    // Only numeric instances
    //--------------------------------------------------------------------------
    void Schema::handle_exclusiveMaximum (vdata_t& vdata, jvalue& value)
    {
        // 6.2.3. The value of "exclusiveMaximum" MUST be a number.
        if (value.type() != j_number) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
#if UJSON_HAVE_GMPXX
        if (vdata.instance.mpf() < value.mpf())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#else
        if (vdata.instance.num() < value.num())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    // Only numeric instances
    //--------------------------------------------------------------------------
    void Schema::handle_minimum (vdata_t& vdata, jvalue& value)
    {
        // 6.2.4. The value of "minimum" MUST be a number.
        if (value.type() != j_number) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
#if UJSON_HAVE_GMPXX
        if (vdata.instance.mpf() >= value.mpf())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#else
        if (vdata.instance.num() >= value.num())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    // Only numeric instances
    //--------------------------------------------------------------------------
    void Schema::handle_exclusiveMinimum (vdata_t& vdata, jvalue& value)
    {
        // 6.2.5. The value of "exclusiveMinimum" MUST be a number.
        if (value.type() != j_number) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
#if UJSON_HAVE_GMPXX
        if (vdata.instance.mpf() > value.mpf())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#else
        if (vdata.instance.num() > value.num())
            vdata.result = valid;
        else
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static size_t simple_utf8_len (const std::string& str)
    {
        size_t retval = 0;
        for (size_t i=0; i<str.length(); ++i)
            if ((str[i] & 0xc0) != 0x80)
                ++retval;
        return retval;
    }


    //--------------------------------------------------------------------------
    // Only string instances
    //--------------------------------------------------------------------------
    void Schema::handle_maxLength (vdata_t& vdata, jvalue& value)
    {
        // 6.3.1. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

#if UJSON_HAVE_GMPXX
        size_t max_len = (size_t) value.mpf().get_ui ();
#else
        size_t max_len = (size_t) value.num ();
#endif
        if (simple_utf8_len(vdata.instance.str()) > max_len)
            vdata.result = not_valid;
    }


    //--------------------------------------------------------------------------
    // Only string instances
    //--------------------------------------------------------------------------
    void Schema::handle_minLength (vdata_t& vdata, jvalue& value)
    {
        // 6.3.2. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

#if UJSON_HAVE_GMPXX
        size_t min_len = (size_t) value.mpf().get_ui ();
#else
        size_t min_len = (size_t) value.num ();
#endif
        if (simple_utf8_len(vdata.instance.str()) < min_len)
            vdata.result = not_valid;
    }


    //--------------------------------------------------------------------------
    // Only string instances
    //--------------------------------------------------------------------------
    void Schema::handle_pattern (vdata_t& vdata, jvalue& value)
    {
        try {
            // 6.3.3. The value of this keyword MUST be a string.
            if (value.type() != j_string) {
                DBG_ERR_SCHEMA;
                vdata.result = err_schema;
                return;
            }

            std::regex re (value.str(), std::regex::ECMAScript);
            std::cmatch cm;
            if (!std::regex_match(vdata.instance.str().c_str(), cm, re))
                vdata.result = not_valid;
        }
        catch (std::regex_error& re) {
            vdata.result = err_schema;
        }
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_maxItems (vdata_t& vdata, jvalue& value)
    {
        // 6.4.1. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

#if UJSON_HAVE_GMPXX
        if (vdata.instance.size() > (size_t)value.mpf().get_ui())
            vdata.result = not_valid;
#else
        if (vdata.instance.size() > (size_t)value.num())
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_minItems (vdata_t& vdata, jvalue& value)
    {
        // 6.4.2. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

#if UJSON_HAVE_GMPXX
        if (vdata.instance.size() < (size_t)value.mpf().get_ui())
            vdata.result = not_valid;
#else
        if (vdata.instance.size() < (size_t)value.num())
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_uniqueItems (vdata_t& vdata, jvalue& value)
    {
        if (value.type() != j_bool) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
        if (value.boolean() == false)
            return;
        // Not implemented yet
        /*
          DBG_ERR_SCHEMA;
          vdata.result = err_schema;
        */
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_maxContains (vdata_t& vdata, jvalue& value)
    {
        // 6.4.4. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        // Not implemented yet
        DBG_ERR_SCHEMA;
        vdata.result = err_schema;
    }


    //--------------------------------------------------------------------------
    // Only array instances
    //--------------------------------------------------------------------------
    void Schema::handle_minContains (vdata_t& vdata, jvalue& value)
    {
        // 6.4.5. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        // Not implemented yet
        DBG_ERR_SCHEMA;
        vdata.result = err_schema;
    }


    //--------------------------------------------------------------------------
    // Instance is an object
    //--------------------------------------------------------------------------
    void Schema::handle_maxProperties (vdata_t& vdata, jvalue& value)
    {
        // 6.5.1. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
#if UJSON_HAVE_GMPXX
        if (vdata.instance.size() > (size_t)value.mpf().get_ui())
            vdata.result = not_valid;
#else
        if (vdata.instance.size() > (size_t)value.num())
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    // Instance is an object
    //--------------------------------------------------------------------------
    void Schema::handle_minProperties (vdata_t& vdata, jvalue& value)
    {
        // 6.5.2. The value of this keyword MUST be a non-negative integer.
        if (!is_non_negative_integer(value)) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }
#if UJSON_HAVE_GMPXX
        if (vdata.instance.size() < (size_t)value.mpf().get_ui())
            vdata.result = not_valid;
#else
        if (vdata.instance.size() < (size_t)value.num())
            vdata.result = not_valid;
#endif
    }


    //--------------------------------------------------------------------------
    // Instance is an object
    //--------------------------------------------------------------------------
    void Schema::handle_required (vdata_t& vdata, jvalue& value)
    {
        // 6.5.3. The value of this keyword MUST be an array.
        if (value.type() != j_array) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        for (auto& i : value.array()) {
            if (i.type() != j_string) {
                DBG_ERR_SCHEMA;
                vdata.result = err_schema;
                return;
            }
            if (!vdata.instance.have(i.str())) {
                vdata.result = not_valid;
                return;
            }
        }
    }


    //--------------------------------------------------------------------------
    // Instance is an object
    //--------------------------------------------------------------------------
    void Schema::handle_dependentRequired (vdata_t& vdata, jvalue& value)
    {
        // 6.5.4. The value of this keyword MUST be an object.
        if (value.type() != j_object) {
            DBG_ERR_SCHEMA;
            vdata.result = err_schema;
            return;
        }

        for (auto& member : value.obj()) {
            auto& property = member.first;
            auto& names = member.second;
            if (names.type() != j_array) {
                DBG_ERR_SCHEMA;
                vdata.result = err_schema;
                return;
            }
            if (vdata.instance.have(property)) {
                for (auto& i : names.array()) {
                    if (i.type() != j_string) {
                        DBG_ERR_SCHEMA;
                        vdata.result = err_schema;
                        return;
                    }
                    if (!vdata.instance.have(i.str())) {
                        vdata.result = not_valid;
                        return;
                    }
                }
            }
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Schema::handle_contentSchema (vdata_t& vdata, jvalue& value)
    {
        // Not implemented yet
        DBG_ERR_SCHEMA;
        vdata.result = err_schema;
    }


}
