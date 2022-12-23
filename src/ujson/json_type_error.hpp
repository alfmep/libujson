/*
 * Copyright (C) 2017,2019-2022 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_JSON_TYPE_ERROR_HPP
#define UJSON_JSON_TYPE_ERROR_HPP

#include <string>
#include <stdexcept>


namespace ujson {

    /**
     * Exception thrown when a method in class ujson::jvalue is called
     * that operates on a different JSON type than the that instance
     * currently represents.
     * ujson::json_type_error is a subclass of std::logic_error.
     * \par Exampe:
     * \code
     * ujson::jvalue val = 42;
     * try {
     *     // val is a JSON number, but here we try to use it as a JSON string
     *     std::cout << val.str() << std::endl;
     * }
     * catch (ujson::json_type_error& jte) {
     *     std::cerr << "Error: " << jte.what() << std::endl;
     * }
     * \endcode
     */
    class json_type_error : public std::logic_error {
    public:
        /**
         * Constructor.
         * @param what_arg Explanatory string.
         */
        json_type_error (const std::string& what_arg) : std::logic_error(what_arg) {};

        /**
         * Constructor.
         * @param what_arg Explanatory string.
         */
        json_type_error (const char* what_arg) : std::logic_error(what_arg) {};

        /**
         * Copy constructor.
         * @param other Another exception object to copy.
         */
        json_type_error (const json_type_error& other) noexcept : std::logic_error(other) {};

        /**
         * Assignment operator.
         * @param other Another exception object to assign with.
         */
        json_type_error& operator= (const json_type_error& other) noexcept {
            std::logic_error::operator= (other);
            return *this;
        }
    };


}
#endif
