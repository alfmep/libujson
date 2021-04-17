/*
 * Copyright (C) 2017,2019,2021 Dan Arrhenius <dan@ultramarin.se>
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
#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include <sstream>
#include <cstdio>
#include <ujson/utils.hpp>
#include <ujson/internal.hpp>


namespace ujson {


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string jtype_to_str (const jvalue_type jtype)
    {
        switch (jtype) {
        case j_bool:
            return "boolean";
        case j_number:
            return "number";
        case j_string:
            return "string";
        case j_array:
            return "array";
        case j_object:
            return "object";
        case j_null:
            return "null";
        default:
            ;
        }
        return "invalid";
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string jtype_to_str (const jvalue& instance)
    {
        return jtype_to_str (instance.type());
    };


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue_type str_to_jtype (const std::string& jtype_name)
    {
        if (jtype_name == "boolean")
            return j_bool;
        else if (jtype_name == "number")
            return j_number;
        else if (jtype_name == "string")
            return j_string;
        else if (jtype_name == "array")
            return j_array;
        else if (jtype_name == "object")
            return j_object;
        else if (jtype_name == "null")
            return j_null;
        else
            return j_invalid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jvalue& get_property_value (jvalue& instance,
                                       const std::string& property)
    {
        std::stringstream ss (property);
        std::string name;
        jvalue* value = nullptr;

        if (!getline(ss, name, '[') || name.empty()) {
            if (property[0] != '[')
                return instance.get (unescape(property));
        }

        if (property[0] != '[')
            value = &(instance.get(unescape(name)));
        else
            value = &instance;

        std::string index;
        while (getline(ss, index, ']')) {
            value = &((*value)[stol(index)]);
            if (!value->valid() || !getline(ss, name, '['))
                break;
        }
        return *value;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& find_jvalue (jvalue& instance, const std::string& location)
    {
        try {
            std::stringstream ss (location);
            std::string property;
            jvalue* entry = &instance;
            while (getline(ss, property, '.')) {
                if (!property.empty()) {
                    entry = &(get_property_value(*entry, property));
                    if (entry->type() == j_invalid)
                        break;
                }
            }
            return *entry;
        }
        catch (...) {
            invalid_jvalue.reset ();
            return invalid_jvalue;
        }
    }


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
    std::string escape (const std::string& in, bool escape_slash)
    {
        static const char* hex = "0123456789abcdef";
        std::string result;
        for (unsigned char ch : in) {
            if (ch<0x20 || ch=='"' || (escape_slash?ch=='/':false) || ch=='\\') {
                switch (ch) {
                case 0x08: // backspace
                    result.append ("\\b");
                    break;
                case 0x09: // tab
                    result.append ("\\t");
                    break;
                case 0x0a: // newline
                    result.append ("\\n");
                    break;
                case 0x0c: // formfeed
                    result.append ("\\f");
                    break;
                case 0x0d: // return
                    result.append ("\\r");
                    break;
                case 0x22: // double quote
                    result.append ("\\\"");
                    break;
                case 0x2f: // slash
                    result.append ("\\/");
                    break;
                case 0x5c: // backslash
                    result.append ("\\\\");
                    break;
                default:
                    if (ch < 0x10)
                        result.append ("\\u000");
                    else
                        result.append ("\\u001");
                    result.push_back (hex[ch & 0x0f]);
                }
            }else{
                result.push_back (ch);
            }
        }
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
