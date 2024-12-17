/*
 * Copyright (C) 2021-2024 Dan Arrhenius <dan@ultramarin.se>
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
#include <ujson.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unistd.h>
#include "option-parser.hpp"



using namespace std;

static constexpr const char* prog_name = "ujson-get";

struct appargs_t {
    string filename;
    ujson::jpointer pointer;
    ujson::jvalue_type jtype;
    ujson::desc_format_t fmt;
    bool strict;
    bool allow_duplicates;
    bool unescape;

    appargs_t () {
        jtype = ujson::j_invalid;
        fmt = ujson::fmt_pretty;
        strict = false;
        allow_duplicates = true;
        unescape = false;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl;
    out << "Print a value from a JSON document." << endl;
    out << endl;
    out << "Usage: " << prog_name << " [OPTIONS] [FILE] POINTER" << endl;
    out << endl;
    out << "A POINTER is a JSON pointer as described in RFC 6901." << endl;
    out << "If the value pointed to is not found in the JSON document," << endl;
    out << "or the pointer is not a valid JSON pointer, or on a parse error," << endl;
    out << prog_name << " exits with code 1." << endl;
    out << endl;
    out << "In no file name is given, a JSON document is read from standard input." << endl;
    out << endl;
    out << "Options:" <<endl;
    out << "  -c, --compact        If the JSON value is an object or an array, print it without whitespace." << endl;
    out << "  -t, --type=TYPE      Require the value to be of a specific type." << endl;
    out << "                       TYPE is one of the following: boolean, number, string, null, object, or array." << endl;
    out << "                       If the value is of a different type, exit with code 1." << endl;
    out << "  -u, --unescape       If the resulting value is a JSON string," << endl;
    out << "                       print it as an unescaped string witout enclosing double quotes." << endl;
    out << "  -s, --strict         Parse the JSON document in strict mode." << endl;
    out << "  -n, --no-duplicates  Don't allow objects with duplicate member names." << endl;
#if (UJSON_HAS_CONSOLE_COLOR)
    out << "  -o, --color          Print in color if the output is to a tty." << endl;
#endif
    out << "  -v, --version        Print version and exit." << endl;
    out << "  -h, --help           Print this help message and exit." << endl;
    out << endl;
    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    optlist_t options = {
        {'c', "compact",       opt_t::none,     0},
        {'t', "type",          opt_t::required, 0},
        {'u', "unescape",      opt_t::none,     0},
        {'r', "relaxed",       opt_t::none,     0},
        {'s', "strict",        opt_t::none,     0},
        {'n', "no-duplicates", opt_t::none,     0},
        {'o', "color",         opt_t::none,     0},
        {'v', "version",       opt_t::none,     0},
        {'h', "help",          opt_t::none,     0},
    };

    option_parser opt (argc, argv);
    while (int id=opt(options)) {
        switch (id) {
        case 'c':
            args.fmt = ujson::fmt_none;
            break;
        case 't':
            args.jtype = ujson::str_to_jtype (opt.optarg());
            if (args.jtype == ujson::j_invalid) {
                cerr << "Invalid json type: " << opt.optarg() << endl;
                exit (1);
            }
            break;
        case 'u':
            args.unescape = true;
            break;
        case 'r':
            args.strict = false;
            break;
        case 's':
            args.strict = true;
            break;
        case 'n':
            args.allow_duplicates = false;
            break;
        case 'o':
#if (UJSON_HAS_CONSOLE_COLOR)
            if (isatty(fileno(stdout)))
                args.fmt |= ujson::fmt_color;
#endif
            break;
        case 'v':
            std::cout << prog_name << ' ' << UJSON_VERSION_STRING << std::endl;
            exit (0);
            break;
        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;
        case -1:
            cerr << "Unknown option: '" << opt.opt() << "'" << endl;
            exit (1);
            break;
        case -2:
            cerr << "Missing argument to option '" << opt.opt() << "'" << endl;
            exit (1);
            break;
        }
    }

    auto& arguments = opt.arguments ();
    switch (arguments.size()) {
    case 0:
        cerr << "Too few arguments" << endl;
        exit (1);
        break;

    case 1:
        args.pointer = arguments[0];
        break;

    case 2:
        try {
            args.filename = arguments[0];
            args.pointer  = arguments[1];
        }
        catch (std::invalid_argument& ia) {
            cerr << "Error: " << ia.what() << endl;
            exit (1);
        }
        break;

    default:
        cerr << "Too many arguments" << endl;
        exit (1);
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t opt;
    int retval = 0;

    try {
        parse_args (argc, argv, opt);

        // Read json file or standard input
        //
        ifstream ifs;
        ifs.exceptions (std::ifstream::failbit);
        if (!opt.filename.empty())
            ifs.open (opt.filename);
        istream& in = opt.filename.empty() ? cin : ifs;
        string json_desc ((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

        // Parse json document
        //
        ujson::jparser parser;
        auto instance = parser.parse_string (json_desc, opt.strict, opt.allow_duplicates);
        if (!instance.valid()) {
            cerr << "Parse error: " << parser.error() << endl;
            exit (1);
        }

        // Get the value
        //
        auto& value = ujson::find_jvalue (instance, opt.pointer);
        bool type_mismatch = false;

        // Print the result
        //
        if (opt.jtype != ujson::j_invalid  &&  value.type() != opt.jtype) {
            // The value is not of the type we required
            type_mismatch = true;
            std::cerr << "Type mismatch, value at \"" << (std::string)opt.pointer
                      << "\" is of type \"" << jtype_to_str(value.type())
                      << "\"" << std::endl;
            value.type (ujson::j_invalid);
        }

        if (value.valid()) {
            if (opt.unescape && value.is_string())
                cout << value.str() << endl;
            else
                cout << value.describe(opt.fmt) << endl;
        }else{
            retval = 1;
        }

        if (retval && !type_mismatch)
            std::cerr << "Value at location \"" << (std::string)opt.pointer << "\" not found" << endl;
    }
    catch (std::ios_base::failure& io_error) {
        if (opt.filename.empty())
            cerr << "Error reading input: " << io_error.code().message() << endl;
        else
            cerr << "Error reading file '" << opt.filename << "': " << io_error.code().message() << endl;
        retval = 1;
    }
    catch (std::exception& e) {
        cerr << "Error: " << e.what() << endl;
        retval = 1;
    }

    return retval;
}
