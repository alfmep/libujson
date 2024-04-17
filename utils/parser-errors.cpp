/*
 * Copyright (C) 2023,2024 Dan Arrhenius <dan@ultramarin.se>
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
#include "parser-errors.hpp"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
const std::string parser_err_to_str (ujson::jparser::err error)
{
    switch (error) {
    case ujson::jparser::err::ok:
        return "ok";

    case ujson::jparser::err::invalid_string:
        return "invalid_string";

    case ujson::jparser::err::unterminated_string:
        return "unterminated_string";

    case ujson::jparser::err::invalid_escape_code:
        return "invalid_escape_code";

    case ujson::jparser::err::invalid_utf8:
        return "invalid_utf8";

    case ujson::jparser::err::invalid_number:
        return "invalid_number";

    case ujson::jparser::err::number_out_of_range:
        return "number_out_of_range";

    case ujson::jparser::err::invalid_token:
        return "invalid_token";

    case ujson::jparser::err::unexpected_character:
        return "unexpected_character";

    case ujson::jparser::err::eob:
        return "Unexpected end of file/buffer.";

    case ujson::jparser::err::io:
        return "I/O error.";

    case ujson::jparser::err::internal:
        return "Internal error.";

    case ujson::jparser::err::misplaced_right_curly_bracket:
        return "misplaced_right_curly_bracket";

    case ujson::jparser::err::misplaced_right_bracket:
        return "misplaced_right_bracket";

    case ujson::jparser::err::misplaced_separator:
        return "misplaced_separator";

    case ujson::jparser::err::misplaced_colon:
        return "misplaced_colon";

    case ujson::jparser::err::expected_separator_or_right_bracket:
        return "expected_separator_or_right_bracket";

    case ujson::jparser::err::expected_separator_or_right_curly_bracket:
        return "expected_separator_or_right_curly_bracket";

    case ujson::jparser::err::expected_obj_member_name:
        return "expected_obj_member_name";

    case ujson::jparser::err::expected_colon:
        return "expected_colon";

    case ujson::jparser::err::duplicate_obj_member:
        return "Duplicate object member name found.";

    case ujson::jparser::err::unterminated_array:
        return "unterminated_array";

    case ujson::jparser::err::unterminated_object:
        return "unterminated_object";

    case ujson::jparser::err::max_depth_exceeded:
        return "Maximum nesting depth exceeded.";

    case ujson::jparser::err::max_array_size_exceeded:
        return "Maximum number of array items exceeded.";

    case ujson::jparser::err::max_obj_size_exceeded:
        return "Maximum number of object members exceeded.";

    default:
        return "(unkown error)";
    }
}
