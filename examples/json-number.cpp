/*
 * Copyright (C) 2022 Dan Arrhenius <dan@ultramarin.se>
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
#include <ujson.hpp>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    // Create a JSON number with its default value (0)
    //
    ujson::jvalue n1 (ujson::j_number);
    cout << n1.describe() << endl << endl;


    // Create a JSON number with a specific value
    //
    ujson::jvalue n2 (3.14);
    cout << n2.describe() << endl << endl;


    // Assign a number to a jvalue
    //
    n2 = 42;
    cout << n2.describe() << endl << endl;


    // Convert a JSON value to a number regardless
    // of what type it currently represents.
    //
    ujson::jvalue jval ("A JSON string");
    cout << jval.describe() << endl;
    jval = 3;  // jval was a JSON string, now it is a JSON number
    cout << jval.describe() << endl << endl;


    // Get the number value from a ujson::jvalue instance
    //
    double val = n2.num ();
    cout << val << endl << endl;


    // Increase the number by 18
    //
    n2 = n2.num() + 18;
    cout << n2.describe() << endl << endl;


    // Method jvalue::num() only works on JSON numbers
    //
    ujson::jvalue jval2 (true); // jval2 is a JSON boolean, and not a number
    try {
        cout << "Try calling num() on a JSON " << ujson::jtype_to_str(jval2) << endl;
        cout << jval2.num() << endl;
    }
    catch (ujson::json_type_error& jte) {
        cout << "Error using num(): " << jte.what() << endl << endl;
    }



#if UJSON_HAVE_GMPXX
    //
    // Numbers with high precision
    //

    // Create a number with high precision
    //
    ujson::jvalue jnum (42.000000000000004_mpf);
    cout << "jnum.describe(): " << jnum.describe() << endl << endl;

    // Using method num() returns a double and will possible lose precision
    //
    cout << "jnum.num(): " << jnum.num() << endl << endl;

    // Using method jvalue::mpf() will get a reference to the number with full precision
    //
    mpf_class& n = jnum.mpf ();
    // Do some calculation
    n *= 4;
    cout << "jnum.num()     : " << jnum.num() << endl; // Here we lose precision
    cout << "jnum.describe(): " << jnum.describe() << endl << endl;
#endif


    cout << endl;
    cout << endl;
    cout << "(Read comments in examples/json-number.cpp)" << endl;
    cout << endl;

    return 0;
}
