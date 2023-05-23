/*
 * Copyright (C) 2017,2020-2023 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_UTILS_HPP
#define UJSON_UTILS_HPP

#include <ujson/jvalue.hpp>
#include <ujson/jpointer.hpp>
#include <string>


namespace ujson {


    /**
     * The result of an individual JSON patch operation.
     * @see ujson::patch
     * @see <a href=https://datatracker.ietf.org/doc/html/rfc6902 rel="noopener noreferrer" target="_blank">RFC 6902 - JavaScript Object Notation (JSON) Patch</a>
     */
    enum jpatch_result {
        patch_ok = 0,  /**< The patch operation was successful. */
        patch_fail,    /**< The patch was a test operation that failed. */
        patch_noent,   /**< A pointer in the patch doesn't point to a value in the instance to patch. */
        patch_invalid  /**< The patch was not a valid patch definition. */
    };


    /**
     * Convert an UTF-16 string to an UTF-8 string.
     * @param u16 An UTF-16 encoded string.
     * @return An UTF-8 encoded string.
     * @throws std::invalid_argument If the input string contains
     *                               invalid UTF-16 characters.
     */
    std::string utf16_to_utf8 (const std::u16string u16);

    /**
     * Get the name of a JSON type.
     * The following strings are returned for each jvalue_type:<br/>
     * - ujson::j_invalid returns the string "invalid".
     * - ujson::j_object returns the string "object".
     * - ujson::j_array returns the string "array".
     * - ujson::j_string returns the string "string".
     * - ujson::j_number returns the string "number".
     * - ujson::j_bool returns the string "boolean".
     * - ujson::j_null returns the string "null".
     *
     * @return The name of a specific ujson::jvalue_type.
     * @see str_to_jtype
     */
    std::string jtype_to_str (const jvalue_type jtype);

    /**
     * Get the name of the JSON type an ujson::jvalue instance represents.
     * The following strings are returned for each jvalue_type:<br/>
     * - ujson::j_invalid returns the string "invalid".
     * - ujson::j_object returns the string "object".
     * - ujson::j_array returns the string "array".
     * - ujson::j_string returns the string "string".
     * - ujson::j_number returns the string "number".
     * - ujson::j_bool returns the string "boolean".
     * - ujson::j_null returns the string "null".
     *
     * @return The name of a specific ujson::jvalue_type.
     * @see str_to_jtype
     */
    std::string jtype_to_str (const jvalue& instance);

    /**
     * Get a ujson::jvalue_type from a string.
     * Return a ujson::jvalue_type depending on the name.<br/>
     * Valid names are:
     *  - number
     *  - string
     *  - boolean
     *  - array
     *  - object
     *  - null
     *
     * Any other name will return ujson::j_invalid.
     * @param name The string representation of a <code>jvalue_type</code>.
     * @return A ujson::jvalue_type depending on the name.
     * @see ujson::jvalue_type
     */
    jvalue_type str_to_jtype (const std::string& name);

    /**
     * Get a reference to a specific value in a
     * JSON instance using a JSON pointer.
     * @param instance A JSON instance.
     * @param pointer A JSON pointer.<br/>
     *                JSON pointers are described in RFC 6901.
     * @return A reference to a ujson::jvalue in the JSON instance
     *         if the value was found by the JSON pointer.<br/>
     *         If the value can't be found, a reference
     *         to a static invalid jvalue is returned
     *         (a jvalue of type ujson::j_invalid). Do not
     *         modify this value since it will be reset to
     *         an invalid state at any time by libujson.
     * \par Exampe:
     * \code
     * auto& item = ujson::find_jvalue (instance, "/pointer/to/value");
     * if (item.valid())
     *     std::cout << item.describe();
     * else
     *     std::cout << "Item not found" << std::endl;
     * \endcode
     * @see <a href=https://datatracker.ietf.org/doc/html/rfc6901 rel="noopener noreferrer" target="_blank">RFC 6901 - JavaScript Object Notation (JSON) Pointer</a>
     */
    jvalue& find_jvalue (jvalue& instance, const jpointer& pointer);

    /**
     * Convert a string to a JSON escaped string.
     * Backslash(0x5c) \  is translated to: \\\\<br>
     * Double quote(0x22) " is translated to: \"<br>
     * Tab(0x09) is translated to: \\t<br>
     * Backspace(0x08) is translated to: \\b<br>
     * Formfeed(0x0c) is translated to: \\f<br>
     * Newline(0x0a) is translated to: \\n<br>
     * Return(0x0d) is translated to: \\r<br>
     *
     * @param in A string to be JSON escaped.
     * @param escape_slash This function does not
     *                     escape the forward slash '/'
     *                     character by default.
     *                     But the JSON specification
     *                     allows it to be escaped to "\/".
     *                     Set this parameter to \c true to
     *                     also esacpe '/' characters.
     * @return A JSON escaped string.
     */
    std::string escape (const std::string& in, bool escape_slash=false);

    /**
     * Convert a JSON escaped string to an unescaped string.
     * @param in An escaped string.
     * @param ok This will be set to <code>false</code>
     *           if the input string is formatted in an
     *           uncorrect way. Otherwise it is set to
     *           <code>true</code>.
     * @return An unescaped string. All escape sequences that
     *         are found incorrect will be excluded from the
     *         result and <code>ok</code> will be set to
     *         <code>false</code> if errors are found.
     */
    std::string unescape (const std::string& in, bool& ok);

    /**
     * Convert a JSON escaped string to an unescaped string, ignoring errors.
     * @param in An escaped string.
     * @return An unescaped string. All escape sequences that
     *         are found incorrect will be excluded from the result.
     */
    inline std::string unescape (const std::string& in) {
        bool ignore_ok_param;
        return unescape (in, ignore_ok_param);
    }


    /**
     * Parse a JSON pointer and return a list of reference tokens.
     */
    std::list<std::string> parse_pointer (const std::string& json_pointer);


    /**
     * Convert a string to a JSON pointer token.
     * A JSON pointer contains one or more reference tokens
     * separated by '/'. In such tokens, the characters '~'
     * and '/' are encoded as '~0' and '~1'. This function
     * converts each '~' to '~0', and each '/' to '~1'.
     * @param token The string to convert to a JSON pointer.
     * @return A string where each '~' is replaced by '~0',
     *         and each '/' is replaced by '~1'.
     */
    std::string escape_pointer_token (const std::string& token);


    /**
     * Convert a JSON ponter token to an unescaped string.
     * A JSON pointer contains one or more reference tokens
     * separated by '/'. In such tokens, the characters '~'
     * and '/' are encoded as '~0' and '~1'. This function
     * converts each '~0' to '~', and each '~1' to '/'.
     * @param token The JSON pointer token to unescape.
     * @return A string where each '~0' is replaced by '~',
     *         and each '~1' is replaced by '/'.
     */
    std::string unescape_pointer_token (const std::string& token);


    /**
     * Patch a JSON instance.
     * @param instance The original JSON instance to patch.
     * @param result_instance The resulting JSON instance after the patch.
     * @param json_patch A JSON patch definition as described in RFC 6902.
     * @return A pair where the first entry is a boolean that is
     *         <code>true</code> if <em>all</em> patches where
     *         successfully applied. And the second entry is a
     *         vector with a result for each individual patch in
     *         the patch definition.
     * @throw std::invalid_argument If any parameter is an invalid JSON value
     *                              (of type ujson::j_invalid).
     * @see <a href=https://datatracker.ietf.org/doc/html/rfc6902 rel="noopener noreferrer" target="_blank">RFC 6902 - JavaScript Object Notation (JSON) Patch</a>
     */
    std::pair<bool, std::vector<jpatch_result>> patch (jvalue& instance,
                                                       jvalue& result_instance,
                                                       jvalue& json_patch);


    /**
     * Patch a JSON instance in place.
     * @param instance A JSON instance to patch.
     * @param json_patch A JSON patch definition as described in RFC 6902.
     * @return A pair where the first entry is a boolean that is
     *         <code>true</code> if <em>all</em> patches where
     *         successfully applied. And the second entry is a
     *         vector with a result for each individual patch in
     *         the patch definition.
     * @throw std::invalid_argument If any parameter is an invalid JSON value
     *                              (of type ujson::j_invalid).
     * @see <a href=https://datatracker.ietf.org/doc/html/rfc6902 rel="noopener noreferrer" target="_blank">RFC 6902 - JavaScript Object Notation (JSON) Patch</a>
     */
    std::pair<bool, std::vector<jpatch_result>> patch (jvalue& instance,
                                                       jvalue& json_patch);


}

#endif
