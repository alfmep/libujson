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
#include "parser-errors.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;


static constexpr const char* prog_name = "ujson-verify";

struct appargs_t {
    std::vector<string> files;
    std::vector<string> schema_files;
    bool strict;
    bool quiet;
    bool verbose;

    appargs_t() {
        strict = true;
        quiet = false;
        verbose = false;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl
        << "Verify the syntax of JSON documents, and optionally using a JSON schema." << endl
        << endl
        << "Usage: " << prog_name << " [OPTIONS] [FILE...]" << endl
        << endl
        << "Options:" <<endl
        << "  -q, --quiet                 Silent mode, don't write anything to standard output." << endl
        << "  -s, --schema=SCHEMA_FILE    Validate the JSON document using a JSON schema file." << endl
        << "                              This option may be set multiple times." << endl
        << "                              The first schema file is the main schema used to validate" << endl
        << "                              the JSON document. More schema files can then be added that" << endl
        << "                              can be referenced by the main and other schema files." << endl
        << "  -d, --verbose               Verbose mode. Print verbose schema validation output." << endl
        << "  -r, --relaxed               Relaxed parsing, don't use strict mode when parsing." << endl
        << "  -v, --version               Print version and exit." << endl
        << "  -h, --help                  Print this help message and exit." << endl
        << endl;
        exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    optlist_t options = {
        { 'q', "quiet",   opt_t::none, 0},
        { 's', "schema",  opt_t::required, 0},
        { 'd', "verbose", opt_t::none, 0},
        { 'r', "relaxed", opt_t::none, 0},
        { 'v', "version", opt_t::none, 0},
        { 'h', "help",    opt_t::none, 0},
    };

    option_parser opt (argc, argv);
    while (int id=opt(options)) {
        switch (id) {
        case 'q':
            args.quiet = true;
            break;
        case 's':
            args.schema_files.emplace_back (opt.optarg());
            break;
        case 'd':
            args.verbose = true;
            break;
        case 'r':
            args.strict = false;
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
                            ujson::jschema& schema,
                            bool use_schema,
                            const appargs_t& args)
{
    std::string log_filename = filename.empty() ? "JSON document" : filename;

    // Parse file and check result
    auto instance = parser.parse_file (filename, args.strict);
    if (!instance.valid()) {
        if (!args.quiet) {
            auto err = parser.get_error ();
            cout << log_filename << ": Error at " << err.row << ", " << err.col
                 << ": " << parser_err_to_str(err.code) << endl;
        }
        return 1;
    }

    ujson::jvalue result;

    if (use_schema) {
        try {
            result = schema.validate (instance);
            if (result["valid"] == false) {
                if (args.quiet)
                    return 1;

                if (!args.verbose) {
                    cout << log_filename << ": Schema not successfully validated" << endl;
                }else{
                    cout << log_filename << ": Validation error: " << endl;
#if (UJSON_HAS_CONSOLE_COLOR)
                    if (isatty(fileno(stdout)))
                        cout << result.describe(ujson::fmt_pretty | ujson::fmt_sep_elements | ujson::fmt_color) << endl;
                    else
#endif
                        cout << result.describe(ujson::fmt_pretty | ujson::fmt_sep_elements) << endl;
                    cout << endl;
                }
                return 1;
            }
        }
        catch (ujson::invalid_schema& is) {
            cerr << "Schema error   : " << is.what() << endl;
            if (args.verbose) {
                if (!is.base_uri.empty())
                    cerr << "Base URI: " << is.base_uri << endl;
                if (!is.pointer.empty())
                    cerr << "Pointer : " << is.pointer << endl;
            }
            exit (1);
        }
    }

    if (!args.quiet) {
        if (!args.verbose || !use_schema) {
            cout << log_filename << ": ok" << endl;
        }else{
            cout << log_filename << ':' << endl;
            //cout << "    ";
#if (UJSON_HAS_CONSOLE_COLOR)
            if (isatty(fileno(stdout)))
                cout << result.describe(ujson::fmt_pretty|ujson::fmt_sep_elements|ujson::fmt_color) << endl;
            else
#endif
                cout << result.describe(ujson::fmt_pretty|ujson::fmt_sep_elements) << endl;
            cout << endl;
        }
    }

    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static bool load_schema (ujson::jparser& parser, ujson::jschema& schema, const appargs_t& args)
{
    bool use_schema = false;

    for (auto& schema_file : args.schema_files) {
        auto schema_def = parser.parse_file (schema_file, args.strict);
        if (schema_def.invalid()) {
            auto err = parser.get_error ();
            cerr << "Error: Parse error in schema file '" << schema_file << "' at "
                 << err.row << ", " << err.col << ": " << parser_err_to_str(err.code) << endl;
            exit (1);
        }
        try {
            if (use_schema == false) {
                schema.reset (schema_def);
                use_schema = true;
            }else{
                schema.add_referenced_schema (schema_def);
            }
        }
        catch (ujson::invalid_schema& is) {
            cerr << "Error: Schema file '" << schema_file << "' is not a valid schema." << endl;
            if (args.verbose) {
                cerr << "Error   : " << is.what() << endl;
                if (!is.base_uri.empty())
                    cerr << "Base URI: " << is.base_uri << endl;
                if (!is.pointer.empty())
                    cerr << "Pointer : " << is.pointer << endl;
            }
            exit (1);
        }
    }

    return use_schema;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t args;
    parse_args (argc, argv, args);

    ujson::jparser parser;
    ujson::jschema schema;

    if (args.files.empty())
        args.files.emplace_back (""); // Parse standard input

    bool use_schema = load_schema (parser, schema, args);

    int retval = 0;
    for (auto& filename : args.files) {
        //std::string log_filename = filename.empty() ? "JSON document" : filename;
        if (verify_document(filename, parser, schema, use_schema, args)) {
            retval = 1;
        }
    }

    return retval;
}
