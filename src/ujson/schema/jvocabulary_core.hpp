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
#ifndef UJSON_SCHEMA_JVOCABULARY_CORE_HPP
#define UJSON_SCHEMA_JVOCABULARY_CORE_HPP

#include <ujson/schema/jvocabulary.hpp>
#include <functional>
#include <ostream>
#include <string>
#include <tuple>
#include <map>


namespace ujson::schema {


    /**
     * JSON schema core vocabulary.
     * This class implements the JSON Schema
     * vocabulary defined in https://json-schema.org/draft/2020-12/meta/core.
     */
    class jvocabulary_core : public jvocabulary {
    public:
        using invalid_ref_cb_t = std::function<bool (jschema& schema,
                                                     const std::string& base_uri,
                                                     const std::string& ref_value)>;

        /**
         * Constructor.
         * @param schema_arg A root schema object.
         */
        jvocabulary_core (jschema& schema_arg);

        virtual void load (jvalue& schema, jvalue& load_ctx);
        virtual bool validate (validation_context& ctx,
                               jvalue& schema,
                               jvalue& instance,
                               const bool quit_on_first_error);

        void set_invalid_ref_cb (invalid_ref_cb_t cb);
        void print_maps (std::ostream& out);


    private:
        void load_schema (jvalue& schema, jvalue& schema_value, jvalue& load_ctx);
        void load_id (jvalue& schema, jvalue& schema_value, jvalue& load_ctx);
        void load_defs (jvalue& schema, jvalue& schema_value, jvalue& load_ctx);
        void load_anchor (jvalue& schema, jvalue& schema_value, jvalue& load_ctx);
        void load_dynamicAnchor (jvalue& schema, jvalue& schema_value, jvalue& load_ctx);
        //void load_comment (jvalue& schema, jvalue& schema_value, jvalue& load_ctx);

        bool validate_id (validation_context& ctx, jvalue& schema_value, jvalue& instance);
        bool validate_ref (validation_context& ctx,
                           jvalue& schema,
                           jvalue& schema_value,
                           jvalue& instance,
                           const bool quit_on_first_error);
        bool validate_dynamicRef (validation_context& ctx,
                                  jvalue& schema,
                                  jvalue& schema_value,
                                  jvalue& instance,
                                  const bool quit_on_first_error);
        //bool validate_comment (validation_context& ctx, jvalue& schema, jvalue& schema_value, jvalue& instance);

        jvalue* resolve_ref (validation_context& ctx, jvalue& schema, std::string& ref);
        jvalue* resolve_dynref (validation_context& ctx, jvalue& schema, std::string& dynref);

        using ids_t = std::map<std::string, std::reference_wrapper<jvalue>>;
        using ids_iter_t = ids_t::iterator;
        ids_t ids;
        std::map<std::string, std::tuple<std::string, std::string, std::reference_wrapper<jvalue>>> anchors;
        std::map<std::string, std::tuple<std::string, std::string, std::reference_wrapper<jvalue>>> dyn_anchors;

        invalid_ref_cb_t invalid_ref_cb;


        using keyword_loader_t = void (jvocabulary_core::*) (jvalue&, jvalue&, jvalue&);
        using keyword_loaders_t = std::map<std::string, keyword_loader_t>;
        static const keyword_loaders_t keyword_loaders;
    };


}
#endif
