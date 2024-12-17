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
#include <string>
#include <cstdio>
#include <unistd.h>
#include "option-parser.hpp"
#include "parser-errors.hpp"


using namespace std;
using fmt = ujson::desc_format_t;

static constexpr const char* prog_name = "ujson-print";

struct appargs_t {
    ujson::desc_format_t fmt;
    bool parse_strict;
    bool allow_duplicates;
    bool multi_doc;
    string filename;

    appargs_t () {
        fmt = fmt::fmt_pretty;
        parse_strict = false;
        allow_duplicates = true;
        multi_doc = false;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl;
    out << "Parse a JSON document and print it to standard output." << endl;
    out << endl;
    out << "Usage: " << prog_name << " [OPTIONS] [JSON-file]" << endl;
    out << endl;
    out << "In no file name is given, a JSON document is read from standard input." << endl;
    out << "By default, the JSON document is parsed in relaxed mode." << endl;
    out << "Options:" <<endl;
    out << "  -c, --compact         Compact output, no newlines or indentation." << endl;
    out << "  -e, --escape-slash    Forward slash characters(\"/\") are escaped to \"\\/\"." << endl;
    out << "  -t, --sort            Object members are listed in sorted order, not in natural order." << endl;
    out << "  -a, --array-lines     For JSON arrays, print each array item on the same line." << endl;
    out << "  -b, --tabs            Indent using tab characters instead of spaces." << endl;
    out << "                        Ignored if option '-c,--compact' is used." << endl;
    out << "  -r, --relaxed         Print the JSON document in relaxed form." << endl;
    out << "                        Object member names are printed without enclosing double quotes" << endl;
    out << "                        when the names are in the following format: [_a-zA-Z][_a-zA-Z0-9]*" << endl;
    out << "  -s, --strict          Parse the JSON document in strict mode." << endl;
    out << "  -n, --no-duplicates   Don't allow objects with duplicate member names." << endl;
    out << "  -m, --multi-doc       Parse multiple JSON instances." << endl;
    out << "                        The input is treated as a stream of JSON " << endl;
    out << "                        instances, separated by line breaks." << endl;
#if (UJSON_HAS_CONSOLE_COLOR)
    out << "  -o, --color           Print in color if the output is to a tty." << endl;
#endif
    out << "  -v, --version         Print version and exit." << endl;
    out << "  -h, --help            Print this help message and exit." << endl;
    out << endl;
    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    optlist_t options = {
        { 'c', "compact",      opt_t::none, 0},
        { 'e', "escape-slash", opt_t::none, 0},
        { 't', "sort",         opt_t::none, 0},
        { 'a', "array-lines",  opt_t::none, 0},
        { 'b', "tabs",         opt_t::none, 0},
        { 'r', "relaxed",      opt_t::none, 0},
        { 's', "strict",       opt_t::none, 0},
        { 'n', "no-duplicates",opt_t::none, 0},
        { 'm', "multi-doc",    opt_t::none, 0},
        { 'o', "color",        opt_t::none, 0},
        { 'v', "version",      opt_t::none, 0},
        { 'h', "help",         opt_t::none, 0},
    };

    option_parser opt (argc, argv);
    while (int id=opt(options)) {
        switch (id) {
        case 'c':
            args.fmt ^= fmt::fmt_pretty;
            break;
        case 'e':
            args.fmt |= fmt::fmt_escape_slash;
            break;
        case 't':
            args.fmt |= fmt::fmt_sorted;
            break;
        case 'a':
            args.fmt |= fmt::fmt_compact_array;
            break;
        case 'b':
            args.fmt |= fmt::fmt_tabs;
            break;
        case 'r':
            args.fmt |= fmt::fmt_relaxed;
            break;
        case 's':
            args.parse_strict = true;
            break;
        case 'n':
            args.allow_duplicates = false;
            break;
        case 'm':
            args.multi_doc = true;
            break;
        case 'o':
#if (UJSON_HAS_CONSOLE_COLOR)
            if (isatty(fileno(stdout)))
                args.fmt |= fmt::fmt_color;
#endif
            break;
        case 'v':
            std::cout << prog_name << ' ' << UJSON_VERSION_STRING << std::endl;
            exit (0);
            break;
        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;
        case -2:
            cerr << "Missing argument to option '" << opt.opt() << "'" << endl;
            exit (1);
            break;
        default:
            cerr << "Unknown option: '" << opt.opt() << "'" << endl;
            exit (1);
            break;
        }
    }

    auto& arguments = opt.arguments ();
    if (!arguments.empty()) {
        args.filename = arguments[0];
        if (arguments.size() > 1) {
            cerr << "Too many arguments" << endl;
            exit (1);
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int parse_multiple_instances (istream& in,appargs_t& opt)
{
    ujson::jparser parser;
    std::string line;

    while (getline(in, line)) {
        auto instance = parser.parse_string (line, opt.parse_strict, opt.allow_duplicates);
        if (instance.valid())
            cout << instance.describe(opt.fmt) << endl;
        else
            cerr << "Error: " << parser.error() << endl;
    }
    return in.fail() ? 1 : 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t opt;
    parse_args (argc, argv, opt);

    try {
        // Open json document
        //
        ifstream ifs;
        ifs.exceptions (std::ifstream::failbit);
        if (!opt.filename.empty())
            ifs.open (opt.filename);
        istream& in = opt.filename.empty() ? cin : ifs;
        in.exceptions (std::ifstream::failbit);

        if (opt.multi_doc) {
            // Treat the input as a stream of
            // JSON instances separated by line breaks.
            return parse_multiple_instances (in, opt);
        }

        // Read and parse json document
        //
        string buffer ((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
        ujson::jparser parser;
        auto instance = parser.parse_string (buffer, opt.parse_strict, opt.allow_duplicates);
        if (!instance.valid()) {
            auto err = parser.get_error ();
            cerr << "Parse error at " << (err.row+1) << ", " << err.col
                 << ": " << parser_err_to_str(err.code) << endl;
            exit (1);
        }

        // Print the parsed json instance
        //
        cout << instance.describe(opt.fmt) << endl;
        return 0;
    }
    catch (std::ios_base::failure& io_error) {
        if (opt.filename.empty())
            cerr << "Error reading input: " << io_error.code().message() << endl;
        else
            cerr << "Error reading file '" << opt.filename << "': " << io_error.code().message() << endl;
        exit (1);
    }
}
