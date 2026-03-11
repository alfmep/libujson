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
#ifndef UJSON_JTOKENIZER_HPP
#define UJSON_JTOKENIZER_HPP

#include <string>
#include <string_view>
#include <ujson/jtoken.hpp>


/**
 * Classes and types used by the JSON parser.
 */
namespace ujson::parser {


    /**
     * Class used to scan a buffer and produce tokens for JSON parsing.
     * Only UTF8 encoded text buffers are supported.
     */
    class jtokenizer {
    public:
        /**
         * Default Constructor.
         */
        jtokenizer ();

        /**
         * Constructor.
         * @param buffer The buffer to scan for JSON tokens.
         * @param strict_mode Use strict mode when scanning.
         */
        jtokenizer (const std::string_view& buffer, bool strict_mode=true);

        /**
         * Reset the tokenizer.
         * @param buffer The buffer to scan for JSON tokens.
         */
        void reset (const std::string_view& buffer) {
            reset (buffer, strict);
        }

        /**
         * Reset the tokenizer.
         * @param buffer The buffer to scan for JSON tokens.
         * @param strict_mode Use strict mode when scanning.
         */
        void reset (const std::string_view& buffer, bool strict_mode);

        /**
         * Return the next JSON token in the buffer.
         * @return A pointer to a jtoken object describing
         *         the next token in the buffer.
         *         Or <code>nullptr</code> if no more
         *         tokens are available.
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
