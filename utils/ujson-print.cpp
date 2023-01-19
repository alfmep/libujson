/*
 * Copyright (C) 2021-2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <unistd.h>
#include <getopt.h>


using namespace std;

static constexpr const char* prog_name = "ujson-print";

struct appargs_t {
    bool pretty;
    bool escape_slash;
    bool sorted;
    bool array_items_on_same_line;
    bool parse_strict;
    bool print_relaxed;
    string filename;

    appargs_t () {
        pretty = true;
        escape_slash = false;
        array_items_on_same_line = true;
        sorted = false;
        parse_strict = false;
        print_relaxed = false;
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
    out << "  -c, --compact         Compact output, no newlines or intendation." << endl;
    out << "  -e, --escape-slash    Forward slash characters(\"/\") are escaped to \"\\/\"." << endl;
    out << "  -s, --sort            Object members are listed in sorted order, not in natural order." << endl;
    out << "  -a, --array-lines     For JSON arrays, print each array item on a separate line." << endl;
    out << "  -r, --relaxed         Print the JSON document in relaxed form." << endl;
    out << "                        Object member names are printed without enclosing double quotes" << endl;
    out << "                        when the names are in the following format: [_a-zA-Z][_a-zA-Z0-9]*" << endl;
    out << "  -t, --parse-strict    Parse the JSON document in strict mode." << endl;
    out << "  -v, --version         Print version and exit." << endl;
    out << "  -h, --help            Print this help message and exit." << endl;
    out << endl;
    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    struct option long_options[] = {
        { "compact",          no_argument, 0, 'c'},
        { "escape-slash",     no_argument, 0, 'e'},
        { "sort",             no_argument, 0, 's'},
        { "array-lines",      no_argument, 0, 'a'},
        { "relaxed",          no_argument, 0, 'r'},
        { "parse-strict",     no_argument, 0, 't'},
        { "version",          no_argument, 0, 'v'},
        { "help",             no_argument, 0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "cesartvh";

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, nullptr);
        if (c == -1)
            break;
        switch (c) {
        case 'c':
            args.pretty = false;
            break;
        case 'e':
            args.escape_slash = true;
            break;
        case 's':
            args.sorted = true;
            break;
        case 'a':
            args.array_items_on_same_line = false;
            break;
        case 'r':
            args.print_relaxed = true;
            break;
        case 't':
            args.parse_strict = true;
            break;
        case 'v':
            std::cout << prog_name << ' ' << UJSON_VERSION_STRING << std::endl;
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
    if (optind < argc)
        args.filename = argv[optind++];
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

    // Read json file
    //
    ifstream ifs;
    if (!opt.filename.empty())
        ifs.open (opt.filename);
    istream& in = opt.filename.empty() ? cin : ifs;
    string json_desc ((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());

    // Parse json file
    //
    ujson::jparser parser;
    auto instance = parser.parse_string (json_desc, opt.parse_strict);
    if (!instance.valid()) {
        cerr << "Parse error: " << parser.error() << endl;
        exit (1);
    }

    // Print the parsed json instance
    //
    cout << instance.describe (opt.pretty,
                               opt.array_items_on_same_line,
                               opt.sorted,
                               opt.escape_slash,
                               opt.print_relaxed)
         << endl;

    return 0;
}
