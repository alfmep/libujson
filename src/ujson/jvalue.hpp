/*
 * Copyright (C) 2017,2019-2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <sstream>
#include <vector>
#include <list>
#include <memory>
#include <cstdio>
#include <cstdint>
#include <ujson/multimap_list.hpp>
#include <ujson/json_type_error.hpp>
#include <ujson/config.hpp>
#if UJSON_HAVE_GMPXX
#  include <gmpxx.h>
#endif


namespace ujson {


#if UJSON_HAVE_GMPXX
    /**
     * Utility function to convert a number of
     * type <code>mpf_class</code> to a string.
     * @param number The number to converto to a string.
     * @return The number as a string.
     */
    std::string to_string (const mpf_class& number);
#endif


    /**
     * Flags used in method jvalue::describe() to
     * format the output string.
     */
    enum desc_format_t : uint16_t {
        /**
         * If no flag is set, the output will be without any whitespace.
         */
        fmt_none         = 0x00,

        /**
         * If set, whitespaces (line brake and indentation) are used
         * to make the description more readable.<br/>
         * All flags below, except fmt_color,
         * depends on this flag being set.
         */
        fmt_pretty       = 0x01,

        /**
         * If set, each array element is printed on a separate line.<br/>
         * If not set, all array elements are printed on one line
         * (any object in the array will still be printed
         * with each attribute indented on a separate line).<br/>
         */
        fmt_sep_elements = 0x02,

        /**
         * If set, the properties of JSON objects will be
         * printed sorted by name, and not in natural order
         * (the order they were inserted).
         */
        fmt_sorted       = 0x04,

        /**
         * If set, the forward slash character "/" will
         * be esacped to "\/" in JSON strings.
         */
        fmt_escape_slash = 0x08,

        /**
         * If set, indent using TAB instead of four
         * space(' ') characters for each indentation depth.
         */
        fmt_tabs         = 0x10,

#if (UJSON_HAS_CONSOLE_COLOR)
        /**
         * If set, print any object member name without
         * enclosing double quotes if the object member
         * name is in the following format: [_a-zA-Z][_a-zA-Z0-9]*
         */
        fmt_color        = 0x20,
#endif
        /**
         * If set, print any object member name without
         * enclosing double quotes if the object member
         * name is in the following format: [_a-zA-Z][_a-zA-Z0-9]*
         */
        fmt_relaxed      = 0x40,

        fmt_mask         = 0x7f,
    };
    /**  Bitwise OR (desc_format_t | desc_format_t). */
    inline desc_format_t operator| (const desc_format_t& lhs, const desc_format_t& rhs) {
        return (desc_format_t) ((const uint16_t)lhs | (const uint16_t)rhs);
    }
    /**  Bitwise OR assignment (desc_format_t lhs |= desc_format_t lhs). */
    inline desc_format_t& operator|= (desc_format_t& lhs, const desc_format_t& rhs) {
        return (desc_format_t&) ((uint16_t&)lhs |= (const uint16_t&)rhs);
    }

    /**  Bitwise AND (desc_format_t & desc_format_t). */
    inline desc_format_t operator& (const desc_format_t& lhs, const desc_format_t& rhs) {
        return (desc_format_t) ((const uint16_t)lhs & (const uint16_t)rhs);
    }
    /**  Bitwise AND assignment (desc_format_t lhs &= desc_format_t lhs). */
    inline desc_format_t& operator&= (desc_format_t& lhs, const desc_format_t& rhs) {
        return (desc_format_t&) ((uint16_t&)lhs &= (const uint16_t&)rhs);
    }

    /**  Bitwise XOR (desc_format_t ^ desc_format_t). */
    inline desc_format_t operator^ (const desc_format_t& lhs, const desc_format_t& rhs) {
        return (desc_format_t) ((const uint16_t)lhs ^ (const uint16_t)rhs);
    }
    /**  Bitwise XOR assignment (desc_format_t lhs ^= desc_format_t lhs). */
    inline desc_format_t& operator^= (desc_format_t& lhs, const desc_format_t& rhs) {
        return (desc_format_t&) ((uint16_t&)lhs ^= (const uint16_t&)rhs);
    }

    /**  Bitwise NOT (~desc_format_t). */
    inline desc_format_t operator~ (const desc_format_t& a) {
        return (desc_format_t) ~(const uint16_t&)a & fmt_mask;
    }


    /**
     * Type of JSON value.
     * Instances of class ujson::jvalue represents
     * one of the JSON types described here.<br/>
     * Method jvalue::type() returns the JSON type
     * (jvalue_type) that the jvalue instance represents.
     */
    enum jvalue_type {
        j_invalid, /**< An invalid JSON type.
                    * Calling method jvalue::describe() on a ujson::jvalue
                    * with an invalid JSON type will return an empty string.
                    * <br/>The method jvalue::valid() will return
                    * <code>false</code> if the <code>jvalue_type</code>
                    * is <code>j_invalid</code>, and <code>true</code>
                    * if not.
                    * @note In libujson, a function or method returning a
                    * reference to a ujson::jvalue will typically return a
                    * static instance with the <code>jvalue_type</code> set
                    * to <code>j_invalid</code> as an error when a real JSON
                    * type can't be returned. */
        j_object,  /**< A JSON object.
                    * Class ujson::jvalue encapsulates a ujson::json_object to
                    * represent a JSON object. */
        j_array,   /**< A JSON array.
                    * Class ujson::jvalue encapsulates a ujson::json_array to
                    * represent a JSON array. */
        j_string,  /**< A JSON string.
                    * Class ujson::jvalue encapsulates a std::string to
                    * represent a JSON string.
                    * @note Calling method jvalue::describe() on a JSON string
                    * will return a JSON encoded string enclosed by double
                    * quotes that can be parsed as a valid JSON instance.
                    * To get the un-encoded string value without enclosing
                    * double quotes, call method jvalue::str(). */
        j_number,
#if UJSON_HAVE_GMPXX
                   /**< A JSON number.
                    * Class ujson::jvalue encapsulates a <code>mpf_class</code>
                    * to represent a JSON number that can have an arbitrary
                    * precision.
                    * @note Calling method jvalue::num() will return a value
                    * of type <code>double</code> and may lose precision. This
                    * is to be compatible with libujson built without support
                    * for gmpxx (GNU MP C++ API). To get the number with full
                    * precision when compiled with support for gmpxx,
                    * call jvalue::mpf() which returns a number of type
                    * mpf_class.<br/>
                    * The macro <code>UJSON_HAVE_GMPXX</code> will be set to 1
                    * when libujson is built with support for gmpxx.
                    * @see <a href=https://gmplib.org/manual/index
                    *       rel="noopener noreferrer" target="_blank">
                    *       GNU MP - C++ Class Interface</a>
                    */
#else
                   /**< A JSON number.
                    * Class ujson::jvalue encapsulates a <code>double</code>
                    * to represent a JSON number. */
#endif
        j_bool,    /**< A JSON boolean (<code>true</code> or <code>false</code>).
                    * Class ujson::jvalue encapsulates a <code>bool</code>
                    * to represent a JSON boolean. */
        j_null     /**< A JSON <code>null</code> value. */
    };


    // Forward declaration.
    class jvalue;


    /**
     * A jvalue associated with a string,
     * representing a JSON object attribute.
     */
    using json_pair = std::pair<std::string, jvalue>;

    /**
     * A representation of a JSON object.
     * A JSON object is a collection of named JSON values.
     */
    using json_object = multimap_list<std::string, jvalue>;

    /**
     * A representation of a JSON array.
     * This represents a JSON array.
     */
    using json_array = std::vector<jvalue>;


    /**
     * A representation of a JSON value.
     */
    class jvalue {
    public:
        /**
         * Default constructor.
         * Creates JSON null value.
         * @see jvalue::type().
         */
        jvalue ();

        /**
         * Constructor.
         * Creates a jvalue of a specific JSON type with its default value.
         *  - The default value of a JSON number is 0.
         *  - The default value of a JSON boolean is <code>false</code>.
         *  - The default value of a JSON string is an empty string.
         *  - The default value of a JSON object is an empty object without any attributes.
         *  - The default value of a JSON array is an empty array without any elements.
         *  - A JSON null value is... well... null.
         *
         * @param vtype The type of JSON value.
         * @see ujson::jvalue_type
         * @see jvalue::type()
         */
        jvalue (jvalue_type vtype);

        /**
         * Copy constructor.
         * Makes a copy of another jvalue.
         * @param value The jvalue to copy.
         */
        jvalue (const jvalue& value);

        /**
         * Move constructor.
         * Move the content of another jvalue to this object.
         * @param value The jvalue to move.
         */
        jvalue (jvalue&& value);

        /**
         * Create a jvalue of type ujson::j_object and
         * make a copy of the supplied json_object.
         * @param o The json_object to copy.
         */
        jvalue (const json_object& o);

        /**
         * Create a jvalue of type ujson::j_object and
         * move the supplied json_object.
         * @param o The json_object to move.
         */
        jvalue (json_object&& o);

        /**
         * Create a jvalue of type ujson::j_array and
         * make a copy of the supplied JSON array.
         * @param a The json_array to copy.
         */
        jvalue (const json_array& a);

        /**
         * Create a jvalue of type ujson::j_array and
         * move the supplied json_array to this jvalue.
         * @param a The json_array to move.
         */
        jvalue (json_array&& a);

        /**
         * Create a jvalue of type ujson::j_string.
         * @param s The string value to copy.
         * @note The string is copied <em>as is</em>,
         *       no assumptions are made about JSON escape
         *       sequences in the string. It is up to the
         *       caller to make sure that the string is a
         *       valid un-escaped UTF-8 string.
         * @see ujson::unescape
         */
        jvalue (const std::string& s);

        /**
         * Create a jvalue of type ujson::j_string.
         * @param s The string value to move.
         * @note The string is moved <em>as is</em>,
         *       no assumptions are made about json escape
         *       sequences in the string. It is up to the
         *       caller to make sure that the string is a
         *       valid un-escaped UTF-8 string.
         * @see ujson::unescape
         */
        jvalue (std::string&& s);

        /**
         * Create a jvalue of type ujson::j_string.
         * @param s The string value to copy.
         *          A <code>nullptr</code> is treated as an empty string.
         * @note The string is copied <em>as is</em>,
         *       no assumptions are made about json escape
         *       sequences in the string. It is up to the
         *       caller to make sure that the string is a
         *       valid un-escaped UTF-8 string.
         * @see ujson::unescape
         */
        jvalue (const char* s);

#if UJSON_HAVE_GMPXX
        /**
         * Create a jvalue of type ujson::j_number.
         * @param number The mpf_class instance to copy.
         * @note The macro <code>UJSON_HAVE_GMPXX</code> will be set to 1
         *       when libujson is built with support for gmpxx.
         * @see <a href=https://gmplib.org/manual/index
         *       rel="noopener noreferrer" target="_blank">
         *       GNU MP - C++ Class Interface</a>
         */
        jvalue (const mpf_class& number);

        /**
         * Create a jvalue of type ujson::j_number.
         * @param number The mpf_class instance to move.
         * @note The macro <code>UJSON_HAVE_GMPXX</code> will be set to 1
         *       when libujson is built with support for gmpxx.
         * @see <a href=https://gmplib.org/manual/index
         *       rel="noopener noreferrer" target="_blank">
         *       GNU MP - C++ Class Interface</a>
         */
        jvalue (mpf_class&& number);
#endif
        /**
         * Create a jvalue of type ujson::j_number.
         * @param number The number value.
         */
        jvalue (const double number);

        /**
         * Create a jvalue of type ujson::j_number.
         * @param number The number value.
         */
        jvalue (const int number);

        /**
         * Create a jvalue of type ujson::j_number.
         * @param number The number value.
         */
        explicit jvalue (const long number);

        /**
         * Create a jvalue of type ujson::j_bool.
         * @param true_false A boolean value representing
         *                   a JSON <code>true</code> or
         *                   <code>false</code> value.
         */
        jvalue (const bool true_false);

        /**
         * Create a jvalue of type ujson::j_null.
         * @param nil A <code>nullptr</code>.
         *            Always use <code>nullptr</code> as parameter
         *            to create a JSON null type, and not a pointer
         *            with value 0.
         * \par Exampe:
         * \code
         * ...
         * char* ptr = nullptr;
         * ujson::jvalue value1 (ptr); // Creates an empty JSON string (ujson::j_string)
         * ...
         * ujson::jvalue value2 (nullptr); // Creates a JSON null value (ujson::j_null)
         * ...
         * \endcode
         */
        jvalue (const std::nullptr_t nil);

        /**
         * Destructor.
         * Free internal resources.
         * Any references returned by this objects
         * methods will no longer be valid and should
         * not be used any more.
         */
        ~jvalue ();

        /**
         * Assignment operator.
         * Copy another jvalue object.
         * @param value The jvalue to copy.
         */
        jvalue& operator= (const jvalue& value);

        /**
         * Move operator.
         * Move the contents of another jvalue to this object.
         * @param value The jvalue to move.
         */
        jvalue& operator= (jvalue&& value);

        /**
         * Assignment operator.
         * Set the value type of this instance to ujson::j_object and
         * copy the contents of the supplied json_object to this object.
         * @param o A json_object to be copied.
         */
        jvalue& operator= (const json_object& o);

        /**
         * Move operator.
         * Set the value type of this instance to ujson::j_object and
         * move the content of the supplied json_object to this object.
         * @param o A json_object to be moved.
         */
        jvalue& operator= (json_object&& o);

        /**
         * Assignment operator.
         * Set the value type of this instance to ujson::j_array and
         * copy the content of the supplied json_array to this object.
         * @param array A json_array to be copied.
         */
        jvalue& operator= (const json_array& array);

        /**
         * Move operator.
         * Set the value type of this instance to ujson::j_array and
         * move the content of the supplied json_array to this object.
         * @param array A json_array to be moved.
         */
        jvalue& operator= (json_array&& array);

        /**
         * Assignment operator.
         * Make this a value of type ujson::j_string.
         * @note The string is copied <em>as is</em>,
         *      no assumptions are made about json escape
         *      sequences in the string. It is up to the
         *      caller to make sure that the string is a
         *      valid un-escaped UTF-8 string.
         * @param s The new string value to copy.
         * @see ujson::unescape
         */
        jvalue& operator= (const std::string& s) {str(s); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type ujson::j_string.
         * @note The string is copied <em>as is</em>,
         *      no assumptions are made about json escape
         *      sequences in the string. It is up to the
         *      caller to make sure that the string is a
         *      valid un-escaped UTF-8 string.
         * @param s The new string value to copy.
         * @see ujson::unescape
         */
        jvalue& operator= (const char* s) {str((s==nullptr?"":s)); return *this;}

        /**
         * Move operator.
         * Make this a value of type j_string.
         * @note The string is moved <em>as is</em>,
         *       no assumptions are made about json escape
         *       sequences in the string. It is up to the
         *       caller to make sure that the string is a
         *       valid un-escaped UTF-8 string.
         * @param s The string to move to this object.
         * @see ujson::unescape
         */
        jvalue& operator= (std::string&& s) {str(std::move(s)); return *this;}

#if UJSON_HAVE_GMPXX
        /**
         * Assignment operator.
         * Make this a value of type ujson::j_number.
         * @param number The number value to copy.
         * @see <a href=https://gmplib.org/manual/index
         *       rel="noopener noreferrer" target="_blank">
         *       GNU MP - C++ Class Interface</a>
         */
        jvalue& operator= (const mpf_class& number) {num(number); return *this;}

        /**
         * Move operator.
         * Make this a value of type ujson::j_number.
         * @param number The number value to move to this object.
         * @see <a href=https://gmplib.org/manual/index
         *       rel="noopener noreferrer" target="_blank">
         *       GNU MP - C++ Class Interface</a>
         */
        jvalue& operator= (mpf_class&& number) {num(std::forward<mpf_class&&>(number)); return *this;}
#endif
        /**
         * Assignment operator.
         * Make this a value of type ujson::j_number.
         * @param number The new number value.
         */
        jvalue& operator= (const double number) {num(number); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type ujson::j_number.
         * @param number The new number value.
         */
        jvalue& operator= (const int number) {num(static_cast<const long>(number)); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type ujson::j_number.
         * @param number The new number value.
         */
        jvalue& operator= (const long number) {num(number); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type ujson::j_bool.
         * @param true_false The new boolean value.
         */
        jvalue& operator= (const bool true_false) {boolean(true_false); return *this;}

        /**
         * Assignment operator.
         * Make this a value of type ujson::j_null.
         * @param nil A <code>nullptr</code>.
         *            Always use <code>nullptr</code> as parameter
         *            to create a JSON null type, and not a pointer
         *            with value 0.
         * \par Exampe:
         * \code
         * ujson::jvalue value1;
         * ujson::jvalue value2;
         * ...
         * char* ptr = nullptr;
         * value1 = ptr; // Makes value1 an empty JSON string (ujson::j_string)
         * ...
         * value2 = nullptr; // Makes value2 a JSON null value (ujson::j_null)
         * ...
         * \endcode
         */
        jvalue& operator= (const std::nullptr_t nil) { type(j_null); return *this; }

        /**
         * Less-than operator.
         * Returns <code>true</code> is this JSON value is less than
         * <code>rval</code>. Depending of the type of the two JSON values,
         * the following is tested:
         *  - <b>j_invalid</b> <code>false</code> is always returned when comparing two invalid JSON values.
         *  - <b>j_number</b> A normal less-than test of the two numbers.
         *  - <b>j_string</b> std::lexicographical_compare() is used to compare the two string values.
         *  - <b>j_bool</b> A normal less-than test of the two boolean values.
         *  - <b>j_null</b> <code>false</code> is always returned when comparing two JSON null values.
         *  - <b>j_array</b> std::lexicographical_compare() is used to compare the two arrays.
         *                   Note that a JSON array is stored as a std::vector<ujson::jvalue>
         *                   in a jvalue instance.
         *  - <b>j_object</b> std::lexicographical_compare() is used to compare the two objects.
         *                    A JSON object is stored as a ujson::multimap_list in a jvalue instance, and
         *                    when comparing two jvalues of type j_object, sorted iterators will be used.
         *                    This means that the object attributes will always be sorted by name when
         *                    compared by this method. So two JSON objects with equal attributes but
         *                    in different order will be compared as equal objects.<br/>
         *                    If comparing using natural order of the attributes is wanted, you can
         *                    manually use std::lexicographical_compare() with the iterators
         *                    jvalue::obj().begin() and jvalue::obj().end().
         * <br/>
         * <br/>
         * @note Comparing two objects representing different JSON types makes
         * no sense and this method will always return <code>true</code>
         * when they don't have the same ujson::jvalue_type. Otherwise
         * the two objects could be regarded as equal by testing
         * <code>!(a<b) && !(b<a)</code>.
         *
         * @param rval The jvalue to compare with.
         * @return <code>true</code> is this JSON value is less than
         *         <code>rval</code>.
         */
        bool operator< (const jvalue& rval) const;

        /**
         * Comparison operator.
         * @return <code>true</code> if the two jvalues are of the same
         *         JSON type and have equal values. <code>false</code> if not.
         * @note
         *      - Two JSON objects (of type ujson::j_object) are considered
         *        equal is they have the same number of attributes, with each
         *        attribute in both objects equal to an attribute in the other.
         *        The natural order of the attributes make no difference when
         *        comparing, and may differ in two objects that are still
         *        considered equal.
         *      - Comparing two invalid jvalues (both of type ujson::j_invalid)
         *        makes no sense and will always return <code>false</code>.
         */
        bool operator== (const jvalue& rval) const;

        /**
         * Comparison operator.
         * @return <code>false</code> if the two jvalues are of the same
         *         JSON type and have equal values. <code>true</code> if not.
         * @see jvalue::operator==
         */
        bool operator!= (const jvalue& rval) const { return !(operator==(rval)); }

        /**
         * Check if this a JSON object.
         * @return <code>true</code> if this value
         *         is of type ujson::j_object.
         */
        bool is_object () const {return jtype==j_object;}

        /**
         * Get a reference to the ujson::json_object instance used by
         * this jvalue instance to represent a JSON object.
         * @return A reference to a ujson::json_object.
         * @throw ujson::json_type_error If this is not a JSON object.
         *        The type of JSON value can be checked using method jvalue::type().
         * @note The jvalue instance uses an instance of a ujson::json_object
         *       to represent a JSON object. If any method is called on this
         *       jvalue instance that changes the JSON type from an object
         *       into some other JSON type, the reference returned by this
         *       method will be invalid and should not be used.
         */
        json_object& obj ();

        /**
         * Assign a json_object to this jvalue, making this a JSON object.
         * Set the value type of this instance to ujson::j_object and
         * copy the contents of the supplied json_object to this object.
         * @param o A json_object to be copied.
         */
        void obj (const json_object& o);

        /**
         * Move a json_object to this jvalue, making this a JSON object.
         * Set the value type of this instance to ujson::j_object and
         * move the content of the supplied json_object to this object.
         * @param o A json_object to be moved.
         */
        void obj (json_object&& o);

        /**
         * Check if this a JSON array.
         * @return <code>true</code> if this value
         *         is of type ujson::j_array.
         */
        bool is_array () const {return jtype==j_array;}

        /**
         * Get a reference to the ujson::json_array instance used by
         * this jvalue instance to represent a JSON array.
         * @return A reference to a ujson::json_array.
         * @throw ujson::json_type_error If this is not a JSON array.
         *        The type of JSON value can be checked using method jvalue::type().
         * @note The jvalue instance uses an instance of a ujson::json_array
         *       to represent a JSON array. If any method is called on this
         *       jvalue instance that changes the JSON type from an array
         *       into some other JSON type, the reference returned by this
         *       method will be invalid and should not be used.
         */
        json_array& array ();

        /**
         * Assign a json_array to this jvalue, making this a JSON array.
         * Set the value type of this instance to ujson::j_array and
         * copy the content of the supplied json_array to this object.
         * @param a A json_array to be copied.
         */
        void array (const json_array& a);

        /**
         * Move a json_array to this jvalue, making this a JSON array.
         * Set the value type of this instance to ujson::j_array and
         * move the content of the supplied json_array to this object.
         * @param a A json_array to be moved.
         */
        void array (json_array&& a);

        /**
         * Check if this a JSON string.
         * @return <code>true</code> if this value
         *         is of type ujson::j_string.
         */
        bool is_string () const {return jtype==j_string;}

        /**
         * Get a reference to the string value if this is a JSON string.
         * @return A reference to a std::string.
         * @throw ujson::json_type_error If this is not a JSON string.
         *        The type of JSON value can be checked using method jvalue::type().
         * @note The jvalue instance uses an instance of a std::string
         *       to represent a JSON string. If any method is called on this
         *       jvalue instance that changes the JSON type from a string
         *       into some other JSON type, the reference returned by this
         *       method will be invalid and should not be used.
         */
        std::string& str ();

        /**
         * Assing a string to this jvalue, making it a JSON string.
         * Set the value type of this instance to ujson::j_string and
         * copy the content of the supplied string to this object.
         * @note The string is copied <em>as is</em>,
         *       no assumptions are made about json escape
         *       sequences in the string. It is up to the
         *       caller to make sure that the string is a
         *       valid un-escaped UTF-8 string.
         * @param s The string value to copy.
         * @see ujson::unescape
         */
        void str (const std::string& s);

        /**
         * Move a string to this jvalue, making it a JSON string.
         * Set the value type of this instance to ujson::j_string and
         * move the content of the supplied string to this object.
         * @note The string is moved <em>as is</em>,
         *       no assumptions are made about json escape
         *       sequences in the string. It is up to the
         *       caller to make sure that the string is a
         *       valid un-escaped UTF-8 string.
         * @param s The string value to move to this object.
         * @see ujson::unescape
         */
        void str (std::string&& s);

        /**
         * Check if this a JSON number.
         * @return <code>true</code> if this value
         *         is of type ujson::j_number.
         */
        bool is_number () const {return jtype==j_number;}


#if UJSON_HAVE_GMPXX
        /**
         * Return a reference a mpf_class representing the JSON number value.
         * @return A reference to a mpf_class instance.
         * @note The jvalue instance uses an instance of a mpf_class
         *       to represent a JSON number. If any method is called on this
         *       jvalue instance that changes the JSON type from an object
         *       into some other JSON type, the reference returned by this
         *       method will be invalid and should not be used.
         * @throw ujson::json_type_error If this is not a JSON number.
         *        The type of JSON value can be checked using
         *        method jvalue::type().
         * @see <a href=https://gmplib.org/manual/index
         *       rel="noopener noreferrer" target="_blank">
         *       GNU MP - C++ Class Interface</a>
         */
        mpf_class& mpf ();
#endif
#if UJSON_HAVE_GMPXX
        /**
         * Return the JSON number value as a <code>double</code>.
         * @note The returned value will be converted to
         *       a <code>double</code> and possibly lose precision.<br/>
         *       To keep precision, use <code>jvalue::mpf()</code>.
         * @return The json number value.
         * @throw ujson::json_type_error If this is not a JSON number.
         *        The type of JSON value can be checked using
         *        method jvalue::type().
         * @see jvalue::mpf()
         */
#else
        /**
         * Return the JSON number value.
         * @return The JSON number value.
         * @throw ujson::json_type_error If this is not a JSON number.
         *        The type of JSON value can be checked using
         *        method jvalue::type().
         */
#endif
        double num () const;

#if UJSON_HAVE_GMPXX
        /**
         * Assing a number to this jvalue, making it a JSON number.
         * Set the value type of this instance to ujson::j_number and
         * copy the content of the supplied mpf_class to this object.
         * @param number The number value to copy.
         * @see <a href=https://gmplib.org/manual/index
         *       rel="noopener noreferrer" target="_blank">
         *       GNU MP - C++ Class Interface</a>
         */
        void num (const mpf_class& number);

        /**
         * Move a number to this jvalue, making it a JSON number.
         * Set the value type of this instance to ujson::j_number and
         * move the content of the supplied mpf_class to this object.
         * @param number The number value to move to this object.
         * @see <a href=https://gmplib.org/manual/index
         *       rel="noopener noreferrer" target="_blank">
         *       GNU MP - C++ Class Interface</a>
         */
        void num (mpf_class&& number);
#endif
        /**
         * Assing a number to this jvalue, making it a JSON number.
         * @param number The number value.
         */
        void num (const double number);

        /**
         * Assing a number to this jvalue, making it a JSON number.
         * @param number The number value.
         */
        void num (const long number);

        /**
         * Check if this a JSON boolean.
         * @return <code>true</code> if this value
         *         is of type ujson::j_bool.
         */
        bool is_boolean () const {return jtype==j_bool;}

        /**
         * Return the JSON boolean value.
         * @return The JSON boolean value.
         * @throw ujson::json_type_error If this is not a JSON boolean.
         *        The type of JSON value can be checked using
         *        method jvalue::type().
         */
        bool boolean () const;

        /**
         * Assing a boolean to this jvalue, making it a JSON boolean.
         * @param true_false The boolean value.
         */
        void boolean (const bool true_false);

        /**
         * Check if this a JSON null value.
         * @return <code>true</code> if this value
         *         is of type ujson::j_bool.
         */
        bool is_null () const {return jtype==j_null;}

        /**
         * Make this a JSON null value.
         */
        void set_null ();

        /**
         * Check if this is an invalid JSON instance.
         * @returns <code>true</code> if the jvalue_type of this
         *          instance is ujson::j_invalid.
         */
        bool invalid () const {return jtype == j_invalid;}

        /**
         * Check if this is a valid JSON instance.
         * @returns <code>true</code> if the jvalue_type of this
         *          instance is <em>not</em> ujson::j_invalid.
         */
        bool valid () const {return jtype != j_invalid;}

        /**
         * Return the type of JSON value this object represent.
         * @return A jvalue_type.
         * @see ujson::jvalue_type
         */
        jvalue_type type () const {return jtype;}

        /**
         * Set the type of JSON value this instance represents.
         * If the current jvalue_type is the same as requested,
         * nothing is done.
         * If not, the jvalue_type of this instance is changed to
         * the requested type and initialized to its default value.
         * @param t The new jvalue_type of this instance.
         * @see ujson::jvalue_type
         */
        void type (const jvalue_type t);

        /**
         * Check if this is a container.
         * It is a container if it is a JSON array or JSON object.
         * @return <code>true</code> if this is a JSON object or an array.
         */
        bool is_container () const;

        /**
         * Check if this is a JSON object and contains a
         * valid JSON value associated with a given name.
         * @param name The name of the attribute to look for.
         * @return <code>true</code> if this is a JSON object
         *         and it has a valid JSON value associated with
         *         the given name. Otherwise <code>false</code>.
         */
        bool has (const std::string& name) const;

        /**
         * Get a JSON object attribute.
         * Return a reference to the JSON value associated with
         * a specified name in the JSON object.<br/>
         * If no value associated with the specified name is found,
         * a reference to an invalid static jvalue is returned.
         * This invalid jvalue has jvalue_type ujson::j_invalid,
         * and will be resetted by libujson at any time, so don't
         * use it for anything else but checking the return value.
         * @param name The name of the object attribute we want.
         * @return A reference to the value mapped to the given name.
         *         Or a reference to a static invalid jvalue if not found.
         * @throw ujson::json_type_error If this is not a JSON object.
         *        The type of JSON value can be checked using method jvalue::type().
         * @note The JSON specification allows multiple object attributes
         *       with the same name. If a JSON object contains more than
         *       one attribute with the name searched for, the last value
         *       in the JSON object with that name will be returned.
         */
        jvalue& get (const std::string& name);

        /**
         * Get a JSON object attribute and assume there is only
         * one attribute with the specified name.
         * The JSON specification allows multiple object attributes
         * with the same name. This method assumes there are no
         * multiples of an attribute and will throw an exception
         * if there are.<br/>

         * This method will return a reference to the JSON value
         * associated with a specified name in the JSON object.<br/>
         * If no value associated with the specified name is found,
         * a reference to an invalid static jvalue is returned.
         * This invalid jvalue has jvalue_type ujson::j_invalid,
         * and will be resetted by libujson at any time, so don't
         * use it for anything else but checking the return value.
         * @param name The name of the object attribute we want.
         * @return A reference to the value mapped to the given name.
         *         Or a reference to a static invalid jvalue if not found.
         * @throw ujson::json_type_error
         *         - If this is not a JSON object.
         * @throw std::logic_error
         *         - If there are multiple attributes with the same name.
         */
        jvalue& get_unique (const std::string& name);

        /**
         * Access a JSON object attribute.
         * The named object attribute will be created if not
         * found in the object. The newly created jvalue will
         * be of type <code>ujson::j_null</code>.
         * @param name A name of a JSON object attribute.
         * @return A reference to a jvalue mapped to the given name.
         * @throw ujson::json_type_error If this is not a JSON object.
         *        The type of JSON value can be checked using method jvalue::type().
         * @note The JSON specification allows multiple object attributes
         *       with the same name. If a JSON object contains more than
         *       one attribute with the name searched for, the last value
         *       with that name will be returned.
         */
        jvalue& operator[] (const std::string& name);

        /**
         * Access a JSON array entry.
         * Returns a reference to the n'th value in the JSON array.
         * @param n The index of the object in the array we want to access.
         * @return A reference to a jvalue in the JSON array.
         * @throw ujson::json_type_error If this is not a JSON array.
         *        The type of JSON value can be checked using method jvalue::type().
         * @throw std::out_of_range If the index is out of range.
         */
        jvalue& operator[] (const size_t n);

        /**
         * Return the number of JSON values in an array or an object.
         * @note It is possible, but not recommended, to add invalid
         *       JSON values (of type ujson::j_invalid) to a JSON array
         *       or object if directly modifying the ujson::json_array
         *       or ujson::json_object instance representing the JSON
         *       array or object.<br/>
         *       This method will include any invalid jvalue in the
         *       returned size. It simply calls the size() method of the
         *       underlaying ujson::json_array or ujson::json_object.
         * @return If this is a JSON array, returns the number of
         *         items in the array.<br/>
         *         It this is a JSON object, returns the number of
         *         attributes in the object.<br/>
         * @throw ujson::json_type_error If this is not a JSON object or array.
         * @see jvalue::is_container()
         * @see jvalue::type()
         */
        size_t size () const;

        /**
         * Add an attribute to a JSON object.
         * @param name The name of the object attribute.
         * @param value The value of the attribute.
         * @param overwrite If <code>true</code> and an attribute
         *                  with the specified name already
         *                  exists, it will be overwritten
         *                  with the new value.<br/>
         *                  If <code>false</code> and an attribute
         *                  with the specified name already
         *                  exists, it will be untouched and
         *                  keep its current value.
         * @return A reference to the attribute value in the JSON
         *         object that was created, modified, or untouched.
         * @throw ujson::json_type_error If this is not a JSON object
         *        (not of type ujson::j_object).
         * @throw std::invalid_argument If parameter <code>value</code>
         *        is an invalid JSON value (of type ujson::j_invalid).
         */
        jvalue& add (const std::string& name, const jvalue& value,
                     const bool overwrite=true);

        /**
         * Move an attribute to a JSON object.
         * @param name The name of the object attribute.
         * @param value The jvalue to move to this object.
         * @param overwrite If <code>true</code> and an attribute
         *                  with the specified name already
         *                  exists, it will be overwritten
         *                  with the new value.<br/>
         *                  If <code>false</code> and an attribute
         *                  with the specified name already
         *                  exists, it will be untouched and
         *                  keep its current value.
         * @return A reference to the attribute value in the JSON
         *         object that was created, modified, or untouched.
         * @throw ujson::json_type_error If this is not a JSON object
         *        (not of type ujson::j_object).
         * @throw std::invalid_argument If parameter <code>value</code>
         *        is an invalid JSON value (of type ujson::j_invalid).
         */
        jvalue& add (const std::string& name, jvalue&& value,
                     const bool overwrite=true);

        /**
         * Append a value to a JSON array.
         * The size of the JSON array will be increased by one and
         * the supplied value will be copied to the end of the array.
         * @param value The value to append.
         * @return A reference to the jvalue appeded to the array.
         * @throw ujson::json_type_error If this is not a JSON array
         *        (not of type ujson::j_array).
         * @throw std::invalid_argument If parameter <code>value</code>
         *        is an invalid JSON value (of type ujson::j_invalid).
         */
        jvalue& append (const jvalue& value);

        /**
         * Append a value to a JSON array.
         * The size of the JSON array will be increased by one and
         * the supplied value will be moved to the end of the array.
         * @param value The jvalue to move.
         * @return A reference to the jvalue appeded to the array.
         * @throw ujson::json_type_error If this is not a JSON array
         *        (not of type ujson::j_array).
         * @throw std::invalid_argument If parameter <code>value</code>
         *        is an invalid JSON value (of type ujson::j_invalid).
         */
        jvalue& append (jvalue&& value);

        /**
         * Remove an attribute from a JSON object.
         * All attributes with the specified name are erased
         * from the JSON object.<br/>
         * If this isn't a JSON object or the attribute with
         * the specified name doesn't exist, <code>false</code>
         * will be returned.
         * @param name The name of the object attribute to remove.
         * @return <code>true</code> if the object had one or
         *         more attributes with the specified name.<br/>
         *         <code>false</code> if this is not a JSON object,
         *         or it doesn't have an attribute with the
         *         specified name.
         */
        bool remove (const std::string& name);

        /**
         * Remove the n'th value from a JSON array.
         * The n'th value in the JSON array is erased and
         * the size of the array is decreased by one.<br/>
         * If this isn't a JSON array or the index is out
         * of bounds, <code>false</code> will be returned.
         * @param n The index in the JSON array to be erased.
         * @return <code>true</code> if the n'th item in
         *         the array was removed.<br/>
         *         <code>false</code> if this is not a JSON array,
         *         or the index is out of bounds.
         */
        bool remove (const size_t n);

        /**
         * Return a string representation of this JSON value.
         * @param fmt Flags describing the format of the resulting
         *            output string.
         * @return A string in JSON format defining this JSON intance.
         * @note If flag <code>fmt_color</code> is set, the resulting
         *       string is <b>NOT</b> a valid JSON instance, since
         *       is contains escape codes for colors.
         * @see desc_format_t
         */
        inline std::string describe (desc_format_t fmt=fmt_none) const {
            return describe (fmt, 0);
        }

        /**
         * Return a string representation of this JSON value.
         * @param fmt Flags describing the format of the resulting
         *            output string.
         * @param starting_indent_depth Only relevant if flag
         *                              <code></code> is set.
         *                              Start the output with
         *                              this indentation level.
         * @return A string in JSON format defining this JSON intance.
         * @note If flag <code>fmt_color</code> is set, the resulting
         *       string is <b>NOT</b> a valid JSON instance, since
         *       is contains escape codes for colors.
         * @see desc_format_t
         */
        std::string describe (desc_format_t fmt,
                              unsigned starting_indent_depth) const;

        /**
         * Return a string representation of this JSON value.
         * All string output(object member names and string
         * values) will be json encoded using ujson::escape.
         * Any invalid JSON value (of type ujson::j_invalid)
         * will be omitted from the string representation.
         * @param pretty If <code>true</code>, whitespaces
         *               (line brake and indentation) are used
         *               to make the description more readable.<br/>
         *               If <code>false</code>, return a compact
         *               representation without unnecessary whitespace.
         * @param array_items_on_same_line If <code>true</code>, array
         *               items are printed on one line (any object
         *               in the array will still be printed with each
         *               attribute indented on a separate line).<br/>
         *               If <code>false</code>, every array item is
         *               printed on a separate line.<br/>
         *               This parameter is only relevant is parameter
         *               <code>pretty</code> is <code>true</code>.
         * @param sorted_properties If <code>true</code>, the
         *                          properties of JSON objects will be
         *                          printed sorted by name, and not
         *                          in natural order (the order they
         *                          were inserted).
         * @param escape_slash If <code>true</code>, the
         *                     forward slash character "/" will
         *                     be esacped to "\/".
         * @param relaxed_mode If this parameter is <code>true</code>,
         *               print any member name without enclosing double
         *               quotes if the object member name is in the
         *               following format: [_a-zA-Z][_a-zA-Z0-9]*
         * @param indent Indentation to use if <code>pretty</code>
         *               is <code>true</code>. Normally zero or more
         *               space characters.<br/>
         *               If anything other than whitespace, the returned
         *               string will fail to be parsed as a valid JSON instance.<br/>
         *               <b>NOTE!</b> This parameter is deprecated and no longer used.
         * @return A string in JSON format defining this JSON intance.
         * @deprecated Use <code>describe(desc_format_t fmt)</code> instead.
         */
        [[deprecated("Use jvalue::describe(desc_format_t fmt) instead.")]]
        std::string describe (bool pretty,//=false,
                              bool array_items_on_same_line=true,
                              bool sorted_properties=false,
                              bool escape_slash=false,
                              bool relaxed_mode=false,
                              const std::string& indent="    ") const;


    private:
        // The internal representation of a number
#if UJSON_HAVE_GMPXX
        using num_t = mpf_class; // A number is represented by an instance of mpf_class
#else
        using num_t = double; // A number is represented by a double
#endif
        friend class Analyzer; // Using type jvalue::num_t
        friend class Parser;   // Using type jvalue::num_t

        jvalue_type jtype;
        union {
            json_object* jobj;
            json_array*  jarray;
            std::string* jstr;
#if UJSON_HAVE_GMPXX
            num_t*       jnum; // mpf_class*
#else
            num_t        jnum; // double
#endif
            bool         jbool;
        } v;

        void reset ();
        void copy (const jvalue& jvalue);
        void move (jvalue&& jvalue);

        void describe (std::stringstream& ss,
                       desc_format_t fmt,
                       unsigned indent_depth) const;
        void describe_object (std::stringstream& ss,
                              desc_format_t fmt,
                              unsigned indent_depth) const;
        void describe_array (std::stringstream& ss,
                             desc_format_t fmt,
                             unsigned indent_depth) const;
    };


}
#endif
