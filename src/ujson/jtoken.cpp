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
#include <ujson/jtoken.hpp>

namespace ujson::parser {


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string jtoken_type_to_string (const jtoken::type_t type)
    {
        switch (type) {
        case jtoken::tk_lcbrack:
            return "LCBRACK";
        case jtoken::tk_rcbrack:
            return "RCBRACK";
        case jtoken::tk_lbrack:
            return "LBRACK";
        case jtoken::tk_rbrack:
            return "RBRACK";
        case jtoken::tk_separator:
            return "SEPARATOR";
        case jtoken::tk_colon:
            return "COLON";
        case jtoken::tk_null:
            return "NULL";
        case jtoken::tk_true:
            return "TRUE";
        case jtoken::tk_false:
            return "FALSE";
        case jtoken::tk_string:
            return "STRING";
        case jtoken::tk_number:
            return "NUMBER";
        case jtoken::tk_identifier:
            return "IDENTIFIER";
        case jtoken::tk_comment:
            return "COMMENT";
        default:
            return "INVALID";
        }
    }


}
