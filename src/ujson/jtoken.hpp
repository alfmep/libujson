/*
 * Copyright (C) 2023,2025 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_JTOKEN_HPP
#define UJSON_JTOKEN_HPP

#include <string>
#include <string_view>


namespace ujson::parser {


    /**
     * A token returned by class <code>jscanner</code>
     * when scanning a JSON document.
     */
    class jtoken {
    public:
        /**
         * Type of token.
         */
        enum type_t {
            tk_invalid,            /**< Invalid token. */
            tk_lcbrack,            /**< { */
            tk_rcbrack,            /**< } */
            tk_lbrack,             /**< [ */
            tk_rbrack,             /**< ] */
            tk_separator,          /**< , */
            tk_colon,              /**< : */
            tk_null,               /**< null */
            tk_true,               /**< true */
            tk_false,              /**< false */
            tk_string,             /**< string */
            tk_number,             /**< number */
            tk_identifier,         /**< Object key. ([_A-Za-z][_0-9A-Za-z]*) */
            tk_comment,            /**< Comment. */
        };

        /**
         * Error code.
         */
        enum error_t {
            ok = 0,                  /**< No error. */
            err_string,              /**< Invalid string. */
            err_string_unterminated, /**< Unterminated string. */
            err_string_escape,       /**< Invalid escape code. */
            err_string_utf8,         /**< Invalid UTF8 character. */
            err_number,              /**< Invalid number. */
            err_number_lone_minus,   /**< A '-' without digit(s) after. */
            err_number_no_frac,      /**< A '.' without digit(s) after. */
            err_number_no_exp,       /**< No digit(s) after [e|E][+|-]. */
            err_invalid,             /**< Invalid token. */
            err_unexpected_char,     /**< Unexpected character. */
            err_eob,                 /**< Unexpected end of buffer. */
        };

        /**
         * Constructor.
         * This will create a token of type <code>tk_invalid</code>
         */
        jtoken () {
            reset ();
        }

        /**
         * Reset the token state, making it an invalid token.
         */
        void reset () {
            type = tk_invalid;
            row = col = 0;
            err_code = ok;
            data = "";
        }

        type_t type;           /**< The type of token. */
        size_t row;            /**< The row the token starts on. */
        size_t col;            /**< The column the token starts on. */
        error_t err_code;      /**< Error code. */
        std::string_view data; /**< The token data. */
    };

    /**
     * Return a string representation of a token type.
     * @param token The type of token.
     * @return A string representation of a token type.
     */
    std::string jtoken_type_to_string (const jtoken::type_t token);


}
#endif
