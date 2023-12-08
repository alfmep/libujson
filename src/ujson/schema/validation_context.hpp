/*
 * Copyright (C) 2023 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_SCHEMA_VALIDATION_CONTEXT
#define UJSON_SCHEMA_VALIDATION_CONTEXT

#include <ujson/jvalue.hpp>
#include <ujson/jpointer.hpp>
#include <ujson/multimap_list.hpp>
#include <string>
#include <memory>
#include <list>
#include <map>

/**
 * JSON schema implementation classes.
 */
namespace ujson::schema {

    /**
     * Schema validation context.
     */
    class validation_context {
    public:
        enum output_unit_placement_t {
            place_automatic,
            place_annotation,
            place_error
        };

        validation_context ();
        validation_context (validation_context& parent_arg);

        void push_schema_path (const std::string& entry);
        void pop_schema_path ();

        void push_instance_path (const std::string& entry);
        void pop_instance_path ();

        void set_error (const std::string& err_msg);
        void append_error (const std::string& err_msg);
        void append_sub_ou ();
        void annotate (const jvalue& value);
        jvalue* annotation (const std::string& keyword, const std::string& instance_path);

        void collect_annotations (validation_context& sub_ctx);

        //void debug_print ();
        //void debug_print_annotations ();

        void set_valid (const bool is_valid);
        void cleanup ();

        void add_output_unit (jvalue&& sub_output_unit, output_unit_placement_t where=place_automatic);

        jpointer& validation_path () {
            return *validation_path_ptr;
        }
        jpointer& instance_path () {
            return *instance_path_ptr;
        }


        validation_context* parent;

        std::string base_uri;
        jpointer abs_keyword_path;

        jvalue output_unit; // Created with: output_unit["valid"] = true


        //                                       keyword      instance_path   annotation_value
        using annotations_t = std::map<std::pair<std::string, std::string>,   jvalue>;

        annotations_t annotations;
        std::list<annotations_t> in_place_annotations;

    private:
        std::shared_ptr<jpointer> validation_path_ptr;
        std::shared_ptr<jpointer> instance_path_ptr;
    };


}
#endif
