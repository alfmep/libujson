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
#include <ujson.hpp>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void show_value (const ujson::jvalue& value)
{
    cout << "JSON " << ujson::jtype_to_str(value) << ":"
         << endl << value.describe(ujson::fmt_pretty) << endl << endl;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    //
    // Constructor examples
    //

    // The default constructor will create a JSON null value.
    ujson::jvalue j0;
    show_value (j0);

    // Construct a jvalue representing a JSON number
    ujson::jvalue j1 (42);
    show_value (j1);

    // Construct a jvalue representing a JSON boolean
    ujson::jvalue j2 (true);
    show_value (j2);

    // Construct a jvalue representing a JSON null
    ujson::jvalue j3 (nullptr);
    show_value (j3);

    // Construct a jvalue representing a JSON string
    ujson::jvalue j4 ("Hello World!");
    show_value (j4);

    // Construct a jvalue representing a JSON object
    ujson::jvalue j5 ({
            {"a", 0},
            {"b", 1}
        });
    show_value (j5);

    // Construct a jvalue representing a JSON array
    ujson::jvalue j6 (ujson::json_array{0, 1, 2});
    show_value (j6);

    // Construct a jvalue representing a JSON object with different types of properties
    ujson::jvalue j7 ({
            {"name", "The name"},
            {"value", 42},
            {"on", false},
            {"list", {{"first", "second", "third"}}},
            {"empty", nullptr},
            {"node", {{{"name", "Another name"},
                       {"value", 32}}}},
        });
    show_value (j7);


    //
    // Assignment examples
    //
    ujson::jvalue val;

    // Assign a JSON number
    // This will make 'val' represent a JSON number with the assigned value
    val = 1.5;
    show_value (val);

    // Assign a JSON boolean
    // This will make 'val' represent a JSON boolean with the assigned value
    val = false;
    show_value (val);

    // Assign a JSON null
    // This will make 'val' represent a JSON null
    val = nullptr;
    show_value (val);

    // Assign a JSON string
    // This will make 'val' represent a JSON string with the assigned value
    val = "A string value";
    show_value (val);

    // Assign a JSON object
    // This will make 'val' represent a JSON object with the assigned properties
    val = {{"name", "Object name"},
           {"value", 27},
           {"on", true}};
    show_value (val);

    // Assign a JSON array
    // This will make 'val' represent a JSON array with the assigned values
    val = {"first", "second", "third", nullptr, 42, true};
    show_value (val);

    // Assign a JSON array with only numbers
    // This will make 'val' represent a JSON array with the assigned values
    val = ujson::json_array {0,1,2,3};
    show_value (val);


    cout << endl;

    // We can check what JSON type a ujson::jvalue currently represents
    //
    switch (val.type()) {
    case ujson::j_invalid:
        cout << "'val' is an invalid JSON type" << endl;
        break;

    case ujson::j_object:
        cout << "'val' is a JSON object" << endl;
        break;

    case ujson::j_array:
        cout << "'val' is a JSON array" << endl;
        break;

    case ujson::j_string:
        cout << "'val' is a JSON string" << endl;
        break;

    case ujson::j_number:
        cout << "'val' is a JSON number" << endl;
        break;

    case ujson::j_bool:
        cout << "'val' is a JSON boolean" << endl;
        break;

    case ujson::j_null:
        cout << "'val' is a JSON null" << endl;
        break;
    }


    cout << endl;
    cout << endl;
    cout << "(Read comments in examples/initialize-jvalue.cpp)" << endl;
    cout << endl;

    return 0;
}
