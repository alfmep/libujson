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
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "option-parser.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;


static constexpr const char* prog_name = "ujson-verify";

struct appargs_t {
    bool strict;
    bool quiet;
    std::vector<string> files;

    appargs_t() {
        strict = true;
        quiet = false;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl
        << "Verify the syntax of JSON documents." << endl
        << endl
        << "Usage: " << prog_name << " [OPTIONS] [FILE...]" << endl
        << endl
        << "Options:" <<endl
        << "  -q, --quiet          Silent mode, don't write anything to standard output." << endl
        << "  -r, --relaxed        Relaxed parsing, don't use strict mode when parsing." << endl
        << "  -v, --version        Print version and exit." << endl
        << "  -h, --help           Print this help message and exit." << endl
        << endl;
        exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    optlist_t options = {
        { 'r', "relaxed", opt_t::none, 0},
        { 'q', "quiet",   opt_t::none, 0},
        { 'v', "version", opt_t::none, 0},
        { 'h', "help",    opt_t::none, 0},
    };

    option_parser opt (argc, argv);
    while (int id=opt(options)) {
        switch (id) {
        case 'r':
            args.strict = false;
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
            break;
        }
    }
    for (auto& argument : opt.arguments())
        args.files.emplace_back (argument);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int verify_document (const std::string& filename,
                            ujson::jparser& parser,
                            bool strict,
                            bool quiet)
{
    std::string log_filename = filename.empty() ? "JSON document" : filename;

    // Parse file and check result
    auto instance = parser.parse_file (filename, strict);
    if (!instance.valid()) {
        if (!quiet)
            cout << log_filename << ": Error: " << parser.error() << endl;
        return 1;
    }
    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t args;
    parse_args (argc, argv, args);

    ujson::jparser parser;

    if (args.files.empty())
        args.files.emplace_back (""); // Parse standard input

    int retval = 0;
    for (auto& filename : args.files) {
        std::string log_filename = filename.empty() ? "JSON document" : filename;
        if (verify_document(filename, parser, args.strict, args.quiet)) {
            retval = 1;
        }else if (!args.quiet) {
            cout << log_filename << ": ok" << endl;
        }
    }

    return retval;
}
