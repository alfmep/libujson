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
#ifndef UJSON_JVALUE_HPP
#define UJSON_JVALUE_HPP

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <cstdio>
#include <ujson/multimap_list.hpp>


namespace ujson {

    /**
     * Type of json value.
     */
    enum jvalue_type {
        j_invalid, /**< An invalid json type. */
        j_object,  /**< A json object. */
        j_array,   /**< A json array. */
        j_string,  /**< A json string. */
        j_number,  /**< A json number. */
        j_bool,    /**< A json boolean(true or false). */
        j_null     /**< A json null value. */
    };


    // Forward declaration.
    class jvalue;


    /**
     * A jvalue associated with a string,
     * representing a json object member.
     */
    using json_pair = std::pair<std::string, jvalue>;

    /**
     * A representation of a json object.
     * A json object is a collection of named json values.
     */
    using json_object = multimap_list<std::string, jvalue>;

    /**
     * A vector of jvalue objects.
     * This represents a json array.
     */
    using json_array = std::vector<jvalue>;


    /**
     * A representation of a Json value.
     */
    class jvalue {
    public:
        /**
         * Constructor.
         * Creates a json value with a default value of the specified type.
         * Default is type j_invalid.
         * @param vtype The type of json value.
         */
        jvalue (jvalue_type vtype=j_invalid);

        /**
         * Copy constructor.
         * Makes a copy of another jvalue.
         * @param jvalue The json value to copy.
         */
        jvalue (const jvalue& jvalue);

        /**
         * Move constructor.
         * Move the content of another jvalue to this object.
         * @param jvalue The json value to move.
         */
        jvalue (jvalue&& jvalue);

        /**
         * Create a jvalue of type j_object and
         * make a copy of the supplied json object.
         * @param o The json object to copy.
         */
        jvalue (const json_object& o);

        /**
         * Create a jvalue of type j_object and
         * move the supplied json object.
         * @param o The json object to move.
         */
        jvalue (json_object&& o);

        /**
         * Create a jvalue of type j_array and
         * make a copy of the supplied json array.
         * @param a The json array to copy.
         */
        jvalue (const json_array& a);

        /**
         * Create a jvalue of type j_array and
         * move the supplied json array to this jvalue.
         * @param a The json array to move.
         */
        jvalue (json_array&& a);

        /**
         * Create a jvalue of type j_string.
         * <br/><b>Note:</b> The string is copied 'as is',
         * no assumptions are made about json escape
         * sequences in the string. It is up to the
         * caller to make sure that the string is a
         * valid un-escaped UTF-8 string.
         * @param s The string value to copy.
         * @see ujson::unescape
         */
        jvalue (const std::string& s);

        /**
         * Create a jvalue of type j_string.
         * <br/><b>Note:</b> The string is copied 'as is',
         * no assumptions are made about json escape
         * sequences in the string. It is up to the
         * caller to make sure that the string is a
         * valid un-escaped UTF-8 string.
         * @param s The string value to move.
         * @see ujson::unescape
         */
        jvalue (std::string&& s);

        /**
         * Create a jvalue of type j_string.
         * <br/><b>Note:</b> The string is copied 'as is',
         * no assumptions are made about json escape
         * sequences in the string. It is up to the
         * caller to make sure that the string is a
         * valid un-escaped UTF-8 string.
         * @param s The string value to copy.
         * @see ujson::unescape
         */
        jvalue (const char* s);

        /**
         * Create a jvalue of type j_number.
         * @param n The json number value.
         */
        jvalue (const double n);

        /**
         * Create a jvalue of type j_number.
         * @param n The json number value.
         */
        jvalue (const int n);

        /**
         * Create a jvalue of type j_bool.
         * @param true_false A boolean value representing
         *                   a json <code>true</code> or
         *                   <code>false</code> value.
         */
        explicit jvalue (const bool true_false);

        /**
         * Create a jvalue of type j_null.
         * @param nil A null pointer.
         */
        explicit jvalue (const std::nullptr_t nil);

        /**
         * Destructor.
         * Free internal resources.
         * Any references returned by this objects
         * methods will no longer be valid and should
         * not be used ant more.
         */
        ~jvalue ();

        /**
         * Assignment operator.
         * Make a copy of another jvalue object.
         * @param jvalue The json value to copy.
         */
        jvalue& operator= (const jvalue& jvalue);

        /**
         * Move operator.
         * Move the contents of another jvalue to this object.
         * @param jvalue The json value to move.
         */
        jvalue& operator= (jvalue&& jvalue);

        /**
         * Assignment operator.
         * Set the value type to j_object and copy the
         * content of the supplied json_object to this object.
         * @param o A json_object to be copied.
         */
        jvalue& operator= (const json_object& o);

        /**
         * Move operator.
         * Set the value type to j_object and move the
         * content of the supplied json_object to this object.
         * @param o A json_object to be moved.
         */
        jvalue& operator= (json_object&& o);

        /**
         * Assignment operator.
         * Set the value type to j_array and copy the
         * content of the supplied json_array to this object.
         * @param o A json_array to be copied.
         */
        jvalue& operator= (const json_array& aptr);

        /**
         * Move operator.
         * Set the value type to j_array and move the
         * content of the supplied json_array to this object.
         * @param o A json_array to be moved.
         */
        jvalue& operator= (json_array&& aptr);

        /**
         * Assignment operator.
         * Make this a value of type j_string.
         * <br/><b>Note:</b> The string is copied 'as is',
         * no assumptions are made about json escape
         * sequences in the string. It is up to the
         * caller to make sure that the string is a
         * valid un-escaped UTF-8 string.
         * @param s The new string value to copy.
         * @see ujson::unescape
         */
        jvalue& operator= (const std::string& s) {str(s); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type j_string.
         * <br/><b>Note:</b> The string is copied 'as is',
         * no assumptions are made about json escape
         * sequences in the string. It is up to the
         * caller to make sure that the string is a
         * valid un-escaped UTF-8 string.
         * @param s The new string value to copy.
         * @see ujson::unescape
         */
        jvalue& operator= (const char* s) {str((s==nullptr?"":s)); return *this;}

        /**
         * Move operator.
         * Make this a value of type j_string.
         * <br/><b>Note:</b> The string is copied 'as is',
         * no assumptions are made about json escape
         * sequences in the string. It is up to the
         * caller to make sure that the string is a
         * valid un-escaped UTF-8 string.
         * @param s The string to move to this object.
         * @see ujson::unescape
         */
        jvalue& operator= (std::string&& s) {str(std::move(s)); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type j_number.
         * @param n The new number value.
         */
        jvalue& operator= (double n) {num(n); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type j_number.
         * @param n The new number value.
         */
        jvalue& operator= (int n) {num(static_cast<double>(n)); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type j_bool.
         * @param true_false The new boolean value.
         */
        jvalue& operator= (bool true_false) {boolean(true_false); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type null.
         * @param nil A null pointer.
         */
        jvalue& operator= (const std::nullptr_t nil) { type(j_null); return *this; }

        /**
         * Less-than operator.
         */
        bool operator< (const jvalue& rval) const;

        /**
         * Comparison operator.
         * @return <code>true</code> if the json values are equal.
         *         <code>true</code> if not. Note that two invalid
         *         json values are not considered equal.
         */
        bool operator== (const jvalue& rval) const;

        /**
         * Comparison operator.
         * @return <code>true</code> if the json values are not equal.
         *         <code>true</code> if the json values are equal.
         *         Note that two invalid json values are not considered equal.
         */
        bool operator!= (const jvalue& rval) const { return !(operator==(rval)); }

        /**
         * Get a reference to the json_object
         * if this jvalue is of type j_object.
         * If this value is not of type j_object,
         * a reference to a static empty json_object is returned.
         * This empty json_object will be resetted the next
         * time this method is called, so it is a good thing
         * to first check that this jvalue is of type j_object.
         * @return A reference to a json_object.
         */
        json_object& obj ();

        /**
         * Assign a json_object to this jvalue.
         * Set the value type to j_object and copy the
         * content of the supplied json_object.
         * @param o A json_object to be copied.
         */
        void obj (const json_object& o);

        /**
         * Move a json_object to this jvalue.
         * Set the value type to j_object and move the
         * content of the supplied json_object to this objec.
         * @param o A json_object to be moved.
         */
        void obj (json_object&& optr);

        /**
         * Get a reference to the json_array
         * if this jvalue is of type j_array.
         * If this value is not of type j_array,
         * a reference to a static empty json_array is returned.
         * This empty json_array will be resetted the next
         * time this method is called, so it is a good thing
         * to first check that this jvalue is of type j_array.
         * @return A reference to a json_array.
         */
        json_array& array ();

        /**
         * Assign a json_array to this jvalue.
         * Set the value type to j_array and copy the
         * content of the supplied json_array.
         * @param a A json_array to be copied.
         */
        void array (const json_array& a);

        /**
         * Move a json_array to this jvalue.
         * Set the value type to j_array and move the
         * content of the supplied json_array to this object.
         * @param a A json_array to be moved.
         */
        void array (json_array&& a);

        /**
         * Get a reference to the json string value.
         * If this value is not of type j_string,
         * a reference to a static empty string is returned.
         * This empty string will be resetted the next
         * time this method is called, so it is a good thing
         * to first check that this jvalue is of type j_string.
         * @return A reference to the json string value.
         */
        const std::string& str () const;

        /*
         * Make this a value of type j_string.
         * <br/><b>Note:</b> The string is copied 'as is',
         * no assumptions are made about json escape
         * sequences in the string. It is up to the
         * caller to make sure that the string is a
         * valid un-escaped UTF-8 string.
         * @param s The string value to copy.
         * @see ujson::unescape
         */
        void str (const std::string& s);

        /*
         * Make this a value of type j_string.
         * <br/><b>Note:</b> No assumptions are made
         * about json escape sequences in the string.
         * It is up to the caller to make sure that
         * the string is a valid un-escaped UTF-8 string.
         * @param s The string value to move to this object.
         * @see ujson::unescape
         */
        void str (std::string&& s);

        /**
         * Return the json number value.
         * @return The json number value.
         *         If this value isn't of type j_number,
         *         0.0 is returned.
         */
        double num () const;

        /**
         * Make this a value of type j_number.
         * @param n The new number value.
         */
        void num (double n);

        /**
         * Make this a value of type j_number.
         * @param n The new number value.
         */
        void num (int n) {num(static_cast<double>(n));}

        /**
         * Return the json boolean value.
         * @return The json boolean value.
         *         If this value isn't of type j_bool,
         *         <code>false</code> is returned.
         */
        bool boolean () const;

        /**
         * Make this a value of type j_bool.
         * @param b The new boolean value.
         */
        void boolean (bool b);

        /**
         * Check if this a json null value.
         * @return <code>true</code> if this value
         *         is of type j_bool.
         */
        bool is_null () const {return jtype==j_null;}

        /**
         * Set the json value type to j_null.
         */
        void set_null ();

        /**
         * Check if this is a valid json object.
         */
        bool valid () const {return jtype!=j_invalid;}

        /**
         * Set the json value type to j_invalid.
         */
        void reset ();

        /**
         * Return the type of json value this object has.
         * @return The json value type.
         */
        jvalue_type type () const {return jtype;}

        /**
         * Check if this is a container.
         * @return <code>true</code> if this is an object or an array.
         */
        bool is_container () const;

        /**
         * Set the type of json value.
         * If the json value already is of the the same type nothing is done.
         * If not, the json value is resetted and initialized to the
         * default value of the requested type.
         * @param t The json value type.
         */
        void type (const jvalue_type t);

        /**
         * Check if the json object contains a
         * valid json value associated with a given name.
         * @param name The name to look for.
         * @return <code>true</code> if this is a json object
         *         and it has a valid value associated with
         *         the given name. Otherwise <code>false</code>.
         */
        bool have (const std::string& name) const;

        /**
         * If this is a json object, return the last json value
         * in the json object that is associated with the supplied name.
         * If no value associated with the specified name is found,
         * or if this is not a json object,
         * a reference to an invalid static jvalue is returned.
         * This invalid jvalue will be resetted the next
         * time this method is called.
         * It is a good thing to first check that this jvalue
         * is of type j_object.
         * @param name The name of the value we are searching for.
         * @return A reference to the last value mapped to the given name.
         */
        jvalue& get (const std::string& name);

        /**
         * If this is a json object, return the last json value
         * in the json object that is associated with the supplied name.
         * If no value associated with the specified name is found,
         * a new invalid json value is created and associated with
         * the supplied name.<br/>
         * If this is not a json object,
         * a reference to an invalid static jvalue is returned.
         * This invalid jvalue will be resetted the next
         * time this method is called.
         * It is a good thing to first check that this jvalue
         * is of type j_object.
         * @param name The name of the value we are searching for.
         * @return A reference to the last value mapped to the given name.
         */
        jvalue& operator[] (const std::string& name);

        /**
         * If this is a json array, return a reference to the n'th value.
         * If the index is out of bounds, or if this is not a json array,
         * a reference to an invalid static jvalue is returned.
         * This invalid jvalue will be resetted the next
         * time this method is called.
         * It is a good thing to first check that this jvalue
         * is of type j_object.
         * @return A reference to a jvalue in the json array.
         */
        jvalue& operator[] (const size_t n);

        /**
         * Return the number of json values in an array or an object.
         * Note that the size returned by this method will include any
         * invalid json values in the json object or json array.
         * @return If this is an array, return the number of values in the array.
         *         It this is an object, return the number of string value pairs
         *         in the object.
         *         Returns 0 if not an array or object.
         */
        size_t size () const;

        /**
         * Add a named json value to a json object.
         * If this object isn't a json object, the
         * method will return <code>false</code>.
         * @param name The name of the json value.
         * @param value The json value to add.
         * @param overwrite If <code>true</code> and 'name'
         *                  already exists, it will be overwritten
         *                  with the new value.
         *                  If <code>false</code> and 'name' already
         *                  exists and the current value is a valid jvalue,
         *                  it will not be overwritten.
         * @return A reference to the added jsaon value.
         */
        jvalue& add (const std::string& name, const jvalue& value,
                     const bool overwrite=true);

        /**
         * Add a named json value to a json object.
         * If this object isn't a json object, the
         * method will return <code>false</code>.
         * @param name The name of the json value.
         * @param value The json value to move to this object.
         * @param overwrite If <code>true</code> and 'name'
         *                  already exists, it will be overwritten
         *                  with the new value.
         *                  If <code>false</code> and 'name' already
         *                  exists and the current value is a valid jvalue,
         *                  it will not be overwritten.
         * @return A reference to the added jsaon value.
         */
        jvalue& add (const std::string& name, jvalue&& value,
                     const bool overwrite=true);

        /**
         * Add a json value to a json array.
         * If this object isn't a json array, the
         * method will return <code>false</code>.
         * @param value The json value to add to the array.
         * @return A reference to the added jsaon value.
         */
        jvalue& add (const jvalue& value);

        /**
         * Add a json value to a json array.
         * If this object isn't a json array, the
         * method will return <code>false</code>.
         * @param value The json value to move to the array.
         * @return A reference to the added jsaon value.
         */
        jvalue& add (jvalue&& value);

        /**
         * Remove a named json value from a json object.
         * If this object isn't a json object or the
         * named value doesn't exist, the
         * method will return <code>false</code>.
         * @param name The name of the json value to be removed.
         * @return <code>true</code> if the value associated with
         *         the specified name was removed.
         *         <code>false</code> if this is not a json object,
         *         or it doesn't contain a value with the
         *         specified name.
         */
        bool remove (const std::string& name);

        /**
         * Remove the n'th value from a json array.
         * If this object isn't a json array, or the
         * index <code>n</code> is out of bounds, the
         * method will return <code>false</code>.
         * @param n The index in the json array to be removed.
         * @return <code>true</code> if the n'th item in
         *         the array was removed.
         *         <code>false</code> if this is not a json array,
         *         or the value <code>n</code> is out of bounds.
         */
        bool remove (const size_t n);

        /**
         * Return a string representation of this json value.
         * All string output(object member names and string
         * values) will be json encoded using ujson::escape.
         * Any invalid json values (of type ujson::j_invalid)
         * will not be included in the string representation.
         * @param pretty If <code>true</code>, return a string
         *               with each value on a separate line and
         *               use indentation to make it more readable.
         *               If <code>false</code>, return a compact
         *               string with everrything on a single line.
         * @param strict If <code>true</code>, all numbers that
         *               are (+/-)infinite or NaN(not a number),
         *               will be described as type 'null'.
         * @param escape_slash If <code>true</code>, the
         *                     forward slash character "/" will
         *                     be esacped to "\/".
         * @param sorted_properties If <code>true</code>, the
         *                          properties of objects will be
         *                          listed in a sorted order and
         *                          not in the same order they were
         *                          added.
         * @param indent Indentation to use if <code>pretty</code>
         *               is <code>true</code>. Normally zero or more
         *               space characters.
         * @return A string in json format describing this json value.
         * @see ujson::escape
         */
        std::string describe (bool pretty=false,
                              bool strict=false,
                              bool escape_slash=false,
                              bool sorted_properties=false,
                              const std::string& indent="    ") const
        {
            return describe (pretty, strict, escape_slash, sorted_properties, "", indent);
        }

        /**
         * Return a string representation of this json value.
         * All string output(object member names and string
         * values) will be json encoded using ujson::escape.
         * Any invalid json values (of type ujson::j_invalid)
         * will not be included in the string representation.
         * @param pretty If <code>true</code>, return a string
         *               with each value on a separate line and
         *               use indentation to make it more readable.
         *               If <code>false</code>, return a compact
         *               string with everrything on a single line.
         * @param strict If <code>true</code>, all numbers that
         *               are (+/-)infinite or NaN(not a number),
         *               will be described as type 'null'.
         * @param escape_slash If <code>true</code>, the
         *                     forward slash character "/" will
         *                     be esacped to "\/".
         * @param sorted_properties If <code>true</code>, the
         *                          properties of objects will be
         *                          listed in a sorted order and
         *                          not in the same order they were
         *                          added.
         * @param relaxed_obj_member_names This parameter is only
         *               relevant if parameter <code>strict</code>
         *               is <code>false</code>.<br/>
         *               If this parameter is <code>true</code>,
         *               print object member names without enclosing double
         *               quotes if the object member names are in the
         *               following format: [_a-zA-Z][_a-zA-Z0-9]*
         * @param indent Indentation to use if <code>pretty</code>
         *               is <code>true</code>. Normally zero or more
         *               space characters.
         * @return A string in json format describing this json value.
         * @see ujson::escape
         */
        std::string describe (bool pretty,
                              bool strict,
                              bool escape_slash,
                              bool sorted_properties,
                              bool relaxed_obj_member_names,
                              const std::string& indent="    ") const
        {
            return describe (pretty, strict, escape_slash, sorted_properties,
                             relaxed_obj_member_names, "", indent);
        }


    private:
        jvalue_type jtype;
        union {
            json_object* jobj;
            json_array*  jarray;
            std::string* jstr;
            double       jnum;
            bool         jbool;
        } v;

        void copy (const jvalue& jvalue);
        void move (jvalue&& jvalue);

        json_object::iterator find_last_in_jobj (const std::string& key);
        std::string describe (bool pretty,
                              bool strict,
                              bool escape_slash,
                              bool sorted_properties,
                              const std::string& first_indent,
                              const std::string& indent_step) const;
        std::string describe (bool pretty,
                              bool strict,
                              bool escape_slash,
                              bool sorted_properties,
                              bool relaxed_obj_member_names,
                              const std::string& first_indent,
                              const std::string& indent_step) const;
        std::string describe_object (bool pretty,
                                     bool strict,
                                     bool escape_slash,
                                     bool sorted_properties,
                                     bool relaxed_obj_member_names,
                                     const std::string& first_indent,
                                     const std::string& indent_step) const;
        std::string describe_array (bool pretty,
                                    bool strict,
                                    bool escape_slash,
                                    bool sorted_properties,
                                    bool relaxed_obj_member_names,
                                    const std::string& first_indent,
                                    const std::string& indent_step) const;
    };

    /*
    bool operator== (const json_object& lhs, const json_object& rhs);
    inline bool operator!= (const json_object& lhs, const json_object& rhs) {
        return ! (lhs == rhs);
    }

    bool operator== (const json_array& lhs, const json_array& rhs);
    inline bool operator!= (const json_array& lhs, const json_array& rhs) {
        return ! (lhs == rhs);
    }
    */
}
#endif
