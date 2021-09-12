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
#include <stdexcept>
#include <regex>
#include <cstdio>
#include <cerrno>
#include <ujson/utils.hpp>
#include <ujson/internal.hpp>
#include <ujson/Json.hpp>
#include <ujson/Schema.hpp>

#include <iostream>


namespace ujson {


    /**
     * Get (possible emtpy) token on the right hand side of a delimeter.
     * And, of course, all other tokens as well.
     *
     * "/token_1/token_2/"
     *                   ^ token 3 - empty string
     */
    class rhs_tokenizer {
    public:
        rhs_tokenizer (const std::string& str, const char delim);
        bool operator () (std::string& token); // Next token in 'tok', return false if no more tokens available.
    private:
        const char* ptr;
        const char delim;
    };
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    rhs_tokenizer::rhs_tokenizer (const std::string& str, const char delim)
        : ptr (str.c_str()),
          delim (delim)
    {
        if (str.empty())
            ptr = nullptr;
        else if (ptr[0] == delim)
            ++ptr;
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool rhs_tokenizer::operator () (std::string& token)
    {
        token.clear ();
        if (!ptr)
            return false;
        while (*ptr!=delim && *ptr!='\0')
            token.push_back (*ptr++);
        if (*ptr == '\0')
            ptr = nullptr;
        else
            ++ptr;
        return true;
    }


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
    static std::string unescape_pointer_element (const std::string& element)
    {
        std::string retval = element;
        std::string::size_type pos = 0;

        while ((pos=retval.find("~1", pos)) != std::string::npos)
            retval = retval.replace (pos, 2, "/");
        pos = 0;
        while ((pos=retval.find("~0", pos)) != std::string::npos)
            retval = retval.replace (pos, 2, "~");
        return retval;
    }


    //--------------------------------------------------------------------------
    // Check if valid array index, defined as [0]|[1-9][0-9]*
    //--------------------------------------------------------------------------
    static bool is_array_index (const std::string& str)
    {
        static std::regex re ("[0]|[1-9][0-9]*", std::regex::ECMAScript);
        std::cmatch cm;
        return std::regex_match (str.c_str(), cm, re);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& find_jvalue (jvalue& instance, const std::string& pointer)
    {
        if (pointer.empty()) {
            // An empty pointer points to the root of the instance
            return instance;
        }
        else if (pointer[0] != '/') {
            // A non empty pointer must start with '/'
            invalid_jvalue.reset ();
            return invalid_jvalue;
        }

        jvalue* value = &instance;
        rhs_tokenizer get_next_token (pointer, '/');
        std::string token;
        while (get_next_token(token)) {
            std::string name = unescape_pointer_element (token);
            jvalue* tmp = &invalid_jvalue;
            if (value->type() == j_object) {
                tmp = &(value->get(name));
            }else if (value->type() == j_array && is_array_index(name)) {
                tmp = &((*value)[stol(name)]);
            }
            value = tmp;
            if (!value->valid()) {
                invalid_jvalue.reset ();
                break;
            }
        }
        return *value;
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


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    struct pointer_info_t {
        jvalue* instance;  // Instance containing the item pointed to
        jvalue* container; // Conainter of the item ponted to. If nullptr: root instance, or invalid pointer
        jvalue* item;      // Item pointed io. If nullptr, no such item
        std::string path;  // Path to the container of the item.
        std::string name;  // Name of the item pointed to.
        ssize_t index;     // Index of the item pointed to, if container is an array, else -1
        bool added;        // An item was added.
        pointer_info_t () {
            reset ();
        }
        void reset () {
            instance = container = item = nullptr;
            path = name = "";
            index = -1;
            added = false;
        }
    };
    static pointer_info_t get_pointer_info (jvalue& instance, const std::string& pointer, bool add=false)
    {
        pointer_info_t pi;
        pi.instance = &instance;
        errno = 0;

        if (pointer.empty()) {
            pi.item = pi.instance;
            return pi;
        }
        if (pointer[0] != '/') {
            pi.reset ();
            errno = EINVAL;
            return pi;
        }

        auto last_sep_pos = pointer.find_last_of ("/");
        pi.path = pointer.substr (0, last_sep_pos);
        pi.name = pointer.substr (last_sep_pos+1);

        pi.container = &find_jvalue (instance, pi.path);
        if (pi.container->valid()) {
            pi.item = &find_jvalue (instance, pointer);
            if (!pi.item->valid()) {
                pi.item = nullptr;
                errno = ENOENT;
                if (pi.container->type() == j_array) {
                    if (pi.name=="-") {
                        errno = 0;
                        pi.index = pi.container->size ();
                        if (add) {
                            pi.item = &pi.container->add(jvalue(j_null));
                            pi.added = true;
                            errno = 0;
                        }
                    }else if (add) {
                        if (is_array_index(pi.name)) {
                            ssize_t i = stol (pi.name);
                            if (i == (ssize_t)pi.container->size()) {
                                pi.index = i;
                                pi.name.clear ();
                                pi.item = &pi.container->add(jvalue(j_null));
                                pi.added = true;
                                errno = 0;
                            }
                        }
                    }
                }else if (add) {
                    pi.item = &pi.container->add(pi.name, jvalue(j_null));
                    pi.added = true;
                    errno = 0;
                }
            }
            else if (pi.container->type() == j_array) {
                pi.index = stol (pi.name);
                pi.name.clear ();
            }
        }else{
            pi.reset ();
            errno = ENOENT;
        }
        if (errno)
            pi.reset ();

        return pi;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool patch_operation_add (jvalue& instance,
                                     const std::string& pointer,
                                     const jvalue& value)
    {
        if (pointer.empty()) {
            instance = value;
            return true;
        }

        auto pi = get_pointer_info (instance, pointer, true);
        if (errno) {
            return false;
        }

        if (pi.container->type()==j_array && pi.added==false) {
            auto& a = pi.container->array ();
            a.insert (a.begin() + pi.index, value);
        }else{
            *pi.item = value;
        }

        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool patch_operation_remove (jvalue& instance,
                                        const std::string& pointer)
    {
        if (pointer.empty()) {
            instance.type (j_null);
            return true;
        }

        auto pi = get_pointer_info (instance, pointer);
        if (errno)
            return false;

        bool retval = false;
        if (pi.item) {
            if (pi.container->type() == j_object)
                pi.container->remove (pi.name);
            else
                pi.container->remove (pi.index);
            retval = true;
        }
        else if (pi.container &&
                 pi.container->type() == j_array &&
                 pi.name == "-")
        {
            if (pi.container->size() > 0) {
                pi.container->array().pop_back ();
                retval = true;
            }
        }
        if (!retval)
            errno = ENOENT;

        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool patch_operation_replace (jvalue& instance,
                                         const std::string& pointer,
                                         const jvalue& value)
    {
        auto pi = get_pointer_info (instance, pointer);
        if (errno)
            return false;

        if (pi.item) {
            *pi.item = value;
        }
        else if (pi.container->type() == j_array) {
            if (pi.container->size() > 0) {
                (*pi.container)[pi.container->size()-1] = value;
            }else{
                errno = ENOENT;
                return false;
            }
        }
        else {
            *pi.container = value;
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool patch_operation_replace (jvalue& instance,
                                         const std::string& pointer,
                                         jvalue&& value)
    {
        auto pi = get_pointer_info (instance, pointer);
        if (errno)
            return false;

        if (pi.item) {
            *pi.item = std::forward<jvalue&&> (value);
        }
        else if (pi.container->type() == j_array) {
            if (pi.container->size() > 0) {
                (*pi.container)[pi.container->size()-1] = std::forward<jvalue&&> (value);
            }else{
                errno = ENOENT;
                return false;
            }
        }
        else {
            *pi.container = std::forward<jvalue&&> (value);
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool patch_operation_move (jvalue& instance,
                                      const std::string& dst_pointer,
                                      const std::string& src_pointer)
    {
        if (dst_pointer.find(src_pointer)==0) {
            if (src_pointer.size() == dst_pointer.size()) {
                return true;
            }
            // Error - Move a contaner into one of its child entries
            errno = EINVAL;
            return false;
        }


        auto src_pi = get_pointer_info (instance, src_pointer);
        if (errno)
            return false;
        auto dst_pi = get_pointer_info (instance, dst_pointer, true);
        if (errno)
            return false;

        if (dst_pi.added)
            src_pi = get_pointer_info (instance, src_pointer);


        jvalue tmp (std::move(*src_pi.item));
        src_pi.item->type (j_null);

        if (!patch_operation_remove(instance, src_pointer))
            return false;
        return patch_operation_replace (instance, dst_pointer, std::move(tmp));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool patch_operation_copy (jvalue& instance,
                                      const std::string& dst_pointer,
                                      const std::string& src_pointer)
    {
        auto src_pi = get_pointer_info (instance, src_pointer);
        if (errno)
            return false;

        jvalue copy (src_pi.item ? *src_pi.item : *src_pi.container);
        return patch_operation_add (instance, dst_pointer, copy);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static bool patch_operation (jvalue& instance, jvalue& op)
    {
        auto& op_type = op.get("op").str ();
        auto& path_obj = op.get ("path");
        if (path_obj.type() != j_string) {
            errno = EINVAL;
            return false;
        }
        auto& path = op.get("path").str ();

        bool retval = true;
        try {
            if (op_type == "add") {
                auto& value = op.get ("value");
                if (!value.valid()) {
                    errno = EINVAL;
                    retval = false;
                }else{
                    retval = patch_operation_add (instance, path, value);
                }
            }
            else if (op_type == "remove") {
                retval = patch_operation_remove (instance, path);
            }
            else if (op_type == "replace") {
                auto& value = op.get ("value");
                if (!value.valid()) {
                    errno = EINVAL;
                    retval = false;
                }else{
                    retval = patch_operation_replace (instance, path, value);
                }
            }
            else if (op_type == "move") {
                auto& from = op.get ("from");
                if (from.type() != j_string) {
                    errno = EINVAL;
                    retval = false;
                }else{
                    retval = patch_operation_move (instance, path, from.str());
                }
            }
            else if (op_type == "copy") {
                auto& from = op.get ("from");
                if (from.type() != j_string) {
                    errno = EINVAL;
                    retval = false;
                }else{
                    retval = patch_operation_copy (instance, path, from.str());
                }
            }
            else if (op_type == "test") {
                auto& test_value = op.get ("value");
                if (!test_value.valid()) {
                    errno = EINVAL;
                    retval = false;
                }else{
                    auto& value = find_jvalue (instance, path);
                    if (!value.valid()) {
                        errno = ENOENT;
                        retval = false;
                    }else{
                        retval = (value == test_value);
                    }
                }
            }
            else {
                errno = EINVAL;
                retval = false;
            }
        }
        catch (...) {
            errno = ENOENT;
            return false;
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    int patch (jvalue& instance, jvalue& patch)
    {
        //static Schema op_schema (Json().parse_string("{\"oneOf\":[{\"$ref\":\"#/$defs/operation\"},{\"type\":\"array\",\"items\":{\"$ref\":\"#/$defs/operation\"}}],\"$defs\":{\"operation\":{\"type\":\"object\",\"required\":[\"op\",\"path\"],\"properties\":{\"op\":{\"type\":\"string\",\"pattern\":\"add|remove|replace|move|copy|test\"},\"path\":{\"type\":\"string\"},\"value\":true,\"from\":{\"type\":\"string\"}},\"if\":{\"properties\":{\"op\":{\"pattern\":\"add|replace|test\"}}},\"then\":{\"required\":[\"value\"]},\"else\":{\"if\":{\"properties\":{\"op\":{\"pattern\":\"move|copy\"}}},\"then\":{\"required\":[\"from\"]}}}}}"));

        if (!instance.valid() /*|| op_schema.validate(patch) != Schema::valid*/) {
            errno = EINVAL;
            return 0;
        }

        errno = 0;
        int successful_operations = 0;
        if (patch.type() == j_array) {
            for (auto& operation : patch.array()) {
                if (patch_operation(instance, operation))
                    ++successful_operations;
                else
                    break;
            }
        }else{
            if (patch_operation(instance, patch))
                ++successful_operations;
        }
        return successful_operations;
    }


}
