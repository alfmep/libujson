/*
 * Copyright (C) 2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <vector>
#include <map>
#include <unistd.h>
#include "option-parser.hpp"


using namespace std;

static constexpr const char* prog_name = "ujson-tool";

struct appargs_t {
    bool strict_parsing;

    bool print_pretty;
    bool print_sorted;
    bool print_escape_slash;
    bool print_array_items_on_same_line;
    bool print_unescaped_string;

    bool members_escape;
    bool members_as_json_array;

    bool quiet;

    ujson::jpointer ptr;
    ujson::jvalue_type required_type;

    string cmd;
    vector<string> args;

    appargs_t () {
        strict_parsing = true;
        print_pretty = true;
        print_escape_slash = false;
        print_array_items_on_same_line = true;
        print_unescaped_string = false;
        print_sorted = false;
        members_escape = false;
        members_as_json_array = false;
        quiet = false;
        required_type = ujson::j_invalid;
    }
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (std::ostream& out, int exit_code)
{
    out << endl;
    out << "View, inspect, and modify JSON documents." << endl;
    out << endl;
    out << "Usage: " << prog_name << " <COMMAND> [OPTIONS] [COMMAND_ARGUMENTS ...]" << endl;
    out << endl;
    out << "Common options:" << endl;
    out << "  -r, --relaxed          Relaxed parsing, don't use strict mode when parsing." << endl;
    out << "  -p, --pointer=POINTER  Use the JSON instance pointed to by the JSON pointer" << endl;
    out << "                         instead of the root of the input JSON document." << endl;
    out << "  -c, --compact          Any Resulting JSON output is printed without whitespaces." << endl;
    out << "  -s, --sort             Any Resulting JSON output is printed with object members sorted by name." << endl;
    out << "  -e, --escape-slash     In any resulting JSON string output," << endl;
    out << "                         forward slash characters(\"/\") are escaped to \"\\/\"." << endl;
    out << "  -a, --array-lines      In any resulting JSON output, print JSON array items on separate lines." << endl;
    out << "  -v, --version          Print version and exit." << endl;
    out << "  -h, --help             Print this help message and exit." << endl;
    out << endl;
    out << "All commands, except 'patch', reads a JSON document from standard input if no file is supplied." << endl;
    out << endl;
    out << "Commands:" << endl;
    out << "  view [OPTIONS] [JSON_DOCUMENT]" << endl;
    out << "    Print the JSON instance to standard output." << endl;
    out << "    Options:" << endl;
    out << "      -t, --type=TYPE    Require that the viewed instance is of a specific JSON type." << endl;
    out << "                         If the resulting instance is of another type, an error message" << endl;
    out << "                         is printed to standard error and 1 is returned." << endl;
    out << "                         Valid types are: object, array, string, number, boolean, and null." << endl;
    out << "      -u, --unescape     Only if the resulting instance is a JSON string:" << endl;
    out << "                         print the string value, unescaped witout enclosing double quotes." << endl;
    out << endl;
    out << "  verify [OPTIONS] [JSON_DOCUMENT]" << endl;
    out << "    Verify the syntax of the JSON document." << endl;
    out << "    Prints \"Ok\" to standard output and return 0 if the input is a valid JSON document." << endl;
    out << "    Prints an error message to standard error and return 1 if the input is a not valid JSON document." << endl;
    out << "    Common option '--pointer=POINTER' is ignored by this command." << endl;
    out << "    Options:" << endl;
    out << "      -q, --quiet    Print nothing, only return 0 on success, and 1 on error." << endl;
    out << endl;
    out << "  type [OPTIONS] [JSON_DOCUMENT]" << endl;
    out << "    Print or check the JSON type of the instance." << endl;
    out << "    Default is to write the JSON type of the instance to standard output." << endl;
    out << "    But if option '--type=TYPE' is used, the command will check if the JSON type of" << endl;
    out << "    the instance is the same type as specified." << endl;
    out << "    Options:" << endl;
    out << "      -t, --type=TYPE    Check if the JSON instance is of a specific JSON type." << endl;
    out << "                         If it is, print 'Yes' to standard output and return 0." << endl;
    out << "                         If not, print 'No' to standard output and return 1." << endl;
    out << "                         Valid types are: object, array, string, number, boolean, and null." << endl;
    out << "      -q, --quiet        If option '--type' is used, don't print anything." << endl;
    out << endl;
    out << "  size [OPTIONS] [JSON_DOCUMENT]" << endl;
    out << "    Print the number of elements/members to standard output if the JSON instance" << endl;
    out << "    is an array or object. If the JSON instance isn't an array or object," << endl;
    out << "    an error message is printed to standard error and 1 is returned." << endl;
    out << "    Note: It is not a recursive count. It is only the number of elements/members" << endl;
    out << "          in the specified array/object, not including sub-items of the array/object." << endl;
    out << endl;
    out << "  members [OPTIONS] [JSON_DOCUMENT]" << endl;
    out << "    If the instance is a JSON object, print the object member names to standard" << endl;
    out << "    output on separate lines. If not a JSON object, print an error message to" << endl;
    out << "    standard error and return 1." << endl;
    out << "    Note that the member names are by default printed as unescaped string values," << endl;
    out << "    and a single member name can thus be printed on multiple lines if it contains" << endl;
    out << "    one or more line breaks." << endl;
    out << "    Options:" << endl;
    out << "      -s, --sort            Sort the member names." << endl;
    out << "      -m, --escape-members  Print the member names as JSON formatted strings." << endl;
    out << "                            The names are printed JSON escaped, enclosed by double quotes." << endl;
    out << "                            This will ensure that no member name is written on multiple" << endl;
    out << "                            lines since newline characters are escaped." << endl;
    out << "                            This option is not needed if option '--json-array' is used." << endl;
    out << "      -j, --json-array      Print the member names as a JSON formatted array." << endl;
    out << "                            Option '--escape-members' is implied by this option." << endl;
    out << endl;
    out << "  patch [OPTIONS] <JSON_DOCUMENT> [JSON_PATCH_FILE]" << endl;
    out << "    Patch a JSON instance and print the result to standard output." << endl;
    out << "    If option '--pointer=...' is used, the patch definition uses this position in" << endl;
    out << "    the input JSON document as the instance to patch, and the resulting output will" << endl;
    out << "    also be from this position. If no patch file is supplied, the patch definition" << endl;
    out << "    is read from standard input. Errors and failed patch operations are printed to" << endl;
    out << "    standard error. Returns 0 if all patches are successfully aplied, and 1 if not." << endl;
    out << "    JSON patches are described in RFC 6902." << endl;
    out << "    Options:" << endl;
    out << "      -q, --quiet  Don't print failed patch operations to standard error, only return 1." << endl;
    out << "                   Also, if all patch operations are of type 'test', don't print the" << endl;
    out << "                   resulting JSON document to standard output." << endl;
    out << endl;
    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appargs_t& args)
{
    optlist_t options = {
        { 'r', "relaxed",        opt_t::none,     0},
        { 'p', "pointer",        opt_t::required, 0},
        { 'c', "compact",        opt_t::none,     0},
        { 's', "sort",           opt_t::none,     0},
        { 'e', "escape-slash",   opt_t::none,     0},
        { 'e', "escape-members", opt_t::none,     0},
        { 'a', "array-lines",    opt_t::none,     0},
        { 't', "type",           opt_t::required, 0},
        { 'u', "unescaped",      opt_t::none,     0},
        { 'q', "quiet",          opt_t::none,     0},
        { 'm', "escape-members", opt_t::none,     0},
        { 'j', "json-array",     opt_t::none,     0},
        { 'v', "version",        opt_t::none,     0},
        { 'h', "help",           opt_t::none,     0},
    };

    option_parser opt (argc, argv);
    while (int id=opt(options)) {
        switch (id) {
        case 'r':
            args.strict_parsing = false;
            break;

        case 'p':
            try {
                args.ptr = opt.optarg ();
            }
            catch (...) {
                cerr << "Error: Invalid JSON pointer" << endl;
                exit (1);
            }
            break;

        case 'c':
            args.print_pretty = false;
            break;

        case 's':
            args.print_sorted = true;
            break;

        case 'e':
            args.print_escape_slash = true;
            break;

        case 'a':
            args.print_array_items_on_same_line = false;
            break;

        case 't':
            args.required_type = ujson::str_to_jtype (opt.optarg());
            if (args.required_type == ujson::j_invalid) {
                cerr << "Error: Invalid JSON type in option '--type=TYPE'" << endl;
                exit (1);
            }
            break;

        case 'u':
            args.print_unescaped_string = true;
            break;

        case 'q':
            args.quiet = true;
            break;

        case 'm':
            args.members_escape = true;
            break;

        case 'j':
            args.members_as_json_array = true;
            break;

        case 'v':
            std::cout << prog_name << ' ' << UJSON_VERSION_STRING << std::endl;
            exit (0);
            break;

        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;

        case -2:
            cerr << "Missing argument to option '" << opt.opt() << "'" << endl;
            exit (1);
            break;

        default:
            cerr << "Unknown option: '" << opt.opt() << "'" << endl;
            exit (1);
        }
    }


    auto& arguments = opt.arguments ();
    auto arg_entry = arguments.begin ();
    if (arg_entry == arguments.end()) {
        cerr << "Error: Missing command (-h for help)" << endl;
        exit (1);
    }

    args.cmd = *arg_entry;
    while (++arg_entry != arguments.end())
        args.args.emplace_back (*arg_entry);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static ujson::jvalue get_instance (const string& filename,
                                   const bool strict_parsing,
                                   const ujson::jpointer& ptr,
                                   const bool quiet=false)
{
    ujson::jparser parser;
    ujson::jvalue document = parser.parse_file (filename, strict_parsing);
    if (document.invalid()) {
        if (!quiet)
            cerr << "Parse error: " << parser.error() << endl;
        return ujson::jvalue (ujson::j_invalid);
    }

    auto& instance = ujson::find_jvalue (document, ptr);
    if (instance.invalid()) {
        if (!quiet)
            cerr << "Pointer error: No such item" << endl;
        return ujson::jvalue (ujson::j_invalid);
    }
    return instance;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int cmd_view (appargs_t& opt)
{
    if (opt.args.empty())
        opt.args.emplace_back ("");
    if (opt.args.size() > 1) {
        cerr << "Error: Too many arguments" << endl;
        return 1;
    }
    auto instance = get_instance (opt.args[0],
                                  opt.strict_parsing,
                                  opt.ptr);
    if (instance.invalid())
        return 1;

    if (opt.required_type!=ujson::j_invalid && instance.type()!=opt.required_type) {
        cerr << "Error: Instance is not a JSON " << ujson::jtype_to_str(opt.required_type) << endl;
        return 1;
    }

    if (instance.is_string() && opt.print_unescaped_string) {
        cout << instance.str() << endl;
    }else{
        cout << instance.describe(opt.print_pretty,
                                  opt.print_array_items_on_same_line,
                                  opt.print_sorted,
                                  opt.print_escape_slash)
             << endl;
    }
    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int cmd_verify (appargs_t& opt)
{
    if (opt.args.empty())
        opt.args.emplace_back ("");
    if (opt.args.size() > 1) {
        cerr << "Error: Too many arguments" << endl;
        return 1;
    }

    int retval = 0;
    ujson::jparser parser;
    ujson::jvalue document = parser.parse_file (opt.args[0],
                                                opt.strict_parsing);
    if (document.invalid()) {
        if (!opt.quiet)
            cerr << "Parse error: " << parser.error() << endl;
        retval = 1;
    }
    else if (!opt.quiet) {
        cout << "Ok" << endl;
    }
    return retval;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int cmd_type (appargs_t& opt)
{
    if (opt.args.empty())
        opt.args.emplace_back ("");
    if (opt.args.size() > 1) {
        cerr << "Error: Too many arguments" << endl;
        return 1;
    }
    auto instance = get_instance (opt.args[0],
                                  opt.strict_parsing,
                                  opt.ptr,
                                  opt.quiet);
    if (instance.invalid())
        return 1;

    int retval = 0;
    if (opt.required_type != ujson::j_invalid) {
        if (instance.type() == opt.required_type)
            retval = 0;
        else
            retval = 1;
        if (!opt.quiet)
            cout << (retval==0 ? "Yes" : "No") << endl;
    }else{
        cout << ujson::jtype_to_str(instance) << endl;
    }

    return retval;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int cmd_size (appargs_t& opt)
{
    if (opt.args.empty())
        opt.args.emplace_back ("");
    if (opt.args.size() > 1) {
        cerr << "Error: Too many arguments" << endl;
        return 1;
    }
    auto instance = get_instance (opt.args[0],
                                  opt.strict_parsing,
                                  opt.ptr);
    if (instance.invalid())
        return 1;

    if (!instance.is_container()) {
        cerr << "Error: Instance is not an array or an object" << endl;
        return 1;
    }

    cout << instance.size() << endl;
    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int cmd_members (appargs_t& opt)
{
    if (opt.args.empty())
        opt.args.emplace_back ("");
    if (opt.args.size() > 1) {
        cerr << "Error: Too many arguments" << endl;
        return 1;
    }
    auto instance = get_instance (opt.args[0],
                                  opt.strict_parsing,
                                  opt.ptr);
    if (instance.invalid())
        return 1;

    if (instance.is_object() == false) {
        cerr << "Error: Instance is not a JSON object" << endl;
        return 1;
    }

    ujson::json_object& jobj = instance.obj ();
    ujson::jvalue result_array (ujson::j_array);

    for (auto attrib = opt.print_sorted ? jobj.sbegin() : jobj.begin();
         attrib != (opt.print_sorted ? jobj.send() : jobj.end());
         ++attrib)
    {
        if (opt.members_as_json_array) {
            result_array.append (attrib->first);
        }else{
            if (opt.members_escape)
                cout << "\"" << ujson::escape(attrib->first) << "\"" << endl;
            else
                cout << attrib->first << endl;
        }
    }

    if (opt.members_as_json_array) {
        cout << result_array.describe(opt.print_pretty,
                                      opt.print_array_items_on_same_line,
                                      opt.print_sorted,
                                      opt.print_escape_slash)
             << endl;
    }

    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int cmd_patch (appargs_t& opt)
{
    if (opt.args.empty()) {
        cerr << "Error: Missing input file" << endl;
        return 1;
    }
    auto instance = get_instance (opt.args[0],
                                  opt.strict_parsing,
                                  opt.ptr);
    if (instance.invalid())
        return 1;

    // Read and parse patch file
    //
    if (opt.args.size() < 2)
        opt.args.emplace_back ("");
    ujson::jparser parser;
    ujson::jvalue patch = parser.parse_file (opt.args[1], opt.strict_parsing);
    if (patch.invalid()) {
        cerr << "Patch definition parse error: " << parser.error() << endl;
        return 1;
    }

    // Patch the instance
    //
    auto result = ujson::patch (instance, patch);

    bool only_test_ops = false;
    if (opt.quiet != true) {
        auto num_patches = result.second.size ();
        for (unsigned i=0; i<num_patches; ++i) {
            if (result.second[i] == ujson::patch_ok)
                continue;
            cerr << "Patch " << (i+1) << " of " << num_patches << " - ";
            switch (result.second[i]) {
            case ujson::patch_fail:
                cerr << "Test operation failed" << endl;
                break;
            case ujson::patch_invalid:
                cerr << "Error: Invalid patch definition" << endl;
                break;
            case ujson::patch_noent:
                cerr << "Error: JSON pointer mismatch" << endl;
                break;
            default:
                cerr << "Unknown error" << endl;
                break;
            }
        }
    }else{
        only_test_ops = true;
        if (patch.is_array()) {
            // Array of patches
            for (auto& p : patch.array()) {
                auto& op = p.get ("op");
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
    if (!opt.quiet || !only_test_ops) {
        cout << instance.describe(opt.print_pretty,
                                  opt.print_array_items_on_same_line,
                                  opt.print_sorted,
                                  opt.print_escape_slash)
             << endl;
    }

    return result.first ? 0 : 1;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    static const map<const string, int(*const)(appargs_t&)> commands = {
        {"members", cmd_members},
        {"patch",   cmd_patch},
        {"size",    cmd_size},
        {"type",    cmd_type},
        {"verify",  cmd_verify},
        {"view",    cmd_view},
    };

    appargs_t opt;
    parse_args (argc, argv, opt);

    int retval = 1;
    auto cmd = commands.find (opt.cmd);
    if (cmd == commands.end())
        cerr << "Error: unknown command (-h for help)" << endl;
    else
        retval = cmd->second (opt);

    return retval;
}
