/*
 * Copyright (C) 2023,2026 Dan Arrhenius <dan@ultramarin.se>
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
#include <iomanip>
#include <ios>


using namespace std;
using namespace ujson::parser;


static const char* token_err_to_str (jtoken::error_t code)
{
    switch (code) {
    case jtoken::ok:
        return "Ok";

    case jtoken::err_string:
        return "Invalid string";

    case jtoken::err_string_unterminated:
        return "Unterminated string";

    case jtoken::err_string_escape:
        return "Invalid escape code in string.";

    case jtoken::err_string_utf8:
        return "Invalid UTF8 character in string.";

    case jtoken::err_number:
        return "Invalid number";

    case jtoken::err_number_lone_minus:
        return "Expecting number after '-'";

    case jtoken::err_number_no_frac:
        return "Expecting number after '.'";

    case jtoken::err_number_no_exp:
        return "Missing exponent in number";

    case jtoken::err_invalid:
        return "Invalid token";

    case jtoken::err_unexpected_char:
        return "Unexpected character";

    case jtoken::err_eob:
        return "Unexpected end of buffer";

    default:
        return "(unknown)";
    }
}



int main (int argc, char* argv[])
{
    if (argc < 2) {
        cerr << "Usage: jtokens [-s,--strict] <json-file>" << endl;
        return 1;
    }

    bool strict = false;
    bool count_only = false;
    std::string file_name;

    for (int i=1; i<argc; ++i) {
        std::string option = std::string (argv[i]);
        if (option=="-s" || option=="--strict") {
            strict = true;
        }
        else if (option=="-c" || option=="--count") {
            count_only = true;
        }
        else if (file_name.empty()) {
            file_name = option;
        }
        else {
            cerr << "Usage: jtokens [-s,--strict] [-c,--count] <json-file>" << endl;
            return 1;
        }
    }
    if (file_name.empty()) {
        cerr << "Usage: jtokens [-s,--strict] [-c,--count] <json-file>" << endl;
        return 1;
    }

    ifstream in (file_name);
    if (!in.good()) {
        cerr << "Error opening file." << endl;
        return 1;
    }
    std::string json_doc ((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close ();


    jtokenizer tokenizer (json_doc, strict);
    const jtoken* token;
    unsigned long count = 0;
    bool got_error = false;

    while ((token = tokenizer.next_token()) != nullptr) {
        if (count_only  &&  token->type != jtoken::tk_invalid) {
            ++count;
        }else{
            cout << "Token: " << left << setw(10) << jtoken_type_to_string(token->type)
                 << " size: " << setw(2) << right << token->data.size()
                 << ", at (" << setw(2) << right << (token->row+1) << ',' << setw(3) << right << token->col << ')';
            cout << ",\t data: ==>" << token->data << "<==";
            if (token->type == jtoken::tk_invalid) {
                cout << " error: " << token_err_to_str(token->err_code);
                got_error = true;
            }
            cout << endl;
        }
    }
    if (count_only)
        cout << count << endl;

    return got_error ? 1 : 0;
}
