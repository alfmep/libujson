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

#include <string>


namespace ujson {

    /**
     * Convert a string to a json escaped string.
     * Backslash \ is translated to: \\\\<br>
     * Quote " is translated to: \"<br>
     * Slash / is translated to: \\/<br>
     * Tab is translated to: \\t<br>
     * Backspace is translated to: \\b<br>
     * Formfeed is translated to: \\f<br>
     * Newline is translated to: \\n<br>
     * Return is translated to: \\r<br>
     *
     * @param in An unescaped string.
     * @return An escaped string.
     */
    std::string escape (const std::string& in);

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
