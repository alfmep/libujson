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
#include <numeric>
#include <random>
#include <ujson.hpp>


using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    //----------------------------------
    // Construct a jvalue representing an empty JSON array
    //
    ujson::jvalue val (ujson::j_array);
    cout << val.describe() << endl << endl;

    // Check the JSON type
    //
    if (val.type() == ujson::j_array)
        cout << "'val' represents a JSON array" << endl << endl;

    // Append values to the array using method ujson::jvalue::append()
    //
    val.append (42);      // Append a JSON number
    val.append ("str");   // Append a JSON string
    val.append (true);    // Append a JSON boolean
    val.append (nullptr); // Append a JSON null
    cout << val.describe(ujson::fmt_pretty) << endl << endl;

    // Append values to the array using STL
    {
        ujson::json_array& jarray = val.array ();
        jarray.emplace_back (76);              // Append a JSON number
        jarray.emplace_back ("hello");         // Append a JSON string
        jarray.emplace_back (ujson::j_object); // Append an empty JSON object
        jarray.emplace_back (false);           // Append a JSON boolean
        cout << val.describe(ujson::fmt_pretty) << endl << endl;
    }

    // Remove element at index 2 from the array using method ujson::jvalue.remove()
    //
    val.remove (2);
    cout << val.describe(ujson::fmt_pretty) << endl << endl;

    // Remove index 4 from the array using an STL iterator
    {
        ujson::json_array& jarray = val.array ();
        auto i = jarray.begin ();
        i += 4;
        jarray.erase (i);
        cout << val.describe(ujson::fmt_pretty) << endl << endl;
    }

    // Fill the array with random numbers between 10 and 99.
    //
    mt19937 rng (42); // Static seed to make this app predictable
    uniform_int_distribution<int> dice (10, 99);
    for (auto& element : val.array())
        element = dice (rng);
    cout << val.describe() << endl << endl;

    // Sort the array
    // (here we know the array contains only numbers,
    //  it may not make much sense to sort an array
    //  with elements of different JSON types.)
    std::sort (val.array().begin(), val.array().end());
    cout << val.describe() << endl << endl;


    //----------------------------------
    // Create an array with 10 elements.
    // The default constructor of ujson::jvalue creates
    // a JSON null instance. So this array will be filled
    // with 10 JSON null values.
    //
    ujson::jvalue a1 (ujson::json_array(10));
    cout << a1.describe(ujson::fmt_pretty) << endl << endl;

    // The elements in a JSON arrays doesn't have to be of the same type
    //
    a1[0] = 0;
    a1[1] = "A string";
    a1[2] = true;
    cout << a1.describe(ujson::fmt_pretty) << endl << endl;


    //----------------------------------
    // Create an array with 10 JSON numbers.
    //
    ujson::jvalue a2 (ujson::json_array(10, ujson::jvalue(ujson::j_number)));
    cout << a2.describe() << endl << endl;

    // Fill the array with a number sequence using std::iota
    //
    std::iota (a2.array().begin(), a2.array().end(), 0);
    cout << a2.describe() << endl << endl;


    //----------------------------------
    // Create an array with 5 JSON strings initialized to "element".
    //
    ujson::jvalue a3 (ujson::json_array(5, ujson::jvalue("element")));
    cout << a3.describe(ujson::fmt_pretty) << endl << endl;


    // Append one JSON array to another
    //
    a3.array().insert (a3.array().end(),
                       a2.array().begin(),
                       a2.array().end());
    cout << a3.describe(ujson::fmt_pretty) << endl << endl;


    // Calculate the mean value of all the numbers in an array
    //
    {
        double sum = 0;
        size_t n = 0;
        for (auto& item : a3.array()) {
            if (item.type() == ujson::j_number) {
                sum += item.num ();
                ++n;
            }
        }
        cout << "Mean value for " << n
             << " JSON numbers in an array with " << a3.size()
             << " entries: " << (sum/(double)n) << endl
             << endl;
    }


    //----------------------------------
    // Create a JSON array containing different JSON types
    //
    ujson::jvalue a4 ({
            42,
            nullptr,
            "Some text",
            ujson::json_array({0,1,2,3,4}),
            false,
            {{{"one",1},{"two",2}}}
        });
    cout << a4.describe(ujson::fmt_pretty) << endl << endl;


    // Iterate over a JSON array
    //
    for (auto& element : a4.array())
        cout << "JSON type: "    << ujson::jtype_to_str(element)
             << " \tContainer: " << (element.is_container()?"yes":"no")
             << " \tValue: "     << element.describe()
             << endl;
    cout << endl;


    cout << endl;
    cout << endl;
    cout << "(Read comments in examples/json-array.cpp)" << endl;
    cout << endl;

    return 0;
}
