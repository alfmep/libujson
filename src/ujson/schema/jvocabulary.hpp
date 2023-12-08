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
#ifndef UJSON_SCHEMA_JVOCABULARY_HPP
#define UJSON_SCHEMA_JVOCABULARY_HPP

#include <ujson/invalid_schema.hpp>
#include <ujson/schema/validation_context.hpp>
#include <string>
#include <map>


// Forward declarations
namespace ujson {
    class jschema;
}


namespace ujson::schema {

    /**
     * Base class for JSON schema vocabularies.
     */
    class jvocabulary {
    public:
        jvocabulary (jschema& root_schema_arg);
        jvocabulary (const std::string& id_arg);
        jvocabulary (jschema& root_schema_arg, const std::string& id_arg);
        virtual ~jvocabulary () = default;

        virtual void load (jvalue& schema, jvalue& load_ctx) = 0;
        virtual bool validate (validation_context& ctx, jvalue& schema, jvalue& instance) = 0;

        virtual const std::string& id () const;


        static std::string resolve_id (const std::string& base_uri,
                                       const std::string& uri,
                                       std::string& err_msg,
                                       bool allow_fragment=false);
        static int split_uri (const std::string& full_uri, std::string& uri, std::string& fragment);


    protected:
        std::string vocabulary_id;
        jschema& root_schema;

        void load_subschema (jvalue& sub_schema);

        bool validate_subschema (validation_context& ctx,
                                 jvalue& sub_schema,
                                 jvalue& instance,
                                 const bool create_subcontext=true,
                                 const bool ignore_annotations=false,
                                 const bool invalidate_parent_if_invalid=false);

        void push_load_ctx_path (jvalue& load_ctx, const std::string& entry);
        void pop_load_ctx_path (jvalue& load_ctx);

        //          alias         id
        std::map<std::string, std::string>& id_aliases ();
    };


}
#endif
