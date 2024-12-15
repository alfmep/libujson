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
     * A JSON schema.
     * This class represents a JSON schema that
     * may be used to validate JSON instances.
     * Currently, only JSON Schema version 2020-12 is supported by this class.
     */
    class jschema {
    public:
        /**
         * The default base URI of the root schema.
         * If the root schema is missing attribute "$id", this
         * will be used instead as the base URI of the root schema.
         */
        static constexpr const char* const default_base_uri = "xri://root-schema";

        /**
         * Default constructor.
         * Creates a boolean schema with value <code>true</code>.
         * Any JSON instance vill be successfully validated using this default schema.
         */
        jschema ();

        /**
         * Constructor.
         * Creates a JSON schema as defined by the argument.
         * @param root_arg The root schema definition.
         * @throw ujson::invalid_schema If the JSON Schema
         *                              definition is invalid.
         */
        jschema (const jvalue& root_arg);

        /**
         * Constructor.
         * Creates a JSON schema with a root schema definition and additional
         * schema definitions that may be referenced by the root schema.
         * @param root_arg The root schema definition.
         * @param referenced_schemas A list of schema definitions that may
         *                           be referenced by the root schema.
         * @throw ujson::invalid_schema If a JSON Schema definition is invalid.
         */
        jschema (const jvalue& root_arg, const std::list<jvalue>& referenced_schemas);

        /**
         * Reset the schema and set a new root schema definition.
         * @param root_arg The root schema definition.
         */
        void reset (const jvalue& root_arg);

        /**
         * Add a schema definition that may be referenced by the root
         * schema, of by other added schema defintions.
         * @param referenced_schema The JSON schema definition to add.
         * @throw ujson::invalid_schema If the JSON Schema
         *                              definition is invalid.
         */
        void add_referenced_schema (const jvalue& referenced_schema);

        /**
         * Add a schema definition that may be referenced by the root
         * schema, of by other added schema defintions.
         * @param referenced_schema The JSON schema definition to add.
         * @param alias An alias that can be used to reference this schema
         *              if it is missing an "$id" attribute.
         * @throw ujson::invalid_schema If the JSON Schema
         *                              definition is invalid.
         */
        void add_referenced_schema (const jvalue& referenced_schema, const std::string& alias);

        /**
         * Set a callback to be called if a "$ref" or "$dynamicRef"
         * in a schema can't be dereferenced.
         * The callback may be used to add an additional schema definition
         * using method <code>add_referenced_schema()</code> to solve
         * the invalid reference problem.<br/>
         * If a "$ref" or "$dynamicRef" in the schema can't be dereferenced,
         * the validation will pause and call this callback. If the callback
         * returns <code>true</code>, indicating it added a schema definition,
         * the validation will continue and if the "$ref" (or "$dynamicRef")
         * still can't be dereferenced, an <code>ujson::invalid_schema</code>
         * exception will be thrown. If the callback returns <code>false</code>,
         * indicating it didn't add a schema definition, the validation will
         * continue with a failure.
         * @param cb The callback to be called when a "$ref" can't be dereferenced.
         */
        void set_invalid_ref_cb (schema::jvocabulary_core::invalid_ref_cb_t cb);

        /**
         * Validate a JSON instance using this schema.
         * The JSON instance will be validated using the root schema definition,
         * and any added schema that is referenced, directly or indirectly, by
         * the root schema definition.<br/>
         * Validation stops at the first failed annotation test in the
         * JSON instance. To get a fully populated output unit, call
         * <code>validate(instance, false);</code>
         * @param instance The JSON instance to validate.
         * @return A JSON Schema Output Unit. This is a JSON object
         *         describing the result of the validation.<br/>
         *         If the instance was successfully validated, the
         *         attribute <code>"valid"</code> in the Output Unit will be
         *         <code>true</code>. If it was an unsuccessful validation
         *         the attribute <code>"valid"</code> will be set to
         *         <code>false</code>.
         * @throw ujson::invalid_schema If the JSON Schema
         *                              definition is invalid (if, for instance,
         *                              a "$dynamicRef" can't be dereferenced).
         */
        jvalue validate (jvalue& instance);

        /**
         * Validate a JSON instance using this schema.
         * The JSON instance will be validated using the root schema definition,
         * and any added schema that is referenced, directly or indirectly, by
         * the root schema definition.
         * @param instance The JSON instance to validate.
         * @param quit_on_first_error If <code>false</code>,
         *         validation continues even if some value
         *         in the instance fails a validation test.
         *         This makes the output unit to be fully
         *         populated with all annotation results.<br>
         *         If <code>true</code>, quit validation on first error.
         *         This makes validation faster, but the
         *         resulting output unit may not contain all
         *         annotations that would otherwise be produced.
         * @return A JSON Schema Output Unit. This is a JSON object
         *         describing the result of the validation.<br/>
         *         If the instance was successfully validated, the
         *         attribute <code>"valid"</code> in the Output Unit will be
         *         <code>true</code>. If it was an unsuccessful validation
         *         the attribute <code>"valid"</code> will be set to
         *         <code>false</code>.
         * @throw ujson::invalid_schema If the JSON Schema
         *                              definition is invalid (if, for instance,
         *                              a "$dynamicRef" can't be dereferenced).
         */
        jvalue validate (jvalue& instance, bool quit_on_first_error);

        /**
         * Return a pointer to a JSON Schema Vocabulary used by the schema.
         * @param name The name of the JSON Schema Vocabulary.
         * @return A pointer to a <code>schema::jvocabulary</code>,
         *         or <code>nullptr</code> if no such vocabulaty exists.
         */
        std::shared_ptr<schema::jvocabulary> vocabulary (const std::string& name);


    private:
        friend class schema::jvocabulary;

        void initialize (const jvalue& root_arg);
        virtual void load (jvalue& schema);
        virtual void init_vocabularies ();

        bool validate (schema::validation_context& ctx,
                       jvalue& schema,
                       jvalue& instance,
                       bool quit_on_first_error);

        multimap_list<std::string, std::shared_ptr<schema::jvocabulary>> vocabularies;
        jvalue root; // Root schema definition
        std::list<jvalue> ref_schemas; // Referenced schemas
        jvalue load_ctx;

        //          alias         id
        std::map<std::string, std::string> id_alias;
    };


}
#endif
