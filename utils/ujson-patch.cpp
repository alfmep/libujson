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
#include <algorithm>
#include <cerrno>
#include <unistd.h>
#include <getopt.h>


using namespace std;

static constexpr const char* prog_name = "ujson-patch";

struct appargs_t {
    bool compact;
    bool relaxed;
    std::string document_filename;
    std::string patch_filename;

    appargs_t () {
        compact = false;
        relaxed = false;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl;
    out << "Patch a JSON document." << endl;
    out << endl;
    out << "Usage: " << prog_name << " [OPTIONS] [JSON_FILE] [JSON_PATCH_FILE]" << endl;
    out << endl;
    out << "JSON_FILE:       This JSON document to patch." <<endl;
    out << "JSON_PATCH_FILE: JSON document containing the patch/patches, or standard input if no filename given." <<endl;
    out << "The resulting JSON document will be printed to standard output." << endl;
    out << "If one or more patches fails, error info is written to standard error." << endl;
    out << endl;
    out << "Options:" <<endl;
    out << "  -c, --compact       Print the resulting JSON document without whitespaces." << endl;
    out << "  -r, --relaxed       Parse JSON documents in relaxed mode." << endl;
    out << "  -h, --help          Print this help message and exit." << endl;
    out << endl;
    out << "" << endl;
    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    struct option long_options[] = {
        { "compact", no_argument, 0, 'c'},
        { "relaxed", no_argument, 0, 'r'},
        { "help",    no_argument, 0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "crh";

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'c':
            args.compact = true;
            break;
        case 'r':
            args.relaxed = true;
            break;
        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;
        default:
            exit (1);
            break;
        }
    }
    if (optind < argc) {
        args.document_filename = argv[optind++];
    }else{
        cerr << "Missing argument" << endl;
        exit (1);
    }
    if (optind < argc)
        args.patch_filename = argv[optind++];
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

    // Parse the JSON document
    //
    ujson::Json j;
    auto instance = j.parse_file (opt.document_filename, !opt.relaxed);
    if (!instance.valid()) {
        cerr << "Parse error, " << opt.document_filename << ": " << j.error() << endl;
        exit (1);
    }

    // Parse the JSON patch(es)
    //
    ujson::jvalue patch;
    if (opt.patch_filename.empty()) {
        ifstream ifs;
        string json_desc ((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
        patch = j.parse_string (json_desc, !opt.relaxed);
    }else{
        patch = j.parse_file (opt.patch_filename, !opt.relaxed);
    }
    if (!patch.valid()) {
        if (opt.patch_filename.empty())
            cerr << "Parse error: <standard input>: " << j.error() << endl;
        else
            cerr << "Parse error, " << opt.patch_filename << ": " << j.error() << endl;
        exit (1);
    }

    // Apply patches
    //
    int retval = 0;
    if (patch.type() != ujson::j_array) {
        ujson::patch (instance, patch);
        if (errno != 0) {
            switch (errno) {
            case EINVAL:
                cerr << "Patch 1 of 1 - Error: Invalid patch definition" << endl;
                break;
            case ENOENT:
                cerr << "Patch 1 of 1 - Error: Invalid JSON pointer(s)" << endl;
                break;
            default:
                cerr << "Patch 1 of 1 - Unknown error" << endl;
                break;
            }
            retval = 1;
        }
    }else{
        auto& patch_list = patch.array ();
        size_t num_patches = patch_list.size ();
        for (size_t i=0; i<num_patches; ++i) {
            ujson::patch (instance, patch_list[i]);
            if (errno != 0) {
                cerr << "Patch " << (i+1) << " of " << num_patches << " - ";
                switch (errno) {
                case EINVAL:
                    cerr << "Error: Invalid patch definition" << endl;
                    break;
                case ENOENT:
                    cerr << "Error: Invalid JSON pointer(s)" << endl;
                    break;
                default:
                    cerr << "Unknown error" << endl;
                    break;
                }
                retval = 1;
            }
        }
    }

    // Print the patched json instance
    //
    cout << instance.describe(!opt.compact, !opt.relaxed);

    return retval;
}
