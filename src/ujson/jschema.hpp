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
#ifndef UJSON_JSCHEMA_HPP
#define UJSON_JSCHEMA_HPP

#include <ujson/invalid_schema.hpp>
#include <ujson/schema/validation_context.hpp>
#include <ujson/schema/jvocabulary.hpp>
#include <ujson/schema/jvocabulary_core.hpp>
#include <string>
#include <memory>
#include <map>



namespace ujson {


    /**
     * JSON schema.
     */
    class jschema {
    public:
        static const std::string default_base_uri;

        /**
         * Default constructor.
         * Creates a boolean schema with value <code>true</code>.
         */
        jschema ();

        /**
         * Constructor.
         * @param root_arg The root schema definition.
         */
        jschema (const jvalue& root_arg);

        /**
         * Constructor.
         * @param root_arg The root schema definition.
         * @param referenced_schemas A list of schema definitions that may
         *                           be referenced by the root schema.
         */
        jschema (const jvalue& root_arg, const std::list<jvalue>& referenced_schemas);

        /**
         * Reset the schema and set a new root schema definition.
         * @param root_arg The root schema definition.
         */
        void reset (const jvalue& root_arg);

        void add_referenced_schema (const jvalue& referenced_schema);
        void add_referenced_schema (const jvalue& referenced_schema, const std::string& alias);
        std::shared_ptr<schema::jvocabulary> vocabulary (const std::string& name);
        jvalue validate (jvalue& instance);

        void set_invalid_ref_cb (schema::jvocabulary_core::invalid_ref_cb_t cb);

    private:
        friend class schema::jvocabulary;

        void initialize (const jvalue& root_arg);
        virtual void load (jvalue& schema);
        virtual void init_vocabularies ();

        bool validate (schema::validation_context& ctx, jvalue& schema, jvalue& instance);

        multimap_list<std::string, std::shared_ptr<schema::jvocabulary>> vocabularies;
        jvalue root; // Root schema definition
        std::list<jvalue> ref_schemas; // Referenced schemas
        jvalue load_ctx;

        //          alias         id
        std::map<std::string, std::string> id_alias;
    };


}
#endif
