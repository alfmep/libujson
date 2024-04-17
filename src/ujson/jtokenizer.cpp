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
#include <ujson/jtokenizer.hpp>
#include <cctype>

#if 1
#include <iostream>
using std::cerr;
using std::endl;
#endif


namespace ujson::parser {


    enum jtokenizer::str_state_t : unsigned {
        ss_any,
        ss_uany,
        ss_escape,
        ss_escape_unicode,
    };

    enum jtokenizer::num_state_t : unsigned {
        ns_first_digit,
        ns_integer,
        ns_find_frac_or_exp,
        ns_find_exp,
        ns_exp,
        ns_frac,
    };


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


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jtokenizer::jtokenizer ()
        : buf_pos (nullptr),
          buf_end (nullptr),
          row (0),
          col (0),
          strict (true)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jtokenizer::jtokenizer (const std::string_view& buffer, bool strict_mode)
    {
        reset (buffer, strict_mode);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jtokenizer::reset (const std::string_view& buffer, bool strict_mode)
    {
        strict = strict_mode;
        buf_pos = buffer.data ();
        buf_end = buf_pos + buffer.size ();
        token_pos = buf_pos;
        row = 0;
        col = 0;
        token.reset ();
        str_state = ss_any;
        ch_count = 0;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::pair<size_t, size_t> jtokenizer::pos () const
    {
        return std::make_pair (row, col);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jtokenizer::set_token_at_pos (jtoken::type_t type, size_t size, jtoken::error_t err)
    {
        token.type = type;
        token.data = std::string_view (token_pos, size);
        token.row = row;
        token.col = col;
        token.err_code = err;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jtokenizer::set_token (jtoken::type_t type, size_t size, jtoken::error_t err)
    {
        token.type = type;
        token.data = std::string_view (token_pos, size);
        token.err_code = err;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jtokenizer::scan_token (jtoken::type_t type, const char* const name, size_t name_size)
    {
        size_t name_index = 0;

        token.row = row;
        token.col = col;
        advance_pos ();

        while (true) {
            if (buf_pos >= buf_end) {
                set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
                return;
            }
            if (*buf_pos != name[++name_index]) {
                if (strict) {
                    set_token (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_invalid);
                    return;
                }
                // This is an identifier
                if ((std::isalnum(*buf_pos) || *buf_pos=='_')) {
                    do {
                        advance_pos ();
                    }while ((buf_pos < buf_end) && (std::isalnum(*buf_pos) || *buf_pos=='_'));
                    set_token (jtoken::tk_identifier, buf_pos-token_pos);
                    if (buf_pos < buf_end)
                        --buf_pos;
                }else{
                    set_token (jtoken::tk_identifier, buf_pos-token_pos);
                    if (buf_pos < buf_end)
                        --buf_pos;
                }
                return;
            }
            if (name_index >= (name_size-1)) {
                if (strict) {
                    set_token (type, name_size);
                }
                else if (((buf_pos+1)<buf_end)  &&  (std::isalnum(buf_pos[1]) || buf_pos[1]=='_')) {
                    // This is an identifier
                    do {
                        advance_pos ();
                    }while ((buf_pos < buf_end) && (std::isalnum(*buf_pos) || *buf_pos=='_'));
                    set_token (jtoken::tk_identifier, buf_pos-token_pos);
                    if (buf_pos < buf_end)
                        --buf_pos;
                }else{
                    set_token (type, name_size);
                }
                return;
            }
            advance_pos ();
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jtokenizer::scan_string ()
    {
        while (advance_pos_and_check()) {
            const char ch = *buf_pos;
            switch (str_state) {
            case ss_any:
                if (ch == '"') {
                    // String done
                    set_token (jtoken::tk_string, buf_pos-token_pos);
                    return;
                }
                else if (ch==(char)0x20 ||
                         ch==(char)0x21 ||
                         (ch>=(char)0x23 && ch<=(char)0x5b) ||
                         (ch>=(char)0x5d && ch<=(char)0x7f))
                {
                    ; // ASCII
                }
                else if (ch == '\\') {
                    str_state = ss_escape;
                }
                else if (ch>=(char)0xc2 && ch<=(char)0xdf) {
                    str_state = ss_uany;
                    ch_count = 1;
                }
                else if (ch>=(char)0xe0 && ch<=(char)0xef) {
                    str_state = ss_uany;
                    ch_count = 2;
                }
                else if (ch>=(char)0xf0 && ch<=(char)0xf4) {
                    str_state = ss_uany;
                    ch_count = 3;
                }
                else {
                    set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_string_unterminated);
                    return;
                }
                break;

            case ss_uany:
                if (ch>=(char)0x80 && ch<=(char)0xbf) {
                    if (--ch_count == 0)
                        str_state = ss_any;
                }else{
                    set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_string_utf8);
                    return;
                }
                break;

            case ss_escape:
                switch (ch) {
                case '\\':
                case '"':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                case '/':
                    str_state = ss_any;
                    break;
                case 'u':
                    str_state = ss_escape_unicode;
                    ch_count = 4;
                    break;
                default:
                    set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_string_escape);
                    return;
                }
                break;

            case ss_escape_unicode:
                if (isxdigit(ch)) {
                    if (--ch_count == 0)
                        str_state = ss_any;
                }else{
                    set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_string_escape);
                    return;
                }
                break;
            }
        }
        --token_pos;
        set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jtokenizer::scan_number ()
    {
        do {
            char ch = *buf_pos;

            switch (num_state) {
            case ns_first_digit:
                if (ch == '-') {
                    if (!advance_pos_and_check()) {
                        set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
                        return;
                    }
                    ch = *buf_pos;
                }
                if (ch == '0') {
                    // Possibly a fraction or exponent after
                    num_state = ns_find_frac_or_exp;
                }else if (std::isdigit(ch)) {
                    // Possibly more digits after
                    num_state = ns_integer;
                }else{
                    // Only a '-' with no digit after, not ok.
                    set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_number_lone_minus);
                    return;
                }
                break;

            case ns_find_frac_or_exp:
                if (ch == '.') {
                    // We have a fraction
                    num_state = ns_frac;
                    if (!advance_pos_and_check()) {
                        set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
                        return;
                    }
                    if (!std::isdigit(*buf_pos)) {
                        set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_number_no_frac);
                        return;
                    }
                    ch = *buf_pos;
                    break;
                }
                // Fall through
                // ...
            case ns_find_exp:
                if (ch == 'e' || ch == 'E') {
                    // We have an exponent
                    num_state = ns_exp;
                    if (!advance_pos_and_check()) {
                        set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
                        return;
                    }
                    ch = *buf_pos;
                    if (ch == '-' || ch == '+') {
                        if (!advance_pos_and_check()) {
                            set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
                            return;
                        }
                        ch = *buf_pos;
                    }
                    if ( ! std::isdigit(ch)) {
                        set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_number_no_exp);
                        return;
                    }
                }else{
                    // No exponent, number done
                    set_token (jtoken::tk_number, buf_pos-token_pos);
                    return;
                }
                break;

            case ns_exp:
                if ( ! std::isdigit(ch)) {
                    set_token (jtoken::tk_number, buf_pos-token_pos);
                    return;
                }

            case ns_integer:
                if ( ! std::isdigit(ch)) {
                    // Possibly a fraction or exponent after
                    num_state = ns_find_frac_or_exp;
                    continue;
                }
                break;

            case ns_frac:
                    if ( ! std::isdigit(ch)) {
                    // Possibly an exponent after
                    num_state = ns_find_exp;
                    continue;
                }
                break;
            }

            advance_pos ();
        }while (buf_pos < buf_end);

        set_token (jtoken::tk_number, buf_pos-token_pos);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    const jtoken* jtokenizer::next_token ()
    {
        // Already done scanning ?
        if (token.type == jtoken::tk_invalid  &&  token.err_code!=jtoken::ok)
            return nullptr;

        // Ignore whitespace
        while (buf_pos < buf_end) {
            if (*buf_pos=='\t' || *buf_pos=='\r' || *buf_pos==' ' || *buf_pos=='\n')
                advance_pos ();
            else
                break;
        }

        // End of buffer ?
        if (buf_pos >= buf_end) {
            set_token_at_pos (jtoken::tk_invalid, 0, jtoken::ok);
            return nullptr;
        }

        token_pos = buf_pos;
        switch (*buf_pos) {
        case '{':
            set_token_at_pos (jtoken::tk_lcbrack, 1);
            break;
        case '}':
            set_token_at_pos (jtoken::tk_rcbrack, 1);
            break;
        case '[':
            set_token_at_pos (jtoken::tk_lbrack, 1);
            break;
        case ']':
            set_token_at_pos (jtoken::tk_rbrack, 1);
            break;
        case ',':
            set_token_at_pos (jtoken::tk_separator, 1);
            break;
        case ':':
            set_token_at_pos (jtoken::tk_colon, 1);
            break;
        case 'n':
            scan_token (jtoken::tk_null, "null", 4);
            break;
        case 't':
            scan_token (jtoken::tk_true, "true", 4);
            break;
        case 'f':
            scan_token (jtoken::tk_false, "false", 5);
            break;
        case '"':
            token.row = row;
            token.col = col+1;
            str_state = ss_any;
            ch_count = 0;
            ++token_pos;
            scan_string ();
            break;
        case '/':
            if (!strict)
                scan_comment ();
            else
                set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_unexpected_char);
            break;

        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            token.row = row;
            token.col = col;
            num_state = ns_first_digit;
            scan_number ();
            return &token;

        default:
            if (!strict) {
                if (std::isalpha(*buf_pos) || *buf_pos=='_') {
                    token.row = row;
                    token.col = col;
                    do {
                        advance_pos ();
                    }while ((buf_pos < buf_end) && (std::isalnum(*buf_pos) || *buf_pos=='_'));
                    set_token (jtoken::tk_identifier, buf_pos-token_pos);
                    return &token;
                }
            }

            set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos+1, jtoken::err_unexpected_char);
            break;
        }

        advance_pos ();

        return &token;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jtokenizer::scan_comment ()
    {
        token.row = row;
        token.col = col;

        if (!advance_pos_and_check()) {
            set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
            return;
        }

        if (*buf_pos == '/') {
            //
            // One line comment
            //
            while (advance_pos_and_check()) {
                if (*buf_pos == '\n')
                    break;
            }
            //set_token (jtoken::tk_one_line_comment, buf_pos-token_pos);
            set_token (jtoken::tk_comment, buf_pos-token_pos);
        }
        else if (*buf_pos == '*') {
            //
            // Multi line comment
            //
            while (advance_pos_and_check()) {
                if (*buf_pos == '*') {
                    if (!advance_pos_and_check()) {
                        set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
                        return;
                    }
                    if (*buf_pos == '/') {
                        set_token (jtoken::tk_comment, buf_pos-token_pos+1);
                        return;
                    }
                }
            }
            set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_eob);
        }
        else{
            //
            // No comment - unexpected character
            //
            row = token.row;
            col = token.col;
            --buf_pos;
            set_token_at_pos (jtoken::tk_invalid, buf_pos-token_pos, jtoken::err_unexpected_char);
            return;
        }
    }


}
