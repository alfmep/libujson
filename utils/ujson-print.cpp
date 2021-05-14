/*
 * Copyright (C) 2021 Dan Arrhenius <dan@ultramarin.se>
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
    bool strict;
    string filename;

    appargs_t () {
        pretty = true;
        escape_slash = false;
        sorted = false;
        strict = true;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl
        << "Parse a json file (in relaxed mode) and print it." << endl
        << endl
        << "Usage: " << prog_name << " [OPTIONS] [json-file]" << endl
        << endl
        << "In no file name is given, a json definition is read from standard input." << endl
        << "Options:" <<endl
        << "  -c, --compact        Compact output, no newlines or intendation." << endl
        << "  -e, --escape-slash   Forward slash characters(\"/\") are escaped to \"\\/\"." << endl
        << "  -s, --sort           Object members are listed in sorted order, not in natural order." << endl
        << "  -r, --relaxed        Numbers that are infinite or NaN are representated as numbers (-)inf and nan," << endl
        << "                       and not as type null as in the JSON specification." << endl
        << "  -h, --help           Print this help message and exit." << endl
        << endl;
        exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    struct option long_options[] = {
        { "compact",      no_argument, 0, 'c'},
        { "escape-slash", no_argument, 0, 'e'},
        { "sort",         no_argument, 0, 's'},
        { "relaxed",      no_argument, 0, 'r'},
        { "help",         no_argument, 0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "cesrh";

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, NULL);
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
        case 'r':
            args.strict = false;
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
    ujson::Json j;
    auto instance = j.parse_string (json_desc, false);
    if (!instance.valid()) {
        cerr << "Parse error: " << j.error() << endl;
        exit (1);
    }

    // Print the parsed json instance
    //
    cout << instance.describe(opt.pretty, opt.strict, opt.escape_slash, opt.sorted) << endl;

    return 0;
}
