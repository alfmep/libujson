/*
 * Copyright (C) 2022 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_INVALID_SCHEMA_HPP
#define UJSON_INVALID_SCHEMA_HPP

#include <stdexcept>
#include <string>


namespace ujson {


    /**
     * Invalid JSON schema error.
     */
    class invalid_schema : public std::invalid_argument {
    public:
        /**
         * Constructor.
         * @param base_uri_arg A JSON Schema base URI.
         * @param pointer_arg A JSON pointer to the schema error.
         * @param what_arg An error description.
         */
        invalid_schema (const std::string& base_uri_arg,
                        const std::string& pointer_arg,
                        const std::string& what_arg);

        /**
         * Constructor.
         * @param base_uri_arg A JSON Schema base URI.
         * @param pointer_arg A JSON pointer to the schema error.
         * @param what_arg An error description.
         */
        invalid_schema (const std::string& base_uri_arg,
                        const std::string& pointer_arg,
                        const char* what_arg);

        /**
         * Constructor.
         * @param what_arg An error description.
         */
        invalid_schema (const std::string& what_arg);

        /**
         * Constructor.
         * @param what_arg An error description.
         */
        invalid_schema (const char* what_arg);

        /**
         * Copy constructor.
         * @param other The invalid_schema object to copy.
         */
        invalid_schema (const invalid_schema& other) noexcept;

        std::string base_uri; /**< A JSON Schema base URI. */
        std::string pointer;  /**< A JSON pointer to the error in the schema. */
    };


}
#endif
