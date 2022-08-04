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
#include <unistd.h>
#include <getopt.h>


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
    struct option long_options[] = {
        { "relaxed", no_argument, 0, 'r'},
        { "quiet",   no_argument, 0, 'q'},
        { "version", no_argument, 0, 'v'},
        { "help",    no_argument, 0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "rqvh";

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'r':
            args.relaxed = true;
            break;
        case 'q':
            args.quiet = true;
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
    if (optind < argc)
        args.filename[0] = argv[optind++];
    if (optind < argc)
        args.filename[1] = argv[optind++];

    if (args.filename[0].empty() || args.filename[1].empty()) {
        if (!args.quiet)
            cerr << "Missing filename(s)" << endl;
        exit (1);
    }
    if (optind < argc) {
        if (!args.quiet)
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
        ifs[i].open (opt.filename[i]);
        json_desc[i] = string ((istreambuf_iterator<char>(ifs[i])), istreambuf_iterator<char>());
        ifs[i].close ();
    }

    // Parse json files
    //
    ujson::Json j;
    ujson::jvalue instance[2];
    for (auto i=0; i<2; ++i) {
        instance[i] = j.parse_string (json_desc[i], false);
        if (!instance[i].valid()) {
            if (!opt.quiet)
                cerr << "Error parsing " << opt.filename[i] << ": " << j.error() << endl;
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
