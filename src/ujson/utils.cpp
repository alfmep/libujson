/*
 * Copyright (C) 2017,2019 Dan Arrhenius <dan@ultramarin.se>
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
#include <string>
#include <regex>
#include <codecvt>
#include <locale>
#include <ujson/utils.hpp>


namespace ujson {



    //--------------------------------------------------------------------------
    // Assumes ch is in range [0-9a-zA-Z]
    //--------------------------------------------------------------------------
    static unsigned char ch_to_hex (unsigned char ch)
    {
        if (ch <= '9')
            return ch - '0';
        else
            return (ch|0x20) - 'W'; // Set lowercase and subtract ('a'-10)
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template<class Iter>
    static std::string unescape_16bit_utf16 (Iter& pos, Iter end, bool& ok, bool to_utf16)
    {
        int num_chars;
        wchar_t utf16_in = 0;

        ok = true;
        for (num_chars=0; num_chars<4 && pos!=end; ++num_chars, ++pos) {
            utf16_in <<= 4;
            unsigned char hex_val = ch_to_hex (*pos);
            if (hex_val & 0xf0)
                ok = false;
            utf16_in |= hex_val;
        }
        if (!ok || num_chars<4)
            return "";

        try {
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conversion;
            return conversion.to_bytes (utf16_in);
        }catch (...) {
            return "";
        };
    }



    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string escape (const std::string& in)
    {
        static std::regex r_backslash ("\\\\");
        static std::regex r_dbl_quote ("\\\"");
        static std::regex r_slash ("/");
        static std::regex r_newline ("\\n");
        static std::regex r_backspace ("\\x08");
        static std::regex r_formfeed ("\\f");
        static std::regex r_return ("\\r");
        static std::regex r_tab ("\\t");

        std::string result;
        result = std::regex_replace (in, r_backslash, "\\\\");
        result = std::regex_replace (result, r_dbl_quote, "\\\"");
        result = std::regex_replace (result, r_slash, "\\/");
        result = std::regex_replace (result, r_newline, "\\n");
        result = std::regex_replace (result, r_formfeed, "\\f");
        result = std::regex_replace (result, r_return, "\\r");
        result = std::regex_replace (result, r_tab, "\\t");
        result = std::regex_replace (result, r_backspace, "\\b");
        return result;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string unescape (const std::string& in, bool& ok)
    {
        std::string result;

        ok = true;
        auto pos = std::find (in.begin(), in.end(), '\\');
        if (pos == in.end())
            return in;

        result.insert (result.end(), in.begin(), pos);

        // Check if string ends with a '\\'
        if (pos++ != in.end()  &&  pos == in.end())
            ok = false;

        while (pos != in.end()) {
            switch (*pos) {
            case '"':
                result.push_back ('"');
                ++pos;
                break;
            case '\\':
                result.push_back ('\\');
                ++pos;
                break;
            case '/':
                result.push_back ('/');
                ++pos;
                break;
            case 'b':
                result.push_back ('\b');
                ++pos;
                break;
            case 'f':
                result.push_back ('\f');
                ++pos;
                break;
            case 'n':
                result.push_back ('\n');
                ++pos;
                break;
            case 'r':
                result.push_back ('\r');
                ++pos;
                break;
            case 't':
                result.push_back ('\t');
                ++pos;
                break;
            case 'u':
                ++pos;
                {
                    bool utf16_ok;
                    result.append (unescape_16bit_utf16(pos, in.end(),
                                                        utf16_ok, false));
                    if (!utf16_ok)
                        ok = false;
                }
                break;
            default:
                ok = false;
                break;
            }
            if (pos == in.end())
                break;
            auto next = std::find (pos, in.end(), '\\');
            result.insert (result.end(), pos, next);
            if (next == in.end())
                break;
            pos = next;
            // Check if string ends with a '\\'
            if (pos++ != in.end()  &&  pos == in.end())
                ok = false;
        }
        return result;
    }


}
