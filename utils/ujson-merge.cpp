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
#include <unistd.h>
#include <getopt.h>


using namespace std;

struct appargs_t {
    bool compact;
    bool relaxed;
    bool type_check;
    string master_filename;
    vector<string> slave_filenames;
    string master_location;
    string slave_location;

    appargs_t () {
        compact = false;
        relaxed = false;
        type_check = false;
    }
};


/*
 * Merge rules:
 *
 * Merge a slave instance into a master instance.
 *
 * A number will be replaced by the slave value.
 * A null will be replaced by the slave value.
 * A boolean will be replaced by the slave value.
 * A string will be replaced by the slave value.
 * An array will be replaced by the slave value.
 * An object:
 *   - If a member name in the slave object exitst in the master object,
 *     the value of that member will be merged according to theese rules.
 *   - If a member name in the slave object doesn't exitst in the master object,
 *     the slave member will be added to the master object.
 */


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl;
    out << "Merge one or more JSON document into a target document.." << endl;
    out << endl;
    out << "Usage: " << program_invocation_short_name << " [OPTIONS] [MASTER_FILE] [SLAVE_FILE...]" << endl;
    out << endl;
    out << "MASTER_FILE: This JSON document serves as the base for the merge." <<endl;
    out << "SLAVE_FILE:  Values from this file will be merged with the master file." <<endl;
    out << "The resulting JSON document will be printed to standard output." << endl;
    out << endl;
    out << "Options:" <<endl;
    out << "  -c, --compact                Print the resulting JSON document without whitespaces." << endl;
    out << "  -r, --relaxed                Parse JSON documents in relaxed mode." << endl;
    out << "  -d, --dst-location=LOCATION  Merge into this specific location in the master document." << endl;
    out << "  -s, --src-location=LOCATION  Merge from this specific location in the slave document(s)." << endl;
    out << "  -m, --match-type             The value(s) in the master and slave document must be of the same type." << endl;
    out << "  -h, --help                   Print this help message and exit." << endl;
    out << endl;
    out << "" << endl;
    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    struct option long_options[] = {
        { "compact",      no_argument,       0, 'c'},
        { "relaxed",      no_argument,       0, 'r'},
        { "dst-location", required_argument, 0, 'd'},
        { "src-location", required_argument, 0, 's'},
        { "match-type",   no_argument,       0, 'm'},
        { "help",         no_argument,       0, 'h'},
        { 0, 0, 0, 0}
    };
    const char* arg_format = "crd:s:mh";

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
        case 'd':
            args.master_location = optarg;
            break;
        case 's':
            args.slave_location = optarg;
            break;
        case 'm':
            args.type_check = true;
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
        args.master_filename = argv[optind++];
    }else{
        cerr << "Missing argument" << endl;
        exit (1);
    }
    while (optind < argc)
        args.slave_filenames.emplace_back (argv[optind++]);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int merge_instance (ujson::jvalue& mvalue, ujson::jvalue& svalue, bool type_check)
{
    // Type check
    if (type_check && mvalue.type()!=svalue.type()) {
        cerr << "JSON type mismatch. Trying to assign a value of type "
             << ujson::jtype_to_str(svalue.type())
             << " to an instance of type " << ujson::jtype_to_str(mvalue.type()) << endl;
        return 1;
    }
    // Sanity check
    if (mvalue.type()!=ujson::j_invalid && svalue.type()==ujson::j_invalid) {
        cerr << "JSON type mismatch. Trying to assign an invalid JSON value "
             << " to an instance of type " << ujson::jtype_to_str(mvalue.type()) << endl;
        return 1;
    }

    if (mvalue.type()!=ujson::j_object || svalue.type()!=ujson::j_object) {
        mvalue = svalue;
    }else{
        for (auto& sv : svalue.obj()) {
            auto& mv = mvalue.get(sv.first);
            if (mv.valid()) {
                if (merge_instance(mv, sv.second, type_check))
                    return 1;
            }else{
                mvalue.add (sv.first, sv.second);
            }
        }
    }

    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int parse_and_merge (const appargs_t& opt,
                            ujson::jvalue& master,
                            istream& ifs,
                            const std::string& slave_filename="")
{
    ujson::Json j;
    string slave_document ((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    auto slave_root = j.parse_string (slave_document, !opt.relaxed);
    if (!slave_root.valid()) {
        if (slave_filename.empty())
            cerr << "Parse error on input: " << j.error() << endl;
        else
            cerr << "Parse error, " << slave_filename << ": " << j.error() << endl;
        return 1;
    }

    auto& slave = ujson::find_jvalue (slave_root, opt.slave_location);
    if (!slave.valid()) {
        if (slave_filename.empty())
            cerr << "Invalid location in input document: " << opt.slave_location << endl;
        else
            cerr << "Invalid location in document "
                 << slave_filename << ": " << opt.slave_location << endl;
        return 1;
    }
    return merge_instance (master, slave, opt.type_check);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t opt;
    parse_args (argc, argv, opt);

    // Parse the master file
    //
    ujson::Json j;
    auto master_root = j.parse_file (opt.master_filename, !opt.relaxed);
    if (!master_root.valid()) {
        cerr << "Parse error, " << opt.master_filename << ": " << j.error() << endl;
        exit (1);
    }

    // Get the location in the master instance
    //
    auto& master = ujson::find_jvalue (master_root, opt.master_location);
    if (!master.valid()) {
        cerr << "Invalid location in document "
             << opt.master_filename << ": " << opt.master_location << endl;
        exit (1);
    }

    // Merge slave file(s)
    //
    int result = 0;
    if (opt.slave_filenames.empty()) {
        // Merge from standard input
        result = parse_and_merge (opt, master, cin);
    }else{
        // Merge from file(s)
        for (auto& filename : opt.slave_filenames) {
            ifstream ifs (filename);
            result = parse_and_merge (opt, master, ifs, filename);
            ifs.close ();
            if (result)
                break;
        }
    }

    if (!result)
        cout << master_root.describe(!opt.compact, !opt.relaxed) << endl;

    return result;
}
