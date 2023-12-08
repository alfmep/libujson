/*
 * Copyright (C) 2017,2019-2023 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_JPARSER_HPP
#define UJSON_JPARSER_HPP

#include <string>
#include <ujson/jvalue.hpp>


namespace ujson {


    /**
     * Class used for parsing JSON documents.
     */
    class jparser {
    public:
        /**
         * An error code generated when parsing a JSON document.
         */
        enum class err {
            ok,                   /**< No error, all is ok. */
            invalid_string,       /**< Invalid string. */
            unterminated_string,  /**< Unterminated string. */
            invalid_escape_code,  /**< Invalid escape code. */
            invalid_utf8,         /**< Invalid UTF8 character. */
            invalid_number,       /**< Invalid number. */
            number_out_of_range,  /**< Number out of range. */
            invalid_token,        /**< Invalid token. */
            unexpected_character, /**< Unexpected character. */
            eob,                  /**< Unexpected end of buffer/file. */
            io,                   /**< Error reading input file. */
            internal,             /**< Internal parser error. */

            misplaced_right_curly_bracket,
            misplaced_right_bracket,
            misplaced_separator,
            misplaced_colon,
            expected_separator_or_right_bracket,
            expected_separator_or_right_curly_bracket,
            expected_obj_member_name,
            expected_colon,
            unterminated_array,
            unterminated_object,
        };

        /**
         * Parser error;
         */
        struct error_t {
            jparser::err code; /**< The error code. @see jparser::err */
            unsigned row;      /**< Line number of the error, starting at index 0. */
            unsigned col;      /**< Column number of the error, starting at index 0. */
        };

        /**
         * Default constructor.
         */
        jparser ();

        /**
         * Destructor.
         */
        virtual ~jparser ();

        /**
         * Disabled copy constructor.
         */
        jparser (const jparser& j) = delete;

        /**
         * Disabled assignment operator.
         */
        jparser& operator= (const jparser& j) = delete;

        /**
         * Parse a JSON file and return a jvalue instance.
         * @param f The name of the JSON file to parse.
         * @param strict_mode if <code>true</code>, parsing is done strictly
         *                    according to the JSON specification (RFC 8259).
         *                    <br/>
         *                    If <code>false</code>, a more relaxed JSON
         *                    format is used, where, among other things,
         *                    C-style comments are allowed.
         * @param allow_duplicates_in_obj If <code>true</code>, duplicate
         *                                member names in objects are allowed.<br/>
         *                                If <code>false</code> and duplicate
         *                                member names exist, only the last
         *                                name/value pair will be present in
         *                                the resulting parsed object.<br/>
         *                                Default is <code>true</code> since
         *                                the JSON specification does not
         *                                prevent duplicate object member names.
         * @return A jvalue representing the JSON document. If parsing fails,
         *         the returned jvalue will be invalid (of type ujson::j_invalid).
         * @see ujson::jvalue
         * @see ujson::jvalue::type()
         * @see ujson::jvalue::valid()
         * @see ujson::jvalue::invalid()
         * @see error()
         */
        jvalue parse_file (const std::string& f,
                           bool strict_mode=true,
                           bool allow_duplicates_in_obj=true);

        /**
         * Parse a string in JSON syntax and return a jvalue instance.
         * @param str The string to parse.
         * @param strict_mode if <code>true</code>, parsing is done strictly
         *                    according to the JSON specification (RFC 8259).
         *                    <br/>
         *                    If <code>false</code>, a more relaxed JSON
         *                    format is used, where, among other things,
         *                    C-style comments are allowed.
         * @param allow_duplicates_in_obj If <code>true</code>, duplicate
         *                                member names in objects are allowed.<br/>
         *                                If <code>false</code> and duplicate
         *                                member names exist, only the last
         *                                name/value pair will be present in
         *                                the resulting parsed object.<br/>
         *                                Default is <code>true</code> since
         *                                the JSON specification does not
         *                                prevent duplicate object member names.
         * @return A jvalue representing the JSON document. If parsing fails,
         *         the returned jvalue will be invalid (of type ujson::j_invalid).
         * @see ujson::jvalue
         * @see ujson::jvalue::type()
         * @see ujson::jvalue::valid()
         * @see ujson::jvalue::invalid()
         * @see error()
         */
        jvalue parse_string (const std::string& str,
                             bool strict_mode=true,
                             bool allow_duplicates_in_obj=true);

        /**
         * Parse a text buffer in JSON syntax and return a jvalue instance.
         * @param buf The text string to parse.
         * @param length The length (in bytes) of the text in the buffer.
         * @param strict_mode if <code>true</code>, parsing is done strictly
         *                    according to the JSON specification (RFC 8259).
         *                    <br/>
         *                    If <code>false</code>, a more relaxed JSON
         *                    format is used, where, among other things,
         *                    C-style comments are allowed.
         * @param allow_duplicates_in_obj If <code>true</code>, duplicate
         *                                member names in objects are allowed.<br/>
         *                                If <code>false</code> and duplicate
         *                                member names exist, only the last
         *                                name/value pair will be present in
         *                                the resulting parsed object.<br/>
         *                                Default is <code>true</code> since
         *                                the JSON specification does not
         *                                prevent duplicate object member names.
         * @return A jvalue representing the JSON document. If parsing fails,
         *         the returned jvalue will be invalid (of type ujson::j_invalid).
         * @see ujson::jvalue
         * @see ujson::jvalue::type()
         * @see ujson::jvalue::valid()
         * @see ujson::jvalue::invalid()
         * @see error()
         */
        jvalue parse_buffer (const char* buf,
                             size_t length,
                             bool strict_mode=true,
                             bool allow_duplicates_in_obj=true);

        /**
         * Get an error code and position.
         * @return An error code and the position in the file/buffer where
         *         the error was found.
         */
        const error_t get_error () const;

        /**
         * Get an error message if parsing has failed.
         * @return An error string if the last parsing failed.
         *         On successfull parsing an empty string is returned.
         */
        const std::string& error () const;


    private:
        void* parse_context;
    };


}

#endif
