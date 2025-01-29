/*
 * Copyright (C) 2017,2019-2025 Dan Arrhenius <dan@ultramarin.se>
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
#include <utility>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <iomanip>
#include <ujson/jvalue.hpp>
#include <ujson/utils.hpp>
#include <cstring>
#include <cmath>


#if (UJSON_HAS_CONSOLE_COLOR)
#  define COLOR_NORMAL "\033[0m"
#  define COLOR_BOLD "\033[1m"
#  define COLOR_ITALIC "\033[3m"

#  define COLOR_RED "\033[31m"
#  define COLOR_GREEN "\033[32m"
#  define COLOR_BLUE "\033[34m"
#  define COLOR_MAGENTA "\033[35m"
#  define COLOR_CYAN "\033[36m"
#  define COLOR_GRAY "\033[90m"

static constexpr const char* const color_normal = COLOR_NORMAL;
static constexpr const char* const color_string = COLOR_RED;
//static constexpr const char* const color_string = COLOR_GREEN;

//static constexpr const char* const color_boolean = COLOR_GREEN COLOR_BOLD;
static constexpr const char* const color_boolean = COLOR_CYAN COLOR_BOLD;
static constexpr const char* const color_boolean_true = COLOR_GREEN COLOR_BOLD;
static constexpr const char* const boolean_false_color = COLOR_RED COLOR_BOLD;

static constexpr const char* const null_color = COLOR_MAGENTA COLOR_BOLD;
static constexpr const char* const attribute_color = COLOR_BLUE;

static constexpr const char* const object_color = ""; //COLOR_BOLD;
static constexpr const char* const array_color = ""; //COLOR_BOLD;
#else
static constexpr const char* const color_normal = "";
static constexpr const char* const color_string = "";
//static constexpr const char* const color_string = "";
//static constexpr const char* const color_boolean = "";
static constexpr const char* const color_boolean = "";
static constexpr const char* const color_boolean_true = "";
static constexpr const char* const boolean_false_color = "";
static constexpr const char* const null_color = "";
static constexpr const char* const attribute_color = "";
static constexpr const char* const object_color = "";
static constexpr const char* const array_color = "";
#endif

#if (UJSON_HAS_CONSOLE_COLOR)
#define HAS_COLOR true
#else
#define HAS_COLOR false
#endif


namespace ujson {


    jvalue invalid_jvalue (j_invalid);


#if UJSON_HAVE_GMPXX
    static void num_t_to_str (const mpf_class& n, std::stringstream& ss)
    {
        mp_exp_t e;
        std::string str = n.get_str (e);
        long slen = str.length ();
        const char* s = str.c_str ();

        if (slen>0 && s[0]=='-') {
            ss << '-';
            ++s;
            --slen;
        }

        if (slen == 0) {
            ss << "0";
        }
        else if (e == 0) {
            ss << "0." << s;
        }
        else if (e >= (ssize_t)slen) {
            if (e > 16) {
                if (slen==1) {
                    ss << s << "e+" << e-1;
                }else{
                    ss << s[0] << '.' << (s+1) << "e+" << e-1;
                }
            }else{
                ss << s;
                for (long i=0; i<(e-slen); ++i)
                    ss << '0';
            }
        }
        else /* if (e < (ssize_t)slen) */ {
            if (e < -3) {
                if (slen==1) {
                    ss << s << 'e' << e-1;
                }else{
                    ss << s[0] << '.' << (s+1) << 'e' << e-1;
                }
            }else{
                if (e > 0) {
                    for (long i=0; i<e; ++i)
                        ss << s[i];
                    ss << '.';
                    ss << (s+e);
                }else{
                    ss << "0.";
                    for (long i=0; i>e; --i)
                        ss << '0';
                    ss << s;
                }
            }
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string to_string (const mpf_class& number)
    {
        std::stringstream ss;
        num_t_to_str (number, ss);
        return ss.str ();
    }
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static json_object::iterator find_last_in_jobj (const std::string& key,
                                                    json_object& jobj)
    {
        auto range = jobj.equal_range (key);
        auto first = range.first;
        auto last  = range.second;

        if (last == first)
            return jobj.send ();

        --last;
        while (!last->second.valid()) {
            if (last == first) {
                jobj.erase (last);
                return jobj.send ();
            }
            jobj.erase (last--);
        }
        return last;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue ()
        : jtype {j_null}
    {
    }


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


#if UJSON_HAVE_GMPXX
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const mpf_class& n)
        : jtype {j_number}
    {
        v.jnum = new num_t (n);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (mpf_class&& n)
        : jtype {j_number}
    {
        v.jnum = new num_t (std::forward<mpf_class&&>(n));
    }
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const double n)
        : jtype {j_number}
    {
#if UJSON_HAVE_GMPXX
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<double>::digits10 + 1) << n;
        v.jnum = new num_t (ss.str());
#else
        v.jnum = n;
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const int n)
        : jtype {j_number}
    {
#if UJSON_HAVE_GMPXX
        v.jnum = new num_t (n);
#else
        v.jnum = static_cast<num_t> (n);
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue::jvalue (const long n)
        : jtype {j_number}
    {
#if UJSON_HAVE_GMPXX
        v.jnum = new num_t (n);
#else
        v.jnum = static_cast<num_t> (n);
#endif
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
    jvalue& jvalue::operator= (jvalue&& value)
    {
        if (this != &value)
            move (std::forward<jvalue&&>(value));
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
            return false;
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
#if UJSON_HAVE_GMPXX
            return *v.jnum < *rval.v.jnum;
#else
            return v.jnum < rval.v.jnum;
#endif

        case j_bool:
            return v.jbool < rval.v.jbool;

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
#if UJSON_HAVE_GMPXX
            return *v.jnum == *rval.v.jnum;
#else
            return v.jnum == rval.v.jnum;
#endif

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
        if (jtype != j_object)
            throw ujson::json_type_error ("Not a JSON object");
        return *v.jobj;
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
        if (jtype != j_array)
            throw ujson::json_type_error ("Not a JSON array");
        return *v.jarray;
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
    std::string& jvalue::str ()
    {
        if (jtype != j_string)
            throw ujson::json_type_error ("Not a JSON string");
        return *v.jstr;
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


#if UJSON_HAVE_GMPXX
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    mpf_class& jvalue::mpf ()
    {
        if (jtype != j_number)
            throw ujson::json_type_error ("Not a JSON number");
        return *v.jnum;
    }
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    double jvalue::num () const
    {
        if (jtype != j_number)
            throw ujson::json_type_error ("Not a JSON number");
#if UJSON_HAVE_GMPXX
        return v.jnum->get_d();
#else
        return v.jnum;
#endif
    }


#if UJSON_HAVE_GMPXX
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::num (const mpf_class& n)
    {
        type (j_number);
        if (n.get_prec() > v.jnum->get_prec())
            v.jnum->set_prec (n.get_prec());
        *v.jnum = n;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::num (mpf_class&& n)
    {
        type (j_number);
        if (n.get_prec() > v.jnum->get_prec())
            v.jnum->set_prec (n.get_prec());
        *v.jnum = std::forward<mpf_class&&> (n);
    }
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::num (const double n)
    {
        type (j_number);
#if UJSON_HAVE_GMPXX
        std::stringstream ss;
        ss << std::setprecision(std::numeric_limits<double>::digits10 + 1) << n;
        *v.jnum = ss.str();
#else
        v.jnum = n;
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::num (const long n)
    {
        type (j_number);
#if UJSON_HAVE_GMPXX
        *v.jnum = n;
#else
        v.jnum = static_cast<num_t> (n);
#endif
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::boolean () const
    {
        if (jtype != j_bool)
            throw ujson::json_type_error ("Not a JSON boolean");

        return v.jbool;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::boolean (const bool b)
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
    // Free resources and set JSON type to null
    //--------------------------------------------------------------------------
    void jvalue::reset ()
    {
        switch (jtype) {
        case j_object:
            if (v.jobj) {
                delete v.jobj;
                v.jobj = nullptr;
            }
            break;

        case j_array:
            if (v.jarray) {
                delete v.jarray;
                v.jarray = nullptr;
            }
            break;

        case j_string:
            if (v.jstr) {
                delete v.jstr;
                v.jstr = nullptr;
            }
            break;

#if UJSON_HAVE_GMPXX
        case j_number:
            if (v.jnum) {
                delete v.jnum;
                v.jnum = nullptr;
            }
            break;
#endif
        default:
            break;
        }

        jtype = j_null;
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

        case j_number:
#if UJSON_HAVE_GMPXX
            v.jnum = new num_t (0);
#else
            v.jnum = 0.0;
#endif
            break;

        case j_bool:
            v.jbool = false;
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
    bool jvalue::has (const std::string& name) const
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
        if (type() != j_object) {
            // This is not a json object
            throw ujson::json_type_error ("Not a JSON object");
        }

        auto entry = find_last_in_jobj (name, *v.jobj);
        if (entry != v.jobj->send())
            return entry->second;

        // Name not found, return an invalid json value
        invalid_jvalue.type (j_invalid);
        return invalid_jvalue;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::get_unique (const std::string& name)
    {
        if (type() != j_object) {
            // This is not a json object
            throw ujson::json_type_error ("Not a JSON object");
        }

        auto items = v.jobj->equal_range (name);
        if (items.first == items.second) {
            // Name not found, return an invalid json value
            invalid_jvalue.type (j_invalid);
            return invalid_jvalue;
        }

        auto& value = items.first->second;

        // Check if there are more attributes with the same name
        ++items.first;
        if (items.first != items.second)
            throw std::logic_error ("JSON object member name not unique");

        return value;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator[] (const std::string& name)
    {
        if (type() != j_object)
            throw ujson::json_type_error ("Not a JSON object");

        auto entry = find_last_in_jobj (name, *v.jobj);
        if (entry != v.jobj->send()) {
            // Found a valid json value associated with 'name'
            return entry->second;
        }else{
            // 'name' not found, create an null json
            // value associated with 'name'.
            return v.jobj->emplace_back (name, jvalue(j_null)).second;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::operator[] (const size_t index)
    {
        if (type() != j_array)
            throw ujson::json_type_error ("Not a JSON array");

        if (index >= v.jarray->size())
            throw std::out_of_range ("Array index out of range");

        return v.jarray->operator[] (index);
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
            throw ujson::json_type_error ("Not a JSON object or array");
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::add (const std::string& name,
                         const jvalue& value,
                         const bool overwrite)
    {
        if (type() != j_object)
            throw ujson::json_type_error ("Not a JSON object");

        if (!value.valid())
            throw std::invalid_argument ("Invalid JSON value");

        auto entry = find_last_in_jobj (name, *v.jobj);
        if (entry == v.jobj->send() || overwrite==false) {
            return v.jobj->emplace_back(name, value).second;
        }else{
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
        if (type() != j_object)
            throw ujson::json_type_error ("Not a JSON object");

        if (!value.valid())
            throw std::invalid_argument ("Invalid JSON value");

        auto entry = find_last_in_jobj (name, *v.jobj);
        if (entry == v.jobj->send() || overwrite==false) {
            return v.jobj->emplace_back(name, std::forward<jvalue&&>(value)).second;
        }else{
            entry->second = std::forward<jvalue&&> (value);
            return entry->second;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::append (const jvalue& value)
    {
        if (type() != j_array)
            throw ujson::json_type_error ("Not a JSON array");

        if (!value.valid())
            throw std::invalid_argument ("Invalid JSON value");

        return v.jarray->emplace_back (value);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue& jvalue::append (jvalue&& value)
    {
        if (type() != j_array)
            throw ujson::json_type_error ("Not a JSON array");

        if (!value.valid())
            throw std::invalid_argument ("Invalid JSON value");

        return v.jarray->emplace_back (std::forward<jvalue&&>(value));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool jvalue::remove (const std::string& name)
    {
        return type()==j_object && v.jobj->erase(name) > 0;
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
#if UJSON_HAVE_GMPXX
            if (rval.v.jnum->get_prec() != v.jnum->get_prec())
                v.jnum->set_prec (rval.v.jnum->get_prec());
            *v.jnum = *rval.v.jnum;;
#else
            v.jnum = rval.v.jnum;
#endif
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
            rval.jtype = j_null;
            break;

        case j_array:
            v.jarray = rval.v.jarray;
            rval.v.jarray = nullptr;
            rval.jtype = j_null;
            break;

        case j_string:
            v.jstr = rval.v.jstr;
            rval.v.jstr = nullptr;
            rval.jtype = j_null;
            break;

        case j_number:
            v.jnum = rval.v.jnum;
#if UJSON_HAVE_GMPXX
            rval.v.jnum = nullptr;
#else
            rval.v.jnum = 0.0;
#endif
            rval.jtype = j_null;
            break;

        case j_bool:
            v.jbool = rval.v.jbool;
            rval.v.jbool = false;
            rval.jtype = j_null;
            break;

        case j_null:
            break;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string jvalue::describe (desc_format_t fmt,
                                  unsigned starting_indent_depth) const
    {
        std::stringstream ss;
        describe (ss, fmt, starting_indent_depth);
        return ss.str ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string jvalue::describe (bool pretty,
                                  bool array_items_on_same_line,
                                  bool sorted_properties,
                                  bool escape_slash,
                                  bool relaxed_mode,
                                  const std::string& indent) const
    {
        desc_format_t fmt;
        if (pretty) {
            fmt = fmt_pretty;
            fmt |= array_items_on_same_line ? fmt_compact_array : fmt_none;
            fmt |= sorted_properties ? fmt_sorted : fmt_none;
            fmt |= escape_slash ? fmt_escape_slash: fmt_none;
            fmt |= relaxed_mode ? fmt_relaxed : fmt_none;
        }else{
            fmt = fmt_none;
        }
        return describe (fmt);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    inline static void put_indent (std::stringstream& ss,
                                   desc_format_t fmt,
                                   unsigned depth)
    {
        ss << std::endl;
        for (unsigned n=0; n<depth; ++n) {
            if (fmt & fmt_tabs)
                ss << '\t';
            else
                ss << "    ";
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::describe (std::stringstream& ss,
                           desc_format_t fmt,
                           unsigned indent_depth) const
    {
        switch (type()) {
        case j_object:
            describe_object (ss, fmt, indent_depth);
            break;

        case j_array:
            describe_array (ss, fmt, indent_depth);
            break;

        case j_string:
            if ((fmt & fmt_color) && HAS_COLOR) {
                ss << '"';
                ss << color_string << escape(*v.jstr, (fmt&fmt_escape_slash)) << color_normal;
                ss << '"';
            }else{
                ss << '"' << escape(*v.jstr, fmt&fmt_escape_slash) << '"';
            }
            break;

        case j_number:
#if UJSON_HAVE_GMPXX
            num_t_to_str (const_cast<jvalue*>(this)->mpf(), ss);
#else
            if (std::isinf(num()) || std::isnan(num()))
                ss << "null";
            else
                ss << std::setprecision(std::numeric_limits<num_t>::digits10 + 1) << num();
#endif
            break;

        case j_bool:
            if ((fmt & fmt_color) && HAS_COLOR) {
                if (boolean())
                    ss << color_boolean_true << "true" << color_normal;
                else
                    ss << boolean_false_color << "false" << color_normal;
            }else{
                ss << (boolean() ? "true" : "false");
            }
            break;

        case j_null:
            if ((fmt & fmt_color) && HAS_COLOR)
                ss << null_color << "null" << color_normal;
            else
                ss << "null";
            break;

        case j_invalid:
        default:
            break;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::describe_object (std::stringstream& ss,
                                  desc_format_t fmt,
                                  unsigned indent_depth) const
    {
        static const std::regex re_identifier ("[_a-zA-Z][_a-zA-Z0-9]*",
                                               std::regex::ECMAScript);
        static const std::regex re_reserved ("([tT][rR][uU][eE])|"
                                             "([fF][aA][lL][sS][eE])|"
                                             "([nN][uU][lL][lL])",
                                             std::regex::ECMAScript);
        std::cmatch re_match;
        bool first {true};

        if ((fmt & fmt_color) && HAS_COLOR)
            ss << object_color << '{' << color_normal;
        else
            ss << '{';

        auto& members = *v.jobj;

        bool one_liner = members.size()==1 &&
            (members.front().second.is_container()==false || members.front().second.size()==0);

        if (!members.empty()) {
            auto i = (fmt & fmt_sorted) ? members.sbegin() : members.begin();
            auto member_end = (fmt & fmt_sorted) ? members.send() : members.end();
            for (; i!=member_end; ++i) {
                if (! i->second.valid())
                    continue; // Skip invalid values
                auto& name = i->first;
                auto& value = i->second;
                bool quoted_name = true;
                if (fmt & fmt_relaxed) {
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
                if (fmt & fmt_pretty) {
                    if (!one_liner)
                        put_indent (ss, fmt, indent_depth+1); //ss << std::endl << indent;
                }

                if ((fmt & fmt_color) && HAS_COLOR) {
                    if (quoted_name) {
                        ss << '"' << attribute_color << escape(name, (fmt & fmt_escape_slash)) << color_normal << '"';
                    }else{
                        ss << attribute_color << name << color_normal;
                    }
                }else{
                    if (quoted_name)
                        ss << '"' << escape(name, (fmt & fmt_escape_slash)) << '"';
                    else
                        ss << name;
                }

                ss << ((fmt & fmt_pretty) ? ": " : ":");
                value.describe (ss, fmt, indent_depth+1);
            }
        }
        if ((fmt & fmt_pretty) && !first) {
            if (!one_liner)
                put_indent (ss, fmt, indent_depth); //ss << std::endl << indent;
        }
        if ((fmt & fmt_color) && HAS_COLOR)
            ss << object_color << '}' << color_normal;
        else
            ss << '}';
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jvalue::describe_array (std::stringstream& ss,
                                 desc_format_t fmt,
                                 unsigned indent_depth) const
    {
        auto& elements = *v.jarray;
        if (elements.empty()) {
            if ((fmt & fmt_color) && HAS_COLOR)
                ss << array_color << "[]" << color_normal;
            else
                ss << "[]";
            return;
        }

        unsigned next_indent_depth = indent_depth;
        bool same_line = fmt & fmt_compact_array;

        if (elements.size()==1 &&
            (elements[0].is_container()==false || elements[0].size()==0))
        {
            same_line = true;
        }
        if (!same_line)
            ++next_indent_depth;

        if ((fmt & fmt_color) && HAS_COLOR)
            ss << array_color << '[' << color_normal;
        else
            ss << '[';

        bool first {true};
        for (auto& e : elements) {
            if (!e.valid())
                continue; // Skip invalid values
            if (!first)
                ss << ',';
            if ((fmt & fmt_pretty)) {
                if (fmt & same_line) {
                    if (!first)
                        ss << ' ';
                }else{
                    put_indent (ss, fmt, next_indent_depth);
                }
            }
            first = false;
            e.describe (ss, fmt, next_indent_depth);
        }

        if ((fmt & fmt_pretty) && !(fmt & same_line))
            put_indent (ss, fmt, indent_depth);
        if ((fmt & fmt_color) && HAS_COLOR)
            ss << array_color << ']' << color_normal;
        else
            ss << ']';
    }


}
