/*
 * Copyright (C) 2017,2019-2021 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_JSON_HPP
#define UJSON_JSON_HPP

#include <string>
#include <list>
#include <ujson/jvalue.hpp>


namespace ujson {


    /**
     * Class used for parsing json files and strings.
     */
    class Json {
    public:
        /**
         * Default constructor.
         */
        Json ();

        /**
         * Destructor.
         */
        virtual ~Json ();

        /**
         * Disabled copy constructor.
         */
        Json (const Json& j) = delete;

        /**
         * Disabled assignment operator.
         */
        Json& operator= (const Json& j) = delete;

        /**
         * Parse a json file.
         * @param f The name of the json file to parse.
         * @return A shared pointer to a json value that is
         *         either of type j_object or j_array.
         *         If parsing failed a nullptr is returned.
         * @see ujson::jvalue
         */
        jvalue parse_file (const std::string& f, bool allow_duplicates_in_obj=false);

        /**
         * Parse a string in json syntax.
         * @param str The string to parse.
         * @return A shared pointer to a json value that is
         *         either of type j_object or j_array.
         *         If parsing failed a nullptr is returned.
         * @see ujson::jvalue
         */
        jvalue parse_string (const std::string& str, bool allow_duplicates_in_obj=false);

        /**
         * Parse a text buffer in json syntax.
         * @param buf The text string to parse.
         * @param length The length of the text in the buffer.
         * @return A shared pointer to a json value that is
         *         either of type j_object or j_array.
         *         If parsing failed a nullptr is returned.
         * @see ujson::jvalue
         */
        jvalue parse_buffer (const char* buf, size_t length, bool allow_duplicates_in_obj=false);

        /**
         * Check if parse debug trace is on.
         */
        bool trace_parsing ();

        /**
         * Set/unset parse debug trace.
         */
        void trace_parsing (bool trace);

        /**
         * Check if scan debug trace is on.
         */
        bool trace_scanning ();

        /**
         * Set/unset scan debug trace.
         */
        void trace_scanning (bool trace);

        /**
         * Get maximum number of reported errors.
         */
        int max_errors () const;

        /**
         * Set maximum number of reported errors.
         */
        void max_errors (int max_error_count);

        /**
         * Get a list of parse errors if parsing failed.
         */
        const std::list<std::string>& errors () const;


    private:
        void* parse_context;
    };


}

#endif
