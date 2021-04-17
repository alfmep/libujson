/*
 * Copyright (C) 2021 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_VALIDATOR_HPP
#define UJSON_VALIDATOR_HPP

#include <ujson/jvalue.hpp>
#include <string>
#include <set>


namespace ujson {

    /**
     *
     */
    class Validator {
    public:
        enum result_t {
            //in_progress = -1, // Validation in progress
            valid=0,      // Instance was validated
            not_valid,    // Instance was not validated
            err_schema,   // Generic schema error, not a valid schema
            err_instance, // Not an instance
        };

        /**
         * Constructor.
         */
        Validator (jvalue& schema);

        /**
         * Destructor.
         */
        ~Validator () = default;

        /**
         * @return Validator::valid on success.
         */
        result_t validate (jvalue& instance);

    private:
        struct vdata_t {
            jvalue& schema;
            jvalue& instance;
            result_t result;

            bool have_if;
            bool if_result;

            bool have_prefix_items;
            size_t prefix_items_size;

            bool have_properties;
            bool have_pattern_properties;
            std::set<std::string> additional_properties;

            vdata_t (jvalue& s, jvalue& i);
        };

        jvalue root_schema;
        jvalue* get_ref_schema (const std::string& ref);
        result_t validate_impl (jvalue& schema, jvalue& instance);

        // Core - all types
        void handle_allOf (vdata_t& vdata, jvalue& value);
        void handle_anyOf (vdata_t& vdata, jvalue& value);
        void handle_oneOf (vdata_t& vdata, jvalue& value);
        void handle_not (vdata_t& vdata, jvalue& value);
        void handle_if (vdata_t& vdata, jvalue& value);
        void handle_then (vdata_t& vdata, jvalue& value);
        void handle_else (vdata_t& vdata, jvalue& value);
        // Core - objects
        void handle_dependentSchemas (vdata_t& vdata, jvalue& value);
        void handle_properties (vdata_t& vdata, jvalue& value);
        void handle_patternProperties (vdata_t& vdata, jvalue& value);
        void handle_additionalProperties (vdata_t& vdata, jvalue& value);
        void handle_propertyNames (vdata_t& vdata, jvalue& value);
        // Core - arrays
        void handle_prefixItems (vdata_t& vdata, jvalue& value);
        void handle_items (vdata_t& vdata, jvalue& value);
        void handle_contains (vdata_t& vdata, jvalue& value);
        // Validation - all types
        void handle_type_keyword (vdata_t& vdata, jvalue& value);
        void handle_enum (vdata_t& vdata, jvalue& value);
        void handle_const (vdata_t& vdata, jvalue& value);
        // Validation - numbers
        void handle_multipleOf (vdata_t& vdata, jvalue& value);
        void handle_maximum (vdata_t& vdata, jvalue& value);
        void handle_exclusiveMaximum (vdata_t& vdata, jvalue& value);
        void handle_minimum (vdata_t& vdata, jvalue& value);
        void handle_exclusiveMinimum (vdata_t& vdata, jvalue& value);
        // Validation - strings
        void handle_maxLength (vdata_t& vdata, jvalue& value);
        void handle_minLength (vdata_t& vdata, jvalue& value);
        void handle_pattern (vdata_t& vdata, jvalue& value);
        // Validation - arrays
        void handle_maxItems (vdata_t& vdata, jvalue& value);
        void handle_minItems (vdata_t& vdata, jvalue& value);
        void handle_uniqueItems (vdata_t& vdata, jvalue& value);
        void handle_maxContains (vdata_t& vdata, jvalue& value);
        void handle_minContains (vdata_t& vdata, jvalue& value);
        // Validation - objects
        void handle_maxProperties (vdata_t& vdata, jvalue& value);
        void handle_minProperties (vdata_t& vdata, jvalue& value);
        void handle_required (vdata_t& vdata, jvalue& value);
        void handle_dependentRequired (vdata_t& vdata, jvalue& value);
        // Validation - content
        void handle_contentSchema (vdata_t& vdata, jvalue& value);
    };


}
#endif
