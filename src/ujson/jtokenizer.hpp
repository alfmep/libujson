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
#ifndef UJSON_JTOKENIZER_HPP
#define UJSON_JTOKENIZER_HPP

#include <string>
#include <string_view>


/**
 * Classes and types used by the JSON parser.
 */
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
         */
        jtoken () {
            reset ();
        }

        /**
         * Reset the token state.
         */
        void reset () {
            type = tk_invalid;
            row = col = 0;
            err_code = ok;
            data = "";
        }

        type_t type;           /**< The type of token. */
        size_t row;            /**< The row the token starts on. */
        size_t col;            /**< The comulm the token start on. */
        error_t err_code;      /**< Error code. */
        std::string_view data; /**< The token data. */
    };

    /**
     * Return a string representation of a token type.
     */
    std::string jtoken_type_to_string (const jtoken::type_t t);



    /**
     * Class used to scan a buffer and produce tokens for JSON parsing.
     */
    class jtokenizer {
    public:
        /**
         * Constructor.
         */
        jtokenizer ();

        /**
         * Constructor.
         * @param buffer The buffer to scan for tokens.
         * @param strict_mode Use strict mode when scanning.
         */
        jtokenizer (const std::string_view& buffer, bool strict_mode=true);

        /**
         * Reset the tokenizer.
         * @param buffer The buffer to scan for tokens.
         */
        void reset (const std::string_view& buffer) {
            reset (buffer, strict);
        }

        /**
         * Reset the tokenizer.
         * @param buffer The buffer to scan for tokens.
         * @param strict_mode Use strict mode when scanning.
         */
        void reset (const std::string_view& buffer, bool strict_mode);

        /**
         * Return the next token.
         * @return The next token in the buffer.
         *         Or nullptr if no more tokens are available.
         */
        const jtoken* next_token ();

        /**
         * Get the current position (row, column) in the buffer.
         */
        std::pair<size_t, size_t> pos () const;


    private:
        enum str_state_t : unsigned;
        enum num_state_t : unsigned;

        const char* buf_pos;
        const char* buf_end;
        const char* token_pos;
        size_t row;
        size_t col;

        jtoken token;
        num_state_t num_state;
        str_state_t str_state;
        size_t ch_count;

        bool strict;

        void set_token_at_pos (jtoken::type_t type, size_t size, jtoken::error_t err=jtoken::ok);
        void set_token (jtoken::type_t type, size_t size, jtoken::error_t err=jtoken::ok);
        void scan_token (jtoken::type_t type, const char* const name, size_t name_size);
        void scan_string ();
        void scan_number ();
        void scan_comment ();

        inline void advance_pos () {
            if (*buf_pos == '\n') {
                ++row;
                col = 0;
            }else{
                ++col;
            }
            ++buf_pos;
        }
        inline bool advance_pos_and_check () {
            if (*buf_pos == '\n') {
                ++row;
                col = 0;
            }else{
                ++col;
            }
            ++buf_pos;
            return buf_pos < buf_end;
        }
    };


}
#endif
