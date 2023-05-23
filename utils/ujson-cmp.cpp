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
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include "option-parser.hpp"


using namespace std;

static constexpr const char* prog_name = "ujson-cmp";

struct appargs_t {
    bool relaxed;
    bool quiet;
    string filename[2];

    appargs_t () {
        relaxed = false;
        quiet = false;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl
        << "Check if two JSON instances are equal." << endl
        << endl
        << "Usage: " << prog_name << " [OPTIONS] [FILE_1] [FILE_2]" << endl
        << endl
        << "Options:" <<endl
        << "  -r, --relaxed    Parse JSON documents in relaxed mode." << endl
        << "  -q, --quiet      Silent mode, don't write anything to standard output." << endl
        << "  -v, --version    Print version and exit." << endl
        << "  -h, --help       Print this help message and exit." << endl
        << endl;
        exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    optlist_t options = {
        {'r', "relaxed", opt_t::none, 0},
        {'q', "quiet",   opt_t::none, 0},
        {'v', "version", opt_t::none, 0},
        {'h', "help",    opt_t::none, 0},
    };

    option_parser opt (argc, argv);
    while (int id=opt(options)) {
        switch (id) {
        case 'r':
            args.relaxed = true;
            break;
        case 'q':
            args.quiet = true;
            break;
        case 'v':
            std::cout << prog_name << ' ' << UJSON_VERSION_STRING << std::endl;
            exit (0);
            break;
        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;
        default:
            cerr << "Unknown option: '" << opt.opt() << "'" << endl;
            exit (1);
        }
    }

    auto& arguments = opt.arguments ();
    switch (arguments.size()) {
    case 0:
    case 1:
        cerr << "Missing filename(s)" << endl;
        exit (1);
    case 2:
        args.filename[0] = arguments[0];
        args.filename[1] = arguments[1];
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

    parse_args (argc, argv, opt);

    // Read json files
    //
    ifstream ifs[2];
    string json_desc[2];
    for (auto i=0; i<2; ++i) {
        // Open
        ifs[i].open (opt.filename[i]);
        if (ifs[i].fail()) {
            cerr << "Can't open file \"" << opt.filename[i] << "\" - " << strerror(errno) << endl;
            exit (1);
        }
        // Read
        json_desc[i] = string ((istreambuf_iterator<char>(ifs[i])), istreambuf_iterator<char>());
        if (ifs[i].fail()) {
            cerr << "Error reading file \"" << opt.filename[i] << "\" - " << strerror(errno) << endl;
            exit (1);
        }
        ifs[i].close ();
    }

    // Parse json files
    //
    ujson::jparser parser;
    ujson::jvalue instance[2];
    for (auto i=0; i<2; ++i) {
        instance[i] = parser.parse_string (json_desc[i], !opt.relaxed);
        if (!instance[i].valid()) {
            if (!opt.quiet)
                cerr << "Error parsing " << opt.filename[i] << ": " << parser.error() << endl;
            exit (1);
        }
    }

    // Check if the instances are equal
    //
    int retval = 0;
    if (instance[0] == instance[1]) {
        if (!opt.quiet)
            cout << "equal" << endl;
    }else{
        if (!opt.quiet)
            cout << "not equal" << endl;
        retval = 1;
    }

    return retval;
}
