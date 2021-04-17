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
     * Get the name of a json type.
     * @return The name of a specific <code>jvalue_type</code>.
     * @see str_to_jtype
     */
    std::string jtype_to_str (const jvalue_type jtype);

    /**
     * Get the name of the type of JSON instance.
     * @return The name of a specific <code>jvalue_type</code>.
     * @see str_to_jtype
     * \par Exampe:
     * The following code:
     * \code
     * ujson::json_array instance = {{
     *         42,
     *         "A JSON string",
     *         {{{"a",1},{"b",2}}},
     *         ujson::jvalue(false),
     *         ujson::jvalue(nullptr),
     *         ujson::json_array({{0,1,2,3}}),
     *     }};
     * for (size_t i=0; i<instance.size(); ++i)
     *     std::cout << "instance[" << i << "] is of type: " << ujson::jtype_to_str(instance[i]) << std::endl;
     * \endcode
     * Gives the following output:
     * \code
     * instance[0] is of type: number
     * instance[1] is of type: string
     * instance[2] is of type: object
     * instance[3] is of type: boolean
     * instance[4] is of type: null
     * instance[5] is of type: array
     * \endcode
     */
    std::string jtype_to_str (const jvalue& instance);

    /**
     * Get a <code>jvalue_type</code> from a string.
     * Return a <code>jvalue_type</code> depending on the name.<br/>
     * Valid names are:
     *  - number
     *  - string
     *  - boolean
     *  - array
     *  - object
     *  - null
     *
     * Any other name will result in <code>j_invalid</code>.
     * @param name The name of the <code>jvalue_type</code>.
     * @return A <code>jvalue_type</code> depending on the name.
     */
    jvalue_type str_to_jtype (const std::string& name);

    /**
     * Get a reference to a specific value in a json instance.
     * @param instance A json instance.
     * @param location The location of the value in the instance.
     *                 The location string is assumed to be json
     *                 escaped where needed.<br/>
     *                 A <b>location</b> is specified by:
     *                  - An single dot "." returns a reference to
     *                    the whole JSON instance.
     *                  - A name is used to access an object property
     *                    with the that name.
     *                  - To accesses nested object properties,
     *                    separate the property names with a dot "."
     *                  - For arrays, use [n] to access the n'th array item.
     *                    (Example: <code>"root_obj.some_array[4]"</code>).
     *                  - To get the whole array, omit the [n].
     *                    (Example: <code>"root_obj.some_array"</code>).
     *                  - Object member names are assumed to be JSON
     *                    escaped where needed.<br/>
     *                  - Since dots "." and left brackets "[" are used
     *                    to find specific object members and/or array items,
     *                    they can't be used directly in member names in
     *                    a location. To access object members with
     *                    a "." or "[" in the name, escape the dot
     *                    as <code>\\u002e</code> and left bracket
     *                    as <code>\\u005b</code>.
     * @return A reference to a jvalue in the instance.<br/>
     *         If the location can't be found, a reference
     *         to a static invalid jvalue is returned
     *         (a jvalue of type ujson::j_invalid). Do not
     *         modify this value since it will be reset to
     *         an invalid state by the library.
     *
     * \par Exampe:
     * Given an instance of <code>ujson::jvalue</code> named
     * <i>instance</i> containing the following:
     * \code
     * {
     *   "name": "Foo",
     *   "value": 42,
     *   "list": ["zero", "one", "two"],
     *   "subobj": {
     *     "value": 128,
     *     "items": [{"a":false}, {"a":true}],
     *     ".hidden": "secret"
     *   },
     * }
     * \endcode
     * The code:
     * \code
     * auto& jv1 = ujson::find_jvalue(instance, "value");
     * auto& jv2 = ujson::find_jvalue(instance, "list");
     * auto& jv3 = ujson::find_jvalue(instance, "list[1]");
     * auto& jv4 = ujson::find_jvalue(instance, "subobj.non.existent");
     * auto& jv5 = ujson::find_jvalue(instance, "subobj.items[1].a");
     * auto& jv6 = ujson::find_jvalue(instance, "subobj.\\u002ehidden");
     * std::cout << "jv1 (" << ujson::jtype_to_str(jv1) << "):\t " << jv1.describe() << std::endl;
     * std::cout << "jv2 (" << ujson::jtype_to_str(jv2) << "):\t " << jv2.describe() << std::endl;
     * std::cout << "jv3 (" << ujson::jtype_to_str(jv3) << "):\t " << jv3.describe() << std::endl;
     * std::cout << "jv4 (" << ujson::jtype_to_str(jv4) << "):\t " << jv4.describe() << std::endl;
     * std::cout << "jv5 (" << ujson::jtype_to_str(jv5) << "):\t " << jv5.describe() << std::endl;
     * std::cout << "jv6 (" << ujson::jtype_to_str(jv6) << "):\t " << jv6.describe() << std::endl;
     * \endcode
     * Will give the following output:<br/>
     * \code
     * jv1 (number):	 42
     * jv2 (array):	 ["zero","one","two"]
     * jv3 (string):	 "one"
     * jv4 (invalid):	 
     * jv5 (boolean):	 true
     * jv6 (string):	 "secret"
     * \endcode
     */
    jvalue& find_jvalue (jvalue& instance, const std::string& location);

    /**
     * Convert a string to a json escaped string.
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
     * Convert a json escaped string to an unescaped string.
     * @param in An escaped string.
     * @param ok This will be set to <code>false</code>
     *           if the input string is formatted in an
     *           uncorrect way. Otherwise it is set to
     *           <code>true</code>.
     * @return An unescaped string. All escape sequences that
     *         are found incorrect will be excluded from the
     *         result and <code>ok</code> will be set to
     *         <code>false</code> if found.
     */
    std::string unescape (const std::string& in, bool& ok);

    /**
     * Convert a json escaped string to an unescaped string, ignoring errors.
     * @param in An escaped string.
     * @return An unescaped string. All escape sequences that
     *         are found incorrect will be excluded from the result.
     */
    static inline std::string unescape (const std::string& in) {
        bool ignore_ok_param;
        return unescape (in, ignore_ok_param);
    }

}

#endif
