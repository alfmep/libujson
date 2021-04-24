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


static constexpr const char* prog_name = "ujson-verify";

struct appargs_t {
    bool strict;
    bool quiet;
    std::vector<string> files;
    std::string schema_file;
    std::vector<string> ref_schema_files;

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
        << "  -r, --relax            Relaxed parsing, don't use strict mode when parsing." << endl
        << "  -s, --schema=FILE      Validate the JSON document with a schema file.." << endl
        << "                         This option can be used multiple times." << endl
        << "                         The first schema will be used to validate the JSON document." << endl
        << "                         All schemas added after the first are schemas" << endl
        << "                         that can be referenced by the first schema." << endl
        << "  -q, --quiet            Silent mode, don't write anything to standard output." << endl
        << "  -h, --help             Print this help message and exit." << endl
        << endl;
        exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    struct option long_options[] = {
        { "relax",  no_argument,       0, 'r'},
        { "schema", required_argument, 0, 's'},
        { "quiet",  no_argument,       0, 'q'},
        { "help",   no_argument,       0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "rs:qh";

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'r':
            args.strict = false;
            break;
        case 's':
            if (args.schema_file.empty())
                args.schema_file = optarg;
            else
                args.ref_schema_files.emplace_back (optarg);
            break;
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
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int verify_document (const std::string& filename,
                            ujson::Json& parser,
                            ujson::Schema& schema,
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

    //
    if (schema.root().valid() &&
        schema.validate(instance) != ujson::Schema::valid)
    {
        if (!quiet)
            cout << log_filename << ": Not validated by schema" << endl;
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

    ujson::Json parser;
    ujson::Schema schema;

    if (!args.schema_file.empty()) {
        schema.root() = parser.parse_file (args.schema_file, args.strict);
        if (!schema.root().valid()) {
            if (!args.quiet)
                std::cerr << "Schema error in file '" << args.schema_file << "': "<< parser.error() << std::endl;
            exit (1);
        }
        for (auto& ref_file : args.ref_schema_files) {
            ujson::jvalue s = parser.parse_file (ref_file, args.strict);
            if (!s.valid()) {
                if (!args.quiet)
                    std::cerr << "Schema error in file '" << ref_file << "': "<< parser.error() << std::endl;
                exit (1);
            }
            if (!schema.add_ref_schema(ujson::Schema(s))) {
                std::cerr << "Error adding schema file '" << ref_file << "': Missing \"$id\"" << parser.error() << std::endl;
                exit (1);
            }
        }
    }

    if (args.files.empty())
        args.files.emplace_back (""); // Parse standard input

    int retval = 0;
    for (auto& filename : args.files) {
        std::string log_filename = filename.empty() ? "JSON document" : filename;
        if (verify_document(filename, parser, schema, args.strict, args.quiet))
            retval = 1;
        else if (!args.quiet)
            cout << log_filename << ": ok" << endl;
    }

    return retval;
}
