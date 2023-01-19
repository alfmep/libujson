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
#include <algorithm>
#include <cerrno>
#include <unistd.h>
#include <getopt.h>


using namespace std;

static constexpr const char* prog_name = "ujson-patch";

struct appargs_t {
    bool compact;
    bool relaxed;
    bool quiet;
    std::string document_filename;
    std::string patch_filename;

    appargs_t () {
        compact = false;
        relaxed = false;
        quiet   = false;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl;
    out << "Patch a JSON document using a patch as described by RFC 6902." << endl;
    out << endl;
    out << "Usage: " << prog_name << " [OPTIONS] JSON_FILE [JSON_PATCH_FILE]" << endl;
    out << endl;
    out << "JSON_FILE:       This a JSON document to patch." <<endl;
    out << "JSON_PATCH_FILE: JSON document containing the patch/patches, or standard input if no filename given." <<endl;
    out << "The resulting JSON document will be printed to standard output." << endl;
    out << "If one or more patches fails, error info is written to standard error." << endl;
    out << endl;
    out << "Options:" <<endl;
    out << "  -c, --compact    Print the resulting JSON document without whitespaces." << endl;
    out << "  -r, --relaxed    Parse JSON input files in relaxed mode." << endl;
    out << "  -q, --quiet      No errors are written to standard error. On errors, or failed patch test operations," << endl;
    out << "                   the application exits with an error code. If the patch definition only contains" << endl;
    out << "                   patch operations of type 'test', nothing is written to standard output." << endl;
    out << "                   If the patch definition contains operations other than 'test', the resulting JSON" << endl;
    out << "                   document is still printed to standard output." << endl;
    out << "  -v, --version    Print version and exit." << endl;
    out << "  -h, --help       Print this help message and exit." << endl;
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
        { "quiet",   no_argument, 0, 'q'},
        { "version", no_argument, 0, 'v'},
        { "help",    no_argument, 0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "crqvh";

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
            exit (1);
            break;
        }
    }
    if (optind < argc) {
        args.document_filename = argv[optind++];
    }else{
        if (!args.quiet)
            cerr << "Missing argument" << endl;
        exit (1);
    }
    if (optind < argc)
        args.patch_filename = argv[optind++];
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

    // Parse the JSON document
    //
    ujson::jparser parser;
    auto instance = parser.parse_file (opt.document_filename, !opt.relaxed);
    if (!instance.valid()) {
        if (!opt.quiet)
            cerr << "Parse error, " << opt.document_filename << ": " << parser.error() << endl;
        exit (1);
    }

    // Parse the JSON patch(es)
    //
    ujson::jvalue patch;
    if (opt.patch_filename.empty()) {
        ifstream ifs;
        string json_desc ((istreambuf_iterator<char>(cin)), istreambuf_iterator<char>());
        patch = parser.parse_string (json_desc, !opt.relaxed);
    }else{
        patch = parser.parse_file (opt.patch_filename, !opt.relaxed);
    }
    if (!patch.valid()) {
        if (!opt.quiet) {
            if (opt.patch_filename.empty())
                cerr << "Parse error: <standard input>: " << parser.error() << endl;
            else
                cerr << "Parse error, " << opt.patch_filename << ": " << parser.error() << endl;
        }
        exit (1);
    }

    auto result = ujson::patch (instance, patch);
    bool only_test_ops = false;

    if (opt.quiet != true) {
        auto num_patches = result.second.size ();
        for (unsigned i=0; i<num_patches; ++i) {
            switch (result.second[i]) {
            case ujson::patch_ok:
                break;
            case ujson::patch_fail:
                cerr << "Patch " << (i+1) << " of " << num_patches << " - ";
                cerr << "Test operation failed" << endl;
                break;
            case ujson::patch_invalid:
                cerr << "Patch " << (i+1) << " of " << num_patches << " - ";
                cerr << "Error: Invalid patch definition" << endl;
                break;
            case ujson::patch_noent:
                cerr << "Patch " << (i+1) << " of " << num_patches << " - ";
                cerr << "Error: JSON pointer mismatch" << endl;
                break;
            default:
                cerr << "Patch " << (i+1) << " of " << num_patches << " - ";
                cerr << "Unknown error" << endl;
                break;
            }
        }
    }else{
        only_test_ops = true;
        // Check if the patch contains only test operations
        if (patch.is_array()) {
            // Array of patches
            for (auto& entry : patch.array()) {
                auto& op = entry.get ("op");
                if (op.type()==ujson::j_string && op.str()!="test") {
                    only_test_ops = false;
                    break;
                }
            }
        }else{
            // Single patch
            auto& op = patch.get ("op");
            if (op.type()==ujson::j_string && op.str()!="test")
                only_test_ops = false;
        }
    }


    // Print the patched json instance
    //
    if (!opt.quiet || !only_test_ops)
        cout << instance.describe(!opt.compact) << endl;

    return result.first ? 0 : 1;
}
