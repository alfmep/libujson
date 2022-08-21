/*
 * Copyright (C) 2017,2019-2022 Dan Arrhenius <dan@ultramarin.se>
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
#include <utility>
#include <sstream>
#include <algorithm>
#include <regex>
#include <iomanip>
#include <ujson/jvalue.hpp>
#include <ujson/utils.hpp>
#include <ujson/internal.hpp>
#include <cstring>
#include <math.h>

namespace ujson {


    jvalue invalid_jvalue;


    //--------------------------------------------------------------------------
    // Functor to compare a string with the key in a json_pair object
    // when searching for an entry in a json object.
    class json_pair_key_cmp_t {
    public:
        json_pair_key_cmp_t (const std::string& k) : key(k) {}
        bool operator() (json_pair& entry) {
            return entry.first == key;
        }
    private:
        const std::string& key;
    };


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (jvalue_type vtype)
        : jtype {j_invalid}
    {
        type (vtype);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const jvalue& jval)
        : jtype {j_invalid}
    {
        copy (jval);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (jvalue&& jval)
        : jtype {j_invalid}
    {
        move (std::forward<jvalue&&>(jval));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const json_object& o)
        : jtype {j_object}
    {
        v.jobj = new json_object (o);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (json_object&& o)
        : jtype {j_object}
    {
        v.jobj = new json_object (std::forward<json_object&&>(o));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const json_array& a)
        : jtype {j_array}
    {
        v.jarray = new json_array (a);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (json_array&& a)
        : jtype {j_array}
    {
        v.jarray = new json_array (std::forward<json_array&&>(a));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const std::string& s)
        : jtype {j_string}
    {
        v.jstr = new std::string (s);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (std::string&& s)
        : jtype {j_string}
    {
        v.jstr = new std::string (std::forward<std::string&&>(s));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const char* s)
        : jtype {j_string}
    {
        v.jstr = new std::string ((s==nullptr?"":s));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const double n)
        : jtype {j_number}
    {
        v.jnum = n;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const int n)
        : jtype {j_number}
    {
        v.jnum = static_cast<double> (n);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const bool true_false)
        : jtype {j_bool}
    {
        v.jbool = true_false;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const std::nullptr_t nil)
        : jtype {j_null}
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::~jvalue ()
    {
        reset ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator= (const jvalue& value)
    {
        if (this != &value)
            copy (value);
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator= (jvalue&& jval)
    {
        if (this != &jval)
            move (std::forward<jvalue&&>(jval));
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator= (const json_object& o)
    {
        obj (o);
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator= (json_object&& o)
    {
        obj (std::forward<json_object&&>(o));
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator= (const json_array& a)
    {
        array (a);
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator= (json_array&& a)
    {
        array (std::forward<json_array&&>(a));
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::operator< (const jvalue& rval) const
    {
        if (this == &rval)
            return true;
        if (jtype != rval.type())
            return true;

        switch (jtype) {
        case j_invalid:
            return false;
        case j_object:
            return *v.jobj < *rval.v.jobj;
        case j_array:
            return std::lexicographical_compare (v.jarray->begin(), v.jarray->end(),
                                                 rval.v.jarray->begin(), rval.v.jarray->end());
        case j_string:
            return std::lexicographical_compare (v.jstr->begin(), v.jstr->end(),
                                                 rval.v.jstr->begin(), rval.v.jstr->end());
        case j_number:
            return v.jnum < rval.v.jnum;
        case j_bool:
            return false;
        case j_null:
            return false;
        }
        return false;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::operator== (const jvalue& rval) const
    {
        if (this == &rval)
            return true;

        if (jtype != rval.type())
            return false;

        switch (jtype) {
        case j_invalid:
            return false;
        case j_object:
            return *v.jobj == *rval.v.jobj;
        case j_array:
            return *v.jarray == *rval.v.jarray;
        case j_string:
            return *v.jstr == *rval.v.jstr;
        case j_number:
            return v.jnum == rval.v.jnum;
        case j_bool:
            return v.jbool == rval.v.jbool;
        case j_null:
            return true;
        }
        return false;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    json_object& jvalue::obj ()
    {
        if (jtype == j_object) {
            return *v.jobj;
        }else{
            static json_object invalid_obj;
            invalid_obj.clear ();
            return invalid_obj;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::obj (const json_object& o)
    {
        type (j_object);
        *v.jobj = o;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::obj (json_object&& o)
    {
        type (j_object);
        *v.jobj = std::forward<json_object&&> (o);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    json_array& jvalue::array ()
    {
        if (jtype == j_array) {
            return *v.jarray;
        }else{
            static json_array invalid_array;
            invalid_array.clear ();
            return invalid_array;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::array (const json_array& a)
    {
        type (j_array);
        *v.jarray = a;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::array (json_array&& a)
    {
        type (j_array);
        *v.jarray = std::forward<json_array&&> (a);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    const std::string& jvalue::str () const
    {
        if (jtype == j_string) {
            return *v.jstr;
        }else{
            static std::string invalid_str;
            invalid_str = "";
            return invalid_str;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::str (const std::string& s)
    {
        type (j_string);
        *v.jstr = s;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::str (std::string&& s)
    {
        type (j_string);
        *v.jstr = std::forward<std::string&&> (s);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    double jvalue::num () const
    {
        return (jtype==j_number ? v.jnum : 0.0);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::num (double n)
    {
        type (j_number);
        v.jnum = n;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::boolean () const
    {
        return (jtype==j_bool ? v.jbool : false);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::boolean (bool b)
    {
        type (j_bool);
        v.jbool = b;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::set_null ()
    {
        type (j_null);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::reset ()
    {
        switch (jtype) {
        case j_invalid:
            break;
        case j_object:
            if (v.jobj)
                delete v.jobj;
            v.jobj = nullptr;
            break;
        case j_array:
            if (v.jarray)
                delete v.jarray;
            v.jarray = nullptr;
            break;
        case j_string:
            if (v.jstr)
                delete v.jstr;
            v.jstr = nullptr;
            break;
        case j_number:
            v.jnum = 0.0;
            break;
        case j_bool:
            v.jbool = false;
            break;
        case j_null:
            break;
        }
        jtype = j_invalid;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::type (const jvalue_type t)
    {
        if (jtype == t)
            return; // This object is already of the requested type

        // Free resources
        reset ();

        // Set the json value type for this object
        jtype = t;

        // Allocate and initialize resources
        switch (jtype) {
        case j_object:
            v.jobj = new json_object;
            break;
        case j_array:
            v.jarray = new json_array;
            break;
        case j_string:
            v.jstr = new std::string;
            break;
        default:
            break;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::is_container () const
    {
        return jtype==j_object || jtype==j_array;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::have (const std::string& name) const
    {
        bool found = false;
        if (type() == j_object) {
            auto range = v.jobj->equal_range (name);
            for (auto i=range.first; !found && i!=range.second; ++i) {
                if (i->second.valid())
                    found = true;
            }
        }
        return found;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::get (const std::string& name)
    {
        if (type() == j_object) {
            auto entry = find_last_in_jobj (name);
            if (entry != v.jobj->send())
                return entry->second;
        }
        // Name not found or not a json object, return an invalid json value
        invalid_jvalue.reset ();
        return invalid_jvalue;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator[] (const std::string& name)
    {
        if (type() != j_object) {
            // This is not a json object, return an invalid json value
            invalid_jvalue.reset ();
            return invalid_jvalue;
        }
        auto entry = find_last_in_jobj (name);
        if (entry != v.jobj->send()) {
            // Found a valid json value associated with 'name'
            return entry->second;
        }else{
            // 'name' not found, create an invalid json value
            // associated with 'name'.
            return v.jobj->emplace_back (name, jvalue()).second;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator[] (const size_t index)
    {
        if (type() == j_array) {
            if (index < v.jarray->size())
                return v.jarray->operator[] (index);
        }
        // Out of bounds, or this is not a json array,
        // return an invalid json value
        invalid_jvalue.reset ();
        return invalid_jvalue;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    size_t jvalue::size () const
    {
        if (jtype == j_object)
            return v.jobj->size ();
        else if (jtype == j_array)
            return v.jarray->size ();
        else
            return 0;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::add (const std::string& name,
                         const jvalue& value,
                         const bool overwrite)
    {
        if (type()!=j_object || !value.valid()) {
            invalid_jvalue.reset ();
            return invalid_jvalue;
        }

        auto entry = find_last_in_jobj (name);
        if (entry == v.jobj->send()) {
            return v.jobj->emplace_back(name, value).second;
        }else{
            if (overwrite)
                entry->second = value;
            return entry->second;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::add (const std::string& name,
                         jvalue&& value,
                         const bool overwrite)
    {
        if (type()!=j_object || !value.valid()) {
            invalid_jvalue.reset ();
            return invalid_jvalue;
        }

        auto entry = find_last_in_jobj (name);
        if (entry == v.jobj->send()) {
            return v.jobj->emplace_back(name, std::forward<jvalue&&>(value)).second;
        }else{
            if (overwrite)
                entry->second = std::forward<jvalue&&> (value);
            return entry->second;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::add (const jvalue& value)
    {
        if (type()!=j_array || !value.valid()) {
            invalid_jvalue.reset ();
            return invalid_jvalue;
        }
        return v.jarray->emplace_back (value);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::add (jvalue&& value)
    {
        if (type()!=j_array || !value.valid()) {
            invalid_jvalue.reset ();
            return invalid_jvalue;
        }
        return v.jarray->emplace_back (std::forward<jvalue&&>(value));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::remove (const std::string& name)
    {
        bool retval = false;
        if (type() == j_object) {
            if (v.jobj->find(name) != v.jobj->end()) {
                v.jobj->erase (name);
                retval = true;
            }
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::remove (const size_t n)
    {
        bool retval = false;
        if (type() == j_array) {
            auto i = v.jarray->begin() + n;
            if (i != v.jarray->end()) {
                v.jarray->erase (i);
                retval = true;
            }
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::copy (const jvalue& rval)
    {
        type (rval.type());

        switch (type()) {
        case j_invalid:
            break;
        case j_object:
            v.jobj->clear ();
            *v.jobj = *rval.v.jobj;
            break;
        case j_array:
            v.jarray->clear ();
            *v.jarray = *rval.v.jarray;
            break;
        case j_string:
            *v.jstr = *rval.v.jstr;
            break;
        case j_number:
            v.jnum = rval.v.jnum;
            break;
        case j_bool:
            v.jbool = rval.v.jbool;
            break;
        case j_null:
            break;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::move (jvalue&& rval)
    {
        reset ();

        jtype = rval.type ();

        switch (jtype) {
        case j_invalid:
            break;
        case j_object:
            v.jobj = rval.v.jobj;
            rval.v.jobj = nullptr;
            break;
        case j_array:
            v.jarray = rval.v.jarray;
            rval.v.jarray = nullptr;
            break;
        case j_string:
            v.jstr = rval.v.jstr;
            rval.v.jstr = nullptr;
            break;
        case j_number:
            v.jnum = rval.v.jnum;
            rval.v.jnum = 0.0;
            break;
        case j_bool:
            v.jbool = rval.v.jbool;
            rval.v.jbool = false;
            break;
        case j_null:
            break;
        }
        rval.jtype = j_invalid;
    }


    //--------------------------------------------------------------------------
    // Assume jtype is j_object
    //--------------------------------------------------------------------------
    json_object::iterator jvalue::find_last_in_jobj (const std::string& key)
    {
        auto range = v.jobj->equal_range (key);
        auto first = range.first;
        auto last  = range.second;

        if (last == first)
            return v.jobj->send ();

        --last;
        while (!last->second.valid()) {
            if (last == first) {
                v.jobj->erase (last);
                return v.jobj->send ();
            }
            v.jobj->erase (last--);
        }
        return last;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string jvalue::describe (bool pretty,
                                  bool relaxed_mode,
                                  bool array_items_on_same_line,
                                  bool escape_slash,
                                  bool sorted_properties,
                                  const std::string& indent) const
    {
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<double>::digits10 + 1);
        describe (ss, pretty, relaxed_mode, array_items_on_same_line,
                  escape_slash, sorted_properties, "", indent);
        return ss.str ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::describe (std::stringstream& ss,
                           bool pretty,
                           bool relaxed_mode,
                           bool array_items_on_same_line,
                           bool escape_slash,
                           bool sorted_properties,
                           const std::string& first_indent,
                           const std::string& indent_step) const
    {
        switch (type()) {
        case j_object:
            describe_object (ss,
                             pretty,
                             relaxed_mode,
                             array_items_on_same_line,
                             escape_slash,
                             sorted_properties,
                             first_indent,
                             indent_step);
            break;

        case j_array:
            describe_array (ss,
                            pretty,
                            relaxed_mode,
                            array_items_on_same_line,
                            escape_slash,
                            sorted_properties,
                            first_indent,
                            indent_step);
            break;

        case j_string:
            ss << '"' << escape(*v.jstr, escape_slash) << '"';
            break;

        case j_number:
            if (std::isinf(num()) || std::isnan(num()))
                ss << "null";
            else
                ss << num ();
            break;

        case j_bool:
            ss << (boolean() ? "true" : "false");
            break;

        case j_null:
            ss << "null";

        case j_invalid:
        default:
            break;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::describe_object (std::stringstream& ss,
                                  bool pretty,
                                  bool relaxed_mode,
                                  bool array_items_on_same_line,
                                  bool escape_slash,
                                  bool sorted_properties,
                                  const std::string& first_indent,
                                  const std::string& indent_step) const
    {
        static const std::regex re_identifier ("[_a-zA-Z][_a-zA-Z0-9]*",
                                               std::regex::ECMAScript);
        static const std::regex re_reserved ("([tT][rR][uU][eE])|"
                                             "([fF][aA][lL][sS][eE])|"
                                             "([nN][uU][lL][lL])",
                                             std::regex::ECMAScript);
        std::cmatch re_match;
        bool first {true};
        std::string indent;
        if (pretty)
            indent = first_indent + indent_step;

        ss << '{';
        auto& members = *v.jobj;
        if (!members.empty()) {
            auto i = sorted_properties ? members.sbegin() : members.begin();
            auto member_end = sorted_properties ? members.send() : members.end();
            for (; i!=member_end; ++i) {
                if (! i->second.valid())
                    continue; // Skip invalid values
                auto& name = i->first;
                auto& value = i->second;
                bool quoted_name = true;
                if (relaxed_mode) {
                    // In relaxed mode, if the member name is an 'identifier',
                    // print it without enclosing double quotes. Unless it
                    // is a reserved name.
                    if (std::regex_match (name.c_str(), re_match, re_identifier) &&
                        ! std::regex_match (name.c_str(), re_match, re_reserved))
                    {
                        quoted_name = false;
                    }
                }
                if (first)
                    first = false;
                else
                    ss << ',';
                if (pretty)
                    ss << std::endl << indent;

                if (quoted_name)
                    ss << '"' << escape(name, escape_slash) << '"';
                else
                    ss << name;
                ss << (pretty ? ": " : ":");
                value.describe (ss,
                                pretty,
                                relaxed_mode,
                                array_items_on_same_line,
                                escape_slash,
                                sorted_properties,
                                indent,
                                indent_step);
            }
        }
        if (pretty && !first)
            ss << std::endl << first_indent;
        ss << '}';
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::describe_array (std::stringstream& ss,
                                 bool pretty,
                                 bool relaxed_mode,
                                 bool array_items_on_same_line,
                                 bool escape_slash,
                                 bool sorted_properties,
                                 const std::string& first_indent,
                                 const std::string& indent_step) const
    {
        std::string indent;
        if (pretty)
            indent = first_indent + indent_step;

        auto& elements = *v.jarray;
        if (elements.empty()) {
            ss << "[]";
            return;
        }

        ss << '[';

        bool first {true};
        for (auto& obj : elements) {
            if (!obj.valid())
                continue; // Skip invalid values
            if (!first)
                ss << ',';
            if (pretty) {
                if (array_items_on_same_line) {
                    if (!first)
                        ss << ' ';
                }else{
                    ss << std::endl << indent;
                }
            }
            first = false;
            obj.describe (ss,
                          pretty,
                          relaxed_mode,
                          array_items_on_same_line,
                          escape_slash,
                          sorted_properties,
                          indent,
                          indent_step);
        }
        if (pretty & !array_items_on_same_line)
            ss << std::endl << first_indent;
        ss << ']';
    }


}
