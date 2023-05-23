/*
 * Copyright (C) 2023 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef OPTION_PARSER_HPP
#define OPTION_PARSER_HPP

#include <string>
#include <vector>


struct opt_t {
    char short_opt;
    std::string long_opt;
    enum {
        none = 0, /**< The option has no argument. */
        optional, /**< The option has an optional argument. */
        required  /**< The option requires an argument. */
    } arg;
    int id;       /**< An option id. If 0, the value of 'short_opt' is used. */
};
using optlist_t = std::vector<opt_t>;



class option_parser {
public:
    option_parser (int argc_arg, char* argv_arg[]);

    /**
     * Parse next command line option/argument.
     *
     * @param option_descriptor A list describing the options.
     * @return  >0 An option was found. The ID of the option is returned.
     *           0 No more options/arguments found.
     *          -1 If the next found option isn't in the option descriptor.
     *          -2 If the next found option is missing a required option argument.
     */
    int operator() (const optlist_t& option_descriptor); /**< Parse next option. */

    std::string optarg ();    /**< Current option argument. */
    char        short_opt (); /**< Current short option, or 0 if none. */
    std::string long_opt ();  /**< Current long option, or empty string if none. */
    std::string opt ();       /**< Current long option if possible, or short option, or empty string if no option. */

    std::vector<std::string>& arguments (); /**< Return arguments that aren't options. */


private:
    int argc;
    char** argv;
    int argc_index;
    char* short_opt_ptr;
    std::string opt_arg;
    std::string current_long_opt;
    char current_short_opt;
    std::vector<std::string> args;

    int handle_short_opt (const optlist_t& option_descriptor);
    int handle_long_opt (const optlist_t& option_descriptor);
};


#endif
