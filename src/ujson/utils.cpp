/*
 * Copyright (C) 2017,2019,2021-2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <ujson/internal.hpp>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <cstdio>
#include <ujson/utils.hpp>
#include <ujson/jparser.hpp>
#include <ujson/jpointer.hpp>


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
        static const std::map<const std::string, const jvalue_type> names {{
                {"array",   j_array},
                {"boolean", j_bool},
                {"null",    j_null},
                {"number",  j_number},
                {"object",  j_object},
                {"string",  j_string},
            }};
        auto entry = names.find (jtype_name);
        if (entry != names.end())
            return entry->second;
        else
            return j_invalid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string escape_pointer_token (const std::string& element)
    {
        std::string escaped_element;
        for (auto ch : element) {
            if (ch == '~')
                escaped_element.append ("~0");
            else if (ch == '/')
                escaped_element.append ("~1");
            else
                escaped_element = escaped_element + ch;
        }
        return escaped_element;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string unescape_pointer_token (const std::string& element)
    {
        std::string retval = element;
        std::string::size_type pos = 0;

        while ((pos=retval.find("~1", pos)) != std::string::npos)
            retval = retval.replace (pos++, 2, "/");
        pos = 0;
        while ((pos=retval.find("~0", pos)) != std::string::npos)
            retval = retval.replace (pos++, 2, "~");
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
    jvalue& find_jvalue (jvalue& instance, const jpointer& pointer)
    {
        if (pointer.empty()) {
            // An empty pointer points to the root of the instance
            return instance;
        }

        try {
            jvalue* value = &instance;
            for (auto& token : pointer) {
                jvalue* tmp = &invalid_jvalue;
                if (value->type() == j_object) {
                    tmp = &(value->get(token));
                }else if (value->type() == j_array && is_array_index(token)) {
                    tmp = &((*value)[stol(token)]);
                }
                value = tmp;
                if (!value->valid()) {
                    invalid_jvalue.type (j_invalid);
                    break;
                }
            }
            return *value;
        }catch (...) {
            invalid_jvalue.type (j_invalid);
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
    static jpatch_result get_pointer_info (pointer_info_t& pi,
                                           jvalue& instance,
                                           const std::string& pointer,
                                           bool add=false)
    {
        pi.reset ();
        pi.instance = &instance;
        jpatch_result retval = patch_ok;

        if (pointer.empty()) {
            pi.item = pi.instance;
            return patch_ok;
        }
        if (pointer[0] != '/') {
            pi.reset ();
            return patch_invalid;
        }

        auto last_sep_pos = pointer.find_last_of ("/");
        pi.path = pointer.substr (0, last_sep_pos);
        pi.name = pointer.substr (last_sep_pos+1);

        pi.container = &find_jvalue (instance, pi.path);
        if (pi.container->invalid()) {
            pi.reset ();
            return patch_noent;
        }

        pi.item = &find_jvalue (instance, pointer);
        if (!pi.item->valid()) {
            pi.item = nullptr;
            retval = patch_noent;
            if (pi.container->type() == j_array) {
                if (pi.name=="-") {
                    retval = patch_ok;
                    pi.index = pi.container->size ();
                    if (add) {
                        pi.item = &pi.container->append(jvalue(j_null));
                        pi.added = true;
                        retval = patch_ok;
                    }
                }else if (add) {
                    if (is_array_index(pi.name)) {
                        ssize_t i = stol (pi.name);
                        if (i == (ssize_t)pi.container->size()) {
                            pi.index = i;
                            pi.name.clear ();
                            pi.item = &pi.container->append(jvalue(j_null));
                            pi.added = true;
                            retval = patch_ok;
                        }
                    }
                }
            }else if (add) {
                // j_object
                pi.item = &pi.container->add(pi.name, jvalue(j_null));
                pi.added = true;
                retval = patch_ok;
            }
        }
        else if (pi.container->type() == j_array) {
            pi.index = stol (pi.name);
            pi.name.clear ();
        }

        if (retval != patch_ok)
            pi.reset ();

        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_add_impl (jvalue& instance,
                                                   const std::string& pointer,
                                                   jvalue& value)
    {
        if (pointer.empty()) {
            instance = value;
            return patch_ok;
        }

        pointer_info_t pi;
        auto retval = get_pointer_info (pi, instance, pointer, true);
        if (retval == patch_ok) {
            if (pi.container->type()==j_array && pi.added==false) {
                auto& a = pi.container->array ();
                a.insert (a.begin() + pi.index, value);
            }else{
                *pi.item = value;
            }
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_add (jvalue& instance,
                                              const std::string& pointer,
                                              jvalue& op)
    {
        auto& value = op.get_unique ("value");
        if (value.valid())
            return patch_operation_add_impl (instance, pointer, value);
        else
            return patch_invalid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_remove (jvalue& instance,
                                                 const std::string& pointer,
                                                 jvalue& op)
    {
        if (pointer.empty()) {
            instance.type (j_null);
            return patch_ok;
        }

        pointer_info_t pi;
        jpatch_result retval = get_pointer_info (pi, instance, pointer);
        if (retval != patch_ok)
            return retval;

        retval = patch_noent;
        if (pi.item) {
            if (pi.container->type() == j_object)
                pi.container->remove (pi.name);
            else
                pi.container->remove (pi.index);
            retval = patch_ok;
        }
        else if (pi.container &&
                 pi.container->type() == j_array &&
                 pi.name == "-")
        {
            if (pi.container->size() > 0) {
                pi.container->array().pop_back ();
                retval = patch_ok;
            }
        }

        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_replace (jvalue& instance,
                                                  const std::string& pointer,
                                                  jvalue& op)
    {
        auto& value = op.get_unique ("value");
        if (!value.valid())
            return patch_invalid;

        pointer_info_t pi;
        auto retval = get_pointer_info (pi, instance, pointer);
        if (retval != patch_ok)
            return retval;

        if (pi.item) {
            *pi.item = value;
        }
        else if (pi.container->type() == j_array) {
            if (pi.container->size() > 0) {
                (*pi.container)[pi.container->size()-1] = value;
            }else{
                retval = patch_noent;
            }
        }
        else {
            *pi.container = value;
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_replace (jvalue& instance,
                                                  const std::string& pointer,
                                                  jvalue&& value)
    {
        pointer_info_t pi;
        auto retval = get_pointer_info (pi, instance, pointer);
        if (retval != patch_ok)
            return retval;

        if (pi.item) {
            *pi.item = std::forward<jvalue&&> (value);
        }
        else if (pi.container->type() == j_array) {
            if (pi.container->size() > 0) {
                (*pi.container)[pi.container->size()-1] = std::forward<jvalue&&> (value);
            }else{
                retval = patch_noent;
            }
        }
        else {
            *pi.container = std::forward<jvalue&&> (value);
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_move (jvalue& instance,
                                               const std::string& dst_pointer,
                                               jvalue& op)
    {
        auto& from = op.get_unique ("from");
        if (from.type() != j_string) {
            return patch_invalid;
        }
        auto& src_pointer = from.str ();

        if (dst_pointer.find(src_pointer)==0) {
            if (src_pointer.size() == dst_pointer.size()) {
                return patch_ok;
            }
            // Error - Move a contaner into one of its child entries
            return patch_invalid;
        }


        pointer_info_t src_pi;
        pointer_info_t dst_pi;
        jpatch_result retval = patch_ok;
        retval = get_pointer_info (src_pi, instance, src_pointer);
        if (retval != patch_ok)
            return retval;
        retval = get_pointer_info (dst_pi, instance, dst_pointer, true);
        if (retval != patch_ok)
            return retval;

        if (dst_pi.added)
            retval = get_pointer_info (src_pi, instance, src_pointer);
        if (retval != patch_ok)
            return retval;


        jvalue tmp (std::move(*src_pi.item));
        src_pi.item->type (j_null);

        retval = patch_operation_remove (instance, src_pointer, op);
        if (retval == patch_ok)
            retval = patch_operation_replace (instance, dst_pointer, std::move(tmp));
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_copy (jvalue& instance,
                                               const std::string& dst_pointer,
                                               jvalue& op)
    {
        auto& from = op.get_unique ("from");
        if (from.type() != j_string)
            return patch_invalid;

        const auto& src_pointer = from.str ();

        pointer_info_t src_pi;
        auto retval = get_pointer_info (src_pi, instance, src_pointer);
        if (retval == patch_ok) {
            jvalue copy (src_pi.item ? *src_pi.item : *src_pi.container);
            retval = patch_operation_add_impl (instance, dst_pointer, copy);
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation_test (jvalue& instance,
                                               const std::string& path,
                                               jvalue& op)
    {
        auto& test_value = op.get_unique ("value");
        if (!test_value.valid())
            return patch_invalid;

        jpatch_result retval = patch_ok;
        auto& value = find_jvalue (instance, path);
        if (!value.valid()) {
            retval = patch_noent;
        }else{
            retval = value == test_value ? patch_ok : patch_fail;
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jpatch_result patch_operation (jvalue& instance, jvalue& op)
    {
        using patch_op_cb = jpatch_result (*) (jvalue&, const std::string&, jvalue&);
        static const std::map<const std::string, const patch_op_cb> patch_ops {{
                {"add",     patch_operation_add},
                {"copy",    patch_operation_copy},
                {"move",    patch_operation_move},
                {"remove",  patch_operation_remove},
                {"replace", patch_operation_replace},
                {"test",    patch_operation_test},
            }};

        jpatch_result retval = patch_ok;
        try {
            // Get the patch operation and path
            auto& op_type = op.get_unique("op").str ();
            auto& path = op.get_unique("path").str ();

            auto entry = patch_ops.find (op_type);
            if (entry != patch_ops.end()) {
                retval = entry->second (instance, path, op);
            }else{
                retval = patch_invalid;
            }
        }
        catch (json_type_error&) {
            retval = patch_invalid;
        }
        catch (...) {
            retval = patch_noent;
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::pair<bool, std::vector<jpatch_result>> patch (jvalue& instance,
                                                       jvalue& result_instance,
                                                       jvalue& json_patch)
    {
        if (result_instance.invalid())
            throw std::invalid_argument ("Invalid JSON instance");

        result_instance = instance;
        return patch (result_instance, json_patch);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::pair<bool, std::vector<jpatch_result>> patch (jvalue& instance,
                                                       jvalue& json_patch)
    {
        if (instance.invalid() || json_patch.invalid())
            throw std::invalid_argument ("Invalid JSON instance");

        std::pair<bool, std::vector<jpatch_result>> retval;
        retval.first = true;

        //int successful_operations = 0;
        if (json_patch.type() == j_array) {
            for (auto& operation : json_patch.array()) {
                auto result = patch_operation (instance, operation);
                if (result != patch_ok)
                    retval.first = false;
                retval.second.emplace_back (result);
            }
        }else{
            auto result = patch_operation (instance, json_patch);
            if (result != patch_ok)
                retval.first = false;
            retval.second.emplace_back (result);
        }

        return retval;
    }


}
