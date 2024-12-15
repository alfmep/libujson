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
#ifndef UJSON_SCHEMA_JVOCABULARY_APPLICATOR_HPP
#define UJSON_SCHEMA_JVOCABULARY_APPLICATOR_HPP

#include <ujson/schema/jvocabulary.hpp>
#include <string>


namespace ujson::schema {


    /**
     * JSON schema applicator vocabulary.
     */
    class jvocabulary_applicator : public jvocabulary {
    public:
        jvocabulary_applicator (jschema& schema_arg);

        virtual void load (jvalue& schema, jvalue& load_ctx);
        virtual bool validate (validation_context& ctx,
                               jvalue& schema,
                               jvalue& instance,
                               const bool quit_on_first_error);


    private:
        void load_allOf (jvalue& schema_value, jvalue& load_ctx);
        void load_anyOf (jvalue& schema_value, jvalue& load_ctx);
        void load_oneOf (jvalue& schema_value, jvalue& load_ctx);
        void load_prefixItems (jvalue& schema_value, jvalue& load_ctx);

        // 10.2. Keywords for Applying Subschemas in Place
        //     Applicator keywords for any instance
        bool validate_allOf (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                             jvalue& instance, const bool quit_on_first_error);
        bool validate_anyOf (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                             jvalue& instance, const bool quit_on_first_error);
        bool validate_oneOf (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                             jvalue& instance, const bool quit_on_first_error);
        bool validate_not (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                           jvalue& instance, const bool quit_on_first_error);
        bool validate_if (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                          jvalue& instance, const bool quit_on_first_error);
        bool validate_then (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                            jvalue& instance, const bool quit_on_first_error);
        bool validate_else (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                            jvalue& instance, const bool quit_on_first_error);

        //     Applicator keywords for objects
        bool validate_dependentSchemas (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                                        jvalue& instance, const bool quit_on_first_error);

        //
        // 10.3. Keywords for Applying Subschemas to Child Instances
        //     Applicator keywords for arrays
        bool validate_prefixItems (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                                   jvalue& instance, const bool quit_on_first_error);
        bool validate_items (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                             jvalue& instance, const bool quit_on_first_error);
        bool validate_contains (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                                jvalue& instance, const bool quit_on_first_error);

        //     Applicator keywords for objects
        bool validate_properties (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                                  jvalue& instance, const bool quit_on_first_error);
        bool validate_patternProperties (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                                         jvalue& instance, const bool quit_on_first_error);
        bool validate_additionalProperties (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                                            jvalue& instance, const bool quit_on_first_error);
        bool validate_propertyNames (validation_context& ctx, jvalue& schema, jvalue& schema_value,
                                     jvalue& instance, const bool quit_on_first_error);

        using kw_loader_t = void (jvocabulary_applicator::*) (jvalue&, jvalue&);
        using kw_validator_t = bool (jvocabulary_applicator::*) (validation_context&,
                                                                 jvalue&,
                                                                 jvalue&,
                                                                 jvalue&,
                                                                 const bool);
        using keywords_t = std::map<std::string, std::tuple<jvalue_type, kw_loader_t, jvalue_type, kw_validator_t>>;
        static const keywords_t keywords;
    };


}
#endif
