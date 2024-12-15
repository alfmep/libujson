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
#ifndef UJSON_SCHEMA_JVOCABULARY_UNEVALUATED_HPP
#define UJSON_SCHEMA_JVOCABULARY_UNEVALUATED_HPP

#include <ujson/schema/jvocabulary.hpp>
#include <ujson/schema/validation_context.hpp>
#include <set>


namespace ujson::schema {


    /**
     * JSON schema unevaluated vocabulary.
     */
    class jvocabulary_unevaluated : public jvocabulary {
    public:
        jvocabulary_unevaluated (jschema& schema_arg);

        virtual void load (jvalue& schema, jvalue& load_ctx);
        virtual bool validate (validation_context& ctx,
                               jvalue& schema,
                               jvalue& instance,
                               const bool quit_on_first_error);


    private:
        bool validate_unevaluatedItems (validation_context& ctx,
                                        std::set<size_t>& indexes,
                                        jvalue& schema,
                                        jvalue& schema_value,
                                        jvalue& instance,
                                        const bool quit_on_first_error);
        bool validate_unevaluatedProperties (validation_context& ctx,
                                             jvalue& schema,
                                             jvalue& schema_value,
                                             jvalue& instance,
                                             const bool quit_on_first_error);




        jvalue get_unevaluatedItems_annotation (validation_context& ctx,
                                                const std::string& instance_path);
        jvalue get_items_annotation (validation_context& ctx,
                                     const std::string& instance_path);
        jvalue get_prefixItems_annotation (validation_context& ctx,
                                           const std::string& instance_path);
        jvalue get_contains_annotation (validation_context& ctx,
                                        const std::string& instance_path);

        std::set<size_t> collect_unevaluatedItems_annotations (validation_context& ctx,
                                                               jvalue& instance);



        void get_properties_annotations (validation_context& ctx,
                                         const std::string& instance_path,
                                         const std::string& keyword,
                                         std::set<std::string>& names);
        std::set<std::string> collect_unevaluatedProperties_annotations (validation_context& ctx);
    };



}
#endif
