/*
 * Copyright (C) 2023,2024 Dan Arrhenius <dan@ultramarin.se>
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
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <list>
#include <set>
#include <cstdlib>
#include <cstdint>
#include <cerrno>
#include <unistd.h>
#include <getopt.h>
#include <ujson.hpp>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

namespace uj = ujson;


//------------------------------------------------------------------------------
//  T Y P E S
//------------------------------------------------------------------------------
struct appargs_t {
    std::string load_dir;
    std::vector<std::string> args; // Arguments that are not options
    bool full_validation;

    appargs_t (int argc, char* argv[]);
    void print_usage_and_exit (std::ostream& out, int exit_code);
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void appargs_t::print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << std::endl
        << "Usage: " << program_invocation_short_name << " [OPTIONS]  <json-schema ...> <json-instance>" << std::endl
        << std::endl
        << "  -d, --dir=DIR            Load referenced schemas from this diectory." << std::endl
        << "  -f, --full_validation    Validate all values in the instance even if some fails validation." << std::endl
        << "  -h, --help               Print this help message." << std::endl
        << std::endl;

        exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
appargs_t::appargs_t (int argc, char* argv[])
    : full_validation (false)
{
    static struct option long_options[] = {
        { "dir",             required_argument, 0, 'd'},
        { "full-validation", no_argument,       0, 'f'},
        { "help",            no_argument,       0, 'h'},
        { 0, 0, 0, 0}
    };
    static const char* arg_format = "d:fh";

    while (1) {
        int c = getopt_long (argc, argv, arg_format, long_options, NULL);
        if (c == -1)
            break;
        switch (c) {
        case 'd':
            load_dir = optarg;
            break;
        case 'f':
            full_validation = true;
            break;
        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;
        default:
            std::cerr << "Use option -h for help." << std::endl;
            exit (1);
        }
    }

    // Collect all arguments thar are not options
    while (optind < argc)
        args.emplace_back (argv[optind++]);

    if (args.size() < 2) {
        std::cerr << "Missing argument" << std::endl;
        exit (1);
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static bool invalid_ref_cb (uj::jschema& schema,
                            const std::string& base_uri,
                            const std::string& ref_value,
                            const std::string& load_dir)
{
    /*
    cerr << "Invalid ref:" << endl;
    cerr << "    base_uri : "  << base_uri << endl;
    cerr << "    ref_value: "  << ref_value << endl;
    cerr << "    load_dir : "  << load_dir << endl;
    */
    uj::jparser parser;

    std::string file_name = load_dir;
    if (file_name[file_name.size()-1] != '/')
        file_name.push_back ('/');

    std::string uri;
    std::string fragment;
    uj::schema::jvocabulary::split_uri (ref_value, uri, fragment);

    std::string local_file_uri = "http://localhost:1234/";
    std::string::size_type pos;

    pos = uri.find (local_file_uri);
    if (pos != 0) {
        if (!base_uri.empty() && base_uri.back()=='/') {
            uri = base_uri + uri;
        }else{
            std::string ignore_me;
            uri = uj::schema::jvocabulary::resolve_id (base_uri, uri, ignore_me);
        }
        pos = uri.find (local_file_uri);
    }

    if (pos == 0) {
        file_name.append (uri.substr(local_file_uri.size()));
        auto ref_schema = parser.parse_file (file_name, false);
        if (ref_schema.invalid())
            return false;

        schema.add_referenced_schema (ref_schema, uri);
        return true;
    }

    return false;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t opt (argc, argv);
    uj::jparser parser;
    auto num_args = opt.args.size ();
    uj::jvalue root;
    uj::jvalue instance;
    std::list<uj::jvalue> dep_schemas;

    try {

        // Parse JSON files
        //
        root = parser.parse_file (opt.args[0], false);
        if (root.invalid()) {
            cerr << "Error parsing file '" << opt.args[0] << "': " << parser.error() << endl;
            return 1;
        }
        instance = parser.parse_file (opt.args[num_args-1], false);
        if (instance.invalid()) {
            cerr << "Error parsing file '" << opt.args[num_args-1] << "': " << parser.error() << endl;
            return 1;
        }
        for (size_t i=1; i<(num_args-1); ++i) {
            auto dep_schema = parser.parse_file (opt.args[i], false);
            if (dep_schema.invalid()) {
                cerr << "Error parsing file '" << opt.args[i] << "': " << parser.error() << endl;
                return 1;
            }
            dep_schemas.emplace_back (dep_schema);
        }

        // Create JSON schema
        //
        uj::jschema s (root, dep_schemas);
        if (opt.load_dir.empty() == false) {
            s.set_invalid_ref_cb ([&opt](uj::jschema& schema,
                                         const std::string& base_uri,
                                         const std::string& ref_value)->bool{
                return invalid_ref_cb (schema, base_uri, ref_value, opt.load_dir);
            });
        }

        auto ou = s.validate (instance, opt.full_validation?false:true);

        if (isatty(fileno(stdout)))
            cout << ou.describe(uj::fmt_pretty | uj::fmt_color) << endl;
        else
            cout << ou.describe(uj::fmt_pretty) << endl;

        //return ou["valid"] == true ? 0 : 1;
        return 0;
    }
    catch (uj::invalid_schema& is) {
        cerr << endl;
        cerr << "Error   : " << is.what() << endl;
        if (!is.base_uri.empty())
            cerr << "Base URI: " << is.base_uri << endl;
        if (!is.pointer.empty())
            cerr << "Pointer : " << is.pointer << endl;
        return 1;
    }
    catch (std::exception& e) {
        cerr << "Unexpected error: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "Error: Unknown expection" << endl;
        return 1;
    }
}
