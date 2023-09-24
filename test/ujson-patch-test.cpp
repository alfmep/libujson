/*
 * Copyright (C) 2022,2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <fstream>
#include <filesystem>
#include <ujson.hpp>
#include <cstdlib>
#include <unistd.h>
#include "../utils/option-parser.hpp"


using namespace std;
namespace fs = filesystem;
namespace uj = ujson;

static constexpr const char* prog_name = "ujson-patch-test";

struct appdata_t {
    fs::path test_filename;
    fs::path passed_filename;
    fs::path failed_filename;
    fs::path disabled_filename;
    fs::path invalid_filename;

    uj::jvalue test_suite; // Array of patch tests
    uj::jvalue results;
    long index; // Index in test_suite

    bool allow_disabled;
    bool force_overwrite;

    appdata_t () {
        index = 0;
        allow_disabled = false;
        force_overwrite = false;
    }
};


static void run_test (appdata_t& app);
static int handle_result (appdata_t& app);
static void print_usage_and_exit (ostream& out, int exit_code);
static void parse_args (int argc, char* argv[], appdata_t& app);
static int do_files_exist (appdata_t& app);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appdata_t app;
    parse_args (argc, argv, app);

    // Parse the test file
    //
    uj::jparser parser;
    app.test_suite = parser.parse_file (app.test_filename.string());
    if (!app.test_suite.valid()) {
        cerr << "Error: Unable to parse test file: " << parser.error() << endl;
        return 1;
    }
    else if (app.test_suite.type() != uj::j_array) {
        // We expect an array of test cases
        cerr << "Error: Test file not a JSON array" << endl;
        return 1;
    }

    // Initialize test results
    //
    app.results.type (uj::j_object);
    app.results["passed"]   = uj::jvalue (uj::j_array);
    app.results["failed"]   = uj::jvalue (uj::j_array);
    app.results["disabled"] = uj::jvalue (uj::j_array);
    app.results["invalid"]  = uj::jvalue (uj::j_array);

    // Do the testing
    //
    for (app.index=0; app.index < (long)app.test_suite.size(); ++app.index)
        run_test (app);

    // Display result, and possibly create result files
    //
    return handle_result (app);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int handle_result (appdata_t& app)
{
    auto& passed   = app.results.get ("passed");
    auto& failed   = app.results.get ("failed");
    auto& disabled = app.results.get ("disabled");
    auto& invalid  = app.results.get ("invalid");

    cout << "Passed tests   : " << passed.size() << endl;
    cout << "Failed tests   : " << failed.size() << endl;
    cout << "Disabled tests : " << disabled.size();
    if (app.allow_disabled == false)
        cout << " (Use option -a to also run disabled tests)" << endl;
    else
        cout << endl;
    if (invalid.size())
        cout << "Invalid tests  : " << invalid.size() << endl;

    // Write passed test results to file
    //
    if (!app.passed_filename.empty()) {
        ofstream out (app.passed_filename, ios_base::out|ios_base::trunc);
        out << passed.describe(ujson::fmt_pretty) << endl;
        if (out.fail()) {
            cerr << "Error writing file '" << app.passed_filename << "'" << endl;
            return 1;
        }
    }

    // Write failed test results to file
    //
    if (!app.failed_filename.empty()) {
        ofstream out (app.failed_filename, ios_base::out|ios_base::trunc);
        out << failed.describe(ujson::fmt_pretty) << endl;
        if (out.fail()) {
            cerr << "Error writing file '" << app.failed_filename << "'" << endl;
            return 1;
        }
    }

    // Write disabled test results to file
    //
    if (!app.disabled_filename.empty()) {
        ofstream out (app.disabled_filename, ios_base::out|ios_base::trunc);
        out << disabled.describe(ujson::fmt_pretty) << endl;
        if (out.fail()) {
            cerr << "Error writing file '" << app.disabled_filename << "'" << endl;
            return 1;
        }
    }

    // Write invalid test results to file, if any
    //
    if (invalid.size()) {
        if (!app.invalid_filename.empty()) {
            ofstream out (app.invalid_filename, ios_base::out|ios_base::trunc);
            out << invalid.describe(ujson::fmt_pretty) << endl;
            if (out.fail()) {
                cerr << "Error writing file '" << app.invalid_filename << "'" << endl;
                return 1;
            }
        }
    }

    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void run_test (appdata_t& app)
{
    // Initialize result
    uj::jvalue result (uj::j_object);

    result["test_file"] = app.test_filename.string ();
    result["pointer_to_test"] = std::string("/") + std::to_string(app.index);

    try {
        // Get the test case
        uj::jvalue& test = app.test_suite[app.index];

        // See if the test only contains 'comment'
        //
        if (test.size()==1 && test.has("comment")) {
            // Treat it as a disabled test
            app.results["disabled"].append (result);
            return;
        }

        if (app.allow_disabled == false) {
            //
            // Check if the test is disabled
            //
            auto& disabled = test.get ("disabled");
            if (disabled.valid() && disabled.boolean() == true) {
                // Return a disabled result
                app.results["disabled"].append (result);
                return;
            }
        }

        // Get the JSON document and test patch
        //
        auto& doc   = test.get_unique ("doc");
        auto& patch = test.get_unique ("patch");

        // See if the patch contains only test operations
        // If so, the test may lack both 'expected' and 'error'
        //
        bool only_test_patch = true;
        for (auto& item : patch.array()) {
            auto& op = item.get ("op");
            if (op.str() != "test") {
                only_test_patch = false;
                break;
            }
        }

        // Get (possible) expected result
        //
        auto& expected_result = test.get ("expected");

        // Get (possible) expected error
        //
        auto& expected_error = test.get ("error");

        // Sanity check
        //
        if (!only_test_patch && expected_result.invalid() && expected_error.invalid()) {
            // Both "expected" and "error" missing in test definition,
            // and this is not a test only patch.
            // Return an invalid test result
            app.results["invalid"].append (result);
            return;
        }

        // Apply the test patch(es)
        //
        auto patch_result = uj::patch (doc, result["patch_test_result"], patch);

        // Analyze the result
        //
        if (expected_result.valid()) {
            //
            // We expect a successful patch
            //
            if (patch_result.first == true  && // All patches applied (and ok on 'test' operations)
                (only_test_patch || result["patch_test_result"] == expected_result)) // Result as expected
            {
                // Patch successful
                app.results["passed"].append (result);
            }else{
                // Patch failed
                app.results["failed"].append (result);
            }
        }
        else if (expected_error.valid()) {
            //
            // We expect a failed patch
            //
            if (patch_result.first == false) {
                // Patch failed - just as we expected
                app.results["passed"].append (result);
            }else{
                // Patch successful - this we didn't want
                app.results["failed"].append (result);
            }
        }
        else {
            //
            // A test case with no "expected" or "error" attribute, and only "test" operations
            //
            if (patch_result.first == true) {
                // Patch passed the tests
                app.results["passed"].append (result);
            }else{
                // Patch faild tests
                app.results["failed"].append (result);
            }
        }
    }
    catch (std::exception& e) {
        // Patch test caused an exception - treat is as an invalid test case
        cerr << "Exception at \"" << result["pointer_to_test"].str() << "\": " << e.what() << endl;
        app.results["invalid"].append (result);
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void print_usage_and_exit (ostream& out, int exit_code)
{
    out << endl;
    out << "Usage: " << prog_name << " [OPTION] <json-patch-test-file>" << endl;
    out << "    Use libujson to test JSON patches in the file containing JSON patch test definitions." << endl;
    out << "    The format of the JSON patch test file is described here:" << endl;
    out << "    https://github.com/json-patch/json-patch-tests" << endl;
    out << endl;
    out << "    Options:" << endl;
    out << "        -a,--allow-disabled         Perform a patch test even if it is marked as disabled." << endl;
    out << "        -s,--passed-doc=FILENAME    Write a JSON document containing info about passed tests." << endl;
    out << "        -f,--failed-doc=FILENAME    Write a JSON document containing info about failed tests." << endl;
    out << "        -d,--disabled-doc=FILENAME  Write a JSON document containing info about disabled tests." << endl;
    out << "        -i,--invalid-doc=FILENAME   Write a JSON document containing info about invalid test cases." << endl;
    out << "        -o,--force-overwrite        If any of the above files already exists, overwrite them." << endl;
    out << "        -v,--version                Print version and exit." << endl;
    out << "        -h,--help                   Print this help and exit." << endl;
    out << endl;

    exit (exit_code);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_args (int argc, char* argv[], appdata_t& app)
{
    optlist_t options = {
        {'a', "allow-disabled",  opt_t::none,     0},
        {'s', "passed-doc",      opt_t::required, 0},
        {'f', "failed-doc",      opt_t::required, 0},
        {'d', "disabled-doc",    opt_t::required, 0},
        {'i', "invalid-doc",     opt_t::required, 0},
        {'o', "force-overwrite", opt_t::none,     0},
        {'v', "version",         opt_t::none,     0},
        {'h', "help",            opt_t::none,     0},
    };

    option_parser opt (argc, argv);
    while (int id=opt(options)) {
        switch (id) {
        case 'a':
            app.allow_disabled = true;
            break;

        case 's':
            app.passed_filename = opt.optarg ();
            break;

        case 'f':
            app.failed_filename = opt.optarg ();
            break;

        case 'd':
            app.disabled_filename = opt.optarg ();
            break;

        case 'i':
            app.invalid_filename = opt.optarg ();
            break;

        case 'o':
            app.force_overwrite = true;
            break;

        case 'v':
            std::cout << prog_name << ' ' << UJSON_VERSION_STRING << std::endl;
            exit (0);
            break;

        case 'h':
            print_usage_and_exit (std::cout, 0);
            break;

        case -1:
            cerr << "Unknown option: '" << opt.opt() << "'" << endl;
            exit (1);
            break;

        case -2:
            cerr << "Missing argument to option '" << opt.opt() << "'" << endl;
            exit (1);
            break;
        }
    }

    // Collect arguments that are not options
    auto& arguments = opt.arguments ();
    switch (arguments.size()) {
    case 0:
        cerr << "Missing argument (--help for help)" << endl;
        exit (1);
        break;
    case 1:
        app.test_filename = arguments[0];
        break;
    default:
        cerr << "Too many arguments (--help for help)" << endl;
        exit (1);
    }

    // Check if the result files already exists
    //
    if (!app.force_overwrite) {
        if (do_files_exist(app))
            exit (1);
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static int do_files_exist (appdata_t& app)
{
    if (!app.passed_filename.empty()) {
        if (fs::exists(app.passed_filename)) {
            cerr << "Error: file " << app.passed_filename
                 << " already exists. Use option --force-overwrite to overwrite." << endl;
            return 1;
        }
    }

    if (!app.failed_filename.empty()) {
        if (fs::exists(app.failed_filename)) {
            cerr << "Error: file " << app.failed_filename
                 << " already exists. Use option --force-overwrite to overwrite." << endl;
            return 1;
        }
    }

    if (!app.disabled_filename.empty()) {
        if (fs::exists(app.disabled_filename)) {
            cerr << "Error: file " << app.disabled_filename
                 << " already exists. Use option --force-overwrite to overwrite." << endl;
            return 1;
        }
    }

    if (!app.invalid_filename.empty()) {
        if (fs::exists(app.invalid_filename)) {
            cerr << "Error: file " << app.invalid_filename
                 << " already exists. Use option --force-overwrite to overwrite." << endl;
            return 1;
        }
    }

    return 0;
}
