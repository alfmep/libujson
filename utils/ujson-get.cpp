/*
 * Copyright (C) 2021,2022 Dan Arrhenius <dan@ultramarin.se>
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
#include <getopt.h>


using namespace std;

static constexpr const char* prog_name = "ujson-get";

struct appargs_t {
    string filename;
    ujson::jpointer pointer;
    ujson::jvalue_type jtype;
    bool compact;
    bool relaxed;
    bool unescape;

    appargs_t () {
        jtype = ujson::j_invalid;
        compact = false;
        relaxed = false;
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
    out << "Usage: " << prog_name << " [OPTIONS] [FILE] [POINTER]" << endl;
    out << endl;
    out << "A POINTER is a JSON pointer as described in RFC 6901." << endl;
    out << "If the value is not found in the JSON document, or on a parse error, "
        << prog_name << " exits with code 1." << endl;
    out << endl;
    out << "Options:" <<endl;
    out << "  -c, --compact    If the JSON value is an object or an array, print it without whitespace." << endl;
    out << "  -t, --type=TYPE  Require the value to be of a specific type." << endl;
    out << "                   TYPE is one of the following: boolean, number, string, null, object, or array." << endl;
    out << "                   If the value is of a different type, exit with code 1." << endl;
    out << "  -u, --unescape   If the resulting value is a JSON string," << endl;
    out << "                   print it as an unescaped string witout enclosing double quotes." << endl;
    out << "  -r, --relaxed    Parse the JSON document in relaxed mode." << endl;
    out << "  -v, --version    Print version and exit." << endl;
    out << "  -h, --help       Print this help message and exit." << endl;
    out << endl;
    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    struct option long_options[] = {
        { "compact",  no_argument,       0, 'c'},
        { "type",     required_argument, 0, 't'},
        { "unescape", no_argument,       0, 'u'},
        { "relaxed",  no_argument,       0, 'r'},
        { "version",  no_argument,       0, 'v'},
        { "help",     no_argument,       0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "ct:urvh";

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'c':
            args.compact = true;
            break;
        case 't':
            args.jtype = ujson::str_to_jtype (optarg);
            if (args.jtype == ujson::j_invalid) {
                cerr << "Invalid json type: " << optarg << endl;
                exit (1);
            }
            break;
        case 'u':
            args.unescape = true;
            break;
        case 'r':
            args.relaxed = true;
            break;
        case 'v':
            std::cout << prog_name << ' ' << PACKAGE_VERSION << std::endl;
            exit (0);
            break;
        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;
        default:
            exit (1);
            break;
        }
    }

    // Get the argument for the JSON document
    //
    if (optind < argc) {
        args.filename = argv[optind++];
    }else{
        cerr << "Too few arguments" << endl;
        exit (1);
    }

    // Get the argument for the JSON pointer
    //
    if (optind < argc) {
        try {
            args.pointer = argv[optind++];
        }
        catch (std::invalid_argument& ia) {
            cerr << "Error: " << ia.what() << endl;
            exit (1);
        }
    }

    // Still more arguments ?
    //
    if (optind < argc) {
        cerr << "Too many arguments" << endl;
        exit (1);
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t opt;
    parse_args (argc, argv, opt);

    // Read json file or standard input
    //
    ifstream ifs;
    if (!opt.filename.empty())
        ifs.open (opt.filename);
    istream& in = opt.filename.empty() ? cin : ifs;
    string json_desc ((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

    // Parse json document
    //
    ujson::jparser parser;
    auto instance = parser.parse_string (json_desc, !opt.relaxed);
    if (!instance.valid()) {
        cerr << "Parse error: " << parser.error() << endl;
        exit (1);
    }

    // Get the value
    //
    auto& value = ujson::find_jvalue (instance, opt.pointer);
    int retval = 0;
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
        if (opt.unescape  &&  value.type() == ujson::j_string)
            cout << value.str() << endl;
        else
            cout << value.describe(!opt.compact) << endl;
    }else{
        retval = 1;
    }

    if (retval && !type_mismatch)
        std::cerr << "Value at location \"" << (std::string)opt.pointer << "\" not found" << endl;

    return retval;
}
