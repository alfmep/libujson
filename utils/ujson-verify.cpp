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
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;


struct appargs_t {
    bool quiet;
    std::vector<string> files;
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl
        << "Verify the syntax of json files." << endl
        << endl
        << "Usage: " << program_invocation_short_name << " [OPTIONS] <json-file ...>" << endl
        << endl
        << "Options:" <<endl
        << "  -q, --quiet   Suppress all output." << endl
        << "  -h, --help    Print this help message." << endl
        << endl;
        exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    struct option long_options[] = {
        { "quiet", no_argument, 0, 'q'},
        { "help",  no_argument, 0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "qh";

    args.quiet = false;

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'q':
            args.quiet = true;
            break;
        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;
        default:
            exit (1);
            break;
        }
    }
    while (optind < argc)
        args.files.emplace_back (argv[optind++]);

    if (args.files.empty()) {
        cerr << "Missing filename" << endl;
        exit (1);
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static bool is_file_ok (const string& filename)
{
    bool retval = true;

    auto fd = open (filename.c_str(), O_RDONLY);
    if (fd < 0)
        return false;

    struct stat sb;
    if (fstat(fd, &sb))
        retval = false;
    else
        retval = sb.st_mode & S_IFREG ? true : false;

    close (fd);
    return retval;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t args;
    parse_args (argc, argv, args);

    int retval = 0;
    ujson::Json parser;

    for (auto& filename : args.files) {
        // Check if file can be read
        if (!is_file_ok(filename)) {
            if (!args.quiet)
                cout << "Can't read file " << filename << endl;
            retval = 1;
            continue;
        }

        // Parse file and check result
        auto root = parser.parse_file (filename);
        if (root.valid()) {
            if (!args.quiet)
                cout << filename << ": ok" << endl;
        }else{
            retval = 1;
            if (!args.quiet)
                cout << filename << ": error: " << parser.errors().front() << endl;
        }
    }

    return retval;
}
