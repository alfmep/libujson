/*
 * Copyright (C) 2017,2020,2021 Dan Arrhenius <dan@ultramarin.se>
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
#include <string>


namespace ujson {


    /**
     * Get the name of a JSON type.
     * @return The name of a specific ujson::jvalue_type.
     * @see str_to_jtype
     */
    std::string jtype_to_str (const jvalue_type jtype);

    /**
     * Get the name of the type of JSON instance.
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
     * Any other name will result in ujson::j_invalid.
     * @param name The name of the <code>jvalue_type</code>.
     * @return A ujson::jvalue_type depending on the name.
     * @see ujson::jvalue_type
     */
    jvalue_type str_to_jtype (const std::string& name);

    /**
     * Get a reference to a specific value in a JSON instance.
     * @param instance A JSON instance.
     * @param pointer A JSON pointer.<br/>
     *                JSON pointers are described in RFC 6901.
     * @return A reference to a jvalue in the instance.<br/>
     *         If the value can't be found, a reference
     *         to a static invalid jvalue is returned
     *         (a jvalue of type ujson::j_invalid). Do not
     *         modify this value since it will be reset to
     *         an invalid state at any time by the library.
     * @see <a href=https://datatracker.ietf.org/doc/html/rfc6901 rel="noopener noreferrer" target="_blank">RFC 6901 - JavaScript Object Notation (JSON) Pointer</a>
     * \par Exampe:
     * \code
     * auto& item = ujson::find_jvalue (instance, "/pointer/to/value");
     * if (item.valid())
     *     std::cout << item.describe();
     * else
     *     std::cout << "Item not found" << std::endl;
     * \endcode
     */
    jvalue& find_jvalue (jvalue& instance, const std::string& pointer);

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
    static inline std::string unescape (const std::string& in) {
        bool ignore_ok_param;
        return unescape (in, ignore_ok_param);
    }

    /**
     * Patch a JSON instance.
     * @param instance The JSON instance to patch.
     * @param patch A patch operation, or an array of patch operations.
     * @return The number of successful patch operations.
     *         On error, <code>errno</code> is set:
     *          - EINVAL is set if a patch is not a valid patch, or the instance isn't a valid JSON value.<br/>
     *          - ENOENT is set if one or more object paths in the patch is invalid.<br/>
     * @see <a href=https://datatracker.ietf.org/doc/html/rfc6902 rel="noopener noreferrer" target="_blank">RFC 6902 - JavaScript Object Notation (JSON) Patch</a>
     */
    int patch (jvalue& instance, jvalue& patch);

}

#endif
