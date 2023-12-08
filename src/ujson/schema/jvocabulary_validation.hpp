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
#ifndef UJSON_SCHEMA_JVOCABULARY_VALIDATION_HPP
#define UJSON_SCHEMA_JVOCABULARY_VALIDATION_HPP

#include <ujson/schema/jvocabulary.hpp>
#include <string>
#include <tuple>
#include <map>


namespace ujson::schema {


    /**
     * JSON schema validation vocabulary.
     */
    class jvocabulary_validation : public jvocabulary {
    public:
        jvocabulary_validation (jschema& schema_arg);

        virtual void load (jvalue& schema, jvalue& load_ctx);
        virtual bool validate (validation_context& ctx, jvalue& schema, jvalue& instance);


    private:
        void load_kw_type_is_number (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_kw_type_is_positive_number (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_single_type (jvalue& schema_value, jvalue& load_ctx);
        void load_type (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_enum (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_multipleOf (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_pattern (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_uniqueItems (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_required (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);
        void load_dependentRequired (const std::string& keyword, jvalue& schema_value, jvalue& load_ctx);

        // Validation keywords for any instance
        bool validate_type (validation_context& ctx,
                            jvalue& schema, jvalue& schema_value,
                            jvalue& instance, std::string& error_msg);
        bool validate_type_impl (validation_context& ctx,
                                 jvalue& schema, std::string& type_name,
                                 jvalue& instance, std::string& error_msg,
                                 bool set_error_msg=true);
        bool validate_enum (validation_context& ctx,
                            jvalue& schema, jvalue& schema_value,
                            jvalue& instance, std::string& error_msg);
        bool validate_const (validation_context& ctx,
                             jvalue& schema, jvalue& schema_value,
                             jvalue& instance, std::string& error_msg);

        // Validation keywords for numeric instances
        bool validate_multipleOf (validation_context& ctx,
                                  jvalue& schema, jvalue& schema_value,
                                  jvalue& instance, std::string& error_msg);
        bool validate_maximum (validation_context& ctx,
                               jvalue& schema, jvalue& schema_value,
                               jvalue& instance, std::string& error_msg);
        bool validate_exclusiveMaximum (validation_context& ctx,
                                        jvalue& schema, jvalue& schema_value,
                                        jvalue& instance, std::string& error_msg);
        bool validate_minimum (validation_context& ctx,
                               jvalue& schema, jvalue& schema_value,
                               jvalue& instance, std::string& error_msg);
        bool validate_exclusiveMinimum (validation_context& ctx,
                                        jvalue& schema, jvalue& schema_value,
                                        jvalue& instance, std::string& error_msg);

        // Validation keywords for strings
        bool validate_maxLength (validation_context& ctx,
                                 jvalue& schema, jvalue& schema_value,
                                 jvalue& instance, std::string& error_msg);
        bool validate_minLength (validation_context& ctx,
                                 jvalue& schema, jvalue& schema_value,
                                 jvalue& instance, std::string& error_msg);
        bool validate_pattern (validation_context& ctx,
                               jvalue& schema, jvalue& schema_value,
                               jvalue& instance, std::string& error_msg);

        // Validation keywords for arrays
        bool validate_maxItems (validation_context& ctx,
                                jvalue& schema, jvalue& schema_value,
                                jvalue& instance, std::string& error_msg);
        bool validate_minItems (validation_context& ctx,
                                jvalue& schema, jvalue& schema_value,
                                jvalue& instance, std::string& error_msg);
        bool validate_uniqueItems (validation_context& ctx,
                                   jvalue& schema, jvalue& schema_value,
                                   jvalue& instance, std::string& error_msg);
        bool validate_maxContains (validation_context& ctx,
                                   jvalue& schema, jvalue& schema_value,
                                   jvalue& instance, std::string& error_msg);
        bool validate_minContains (validation_context& ctx,
                                   jvalue& schema, jvalue& schema_value,
                                   jvalue& instance, std::string& error_msg);

        // Validation keywords for objects
        bool validate_maxProperties (validation_context& ctx,
                                     jvalue& schema, jvalue& schema_value,
                                     jvalue& instance, std::string& error_msg);
        bool validate_minProperties (validation_context& ctx,
                                     jvalue& schema, jvalue& schema_value,
                                     jvalue& instance, std::string& error_msg);
        bool validate_required (validation_context& ctx,
                                jvalue& schema, jvalue& schema_value,
                                jvalue& instance, std::string& error_msg);
        bool validate_dependentRequired (validation_context& ctx,
                                         jvalue& schema, jvalue& schema_value,
                                         jvalue& instance, std::string& error_msg);

        using kw_validator_t = bool (jvocabulary_validation::*) (validation_context&,
                                                                      jvalue&,
                                                                      jvalue&,
                                                                      jvalue&,
                                                                      std::string&);
        using kw_loader_t = void (jvocabulary_validation::*) (const std::string&, jvalue&, jvalue&);

        using keywords_t = std::map<std::string, std::tuple<jvalue_type, kw_loader_t, kw_validator_t>>;
        static const keywords_t keywords;
    };


}
#endif
