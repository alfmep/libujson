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
#include <iostream>
#include <ujson.hpp>

using namespace std;


static constexpr const char* document = "{"
    "    \"number\": 16,"
    "    \"text\":   \"Hello World!\","
    "    \"array\":  [true, \"some text\", 42],"
    "    \"\":       \"Hidden attribute\""
    "}";


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void find_item_in_document (ujson::jvalue& instance, const string& pointer)
{
    cout << "Find value using JSON pointer: " << (pointer.empty()?std::string("\"\""):pointer) << endl;

    try {
        // Find the value using the JSON pointer
        auto& value = ujson::find_jvalue (instance, pointer);

        // Check if the value was found
        if (value.valid())
            cout << "Found value: " << value.describe() << endl;
        else
            cout << "Nothing found, pointer doesn't point to a value in the JSON document." << endl;
        cout << endl;
    }
    catch (std::invalid_argument& ia) {
        cout << "Error: " << ia.what() << endl;
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    auto instance = ujson::jparser().parse_string (document);

    cout << "JSON Document:" << endl;
    cout << instance.describe(ujson::fmt_pretty) << endl;
    cout << endl;

    // Find item using JSON pointer: ""
    // This should find the whole JSON instance, since "" points to the root.
    //
    find_item_in_document (instance, "");

    // Find item using JSON pointer: /number
    // This should find the number 16.
    //
    find_item_in_document (instance, "/number");

    // Find item using JSON pointer: /text
    // This should find the string "Hello World!".
    //
    find_item_in_document (instance, "/text");

    // Find item using JSON pointer: /array/2
    // This should find the number 42.
    //
    find_item_in_document (instance, "/array/2");

    // Find item using JSON pointer: /array/0
    // This should find the boolean true.
    //
    find_item_in_document (instance, "/array/0");

    // Find item using JSON pointer: /noent
    // This should fail to find anything.
    //
    find_item_in_document (instance, "/noent");

    // Find item using JSON pointer: /array/4
    // This should fail to find anything.
    //
    find_item_in_document (instance, "/array/4");

    // Find item using JSON pointer: /
    // This should find the string "Hidden attribute" since it refers to an empty oject member name (/"").
    //
    find_item_in_document (instance, "/");

    // Try to find an item using an invalid JSON pointer
    // This should fail to find anything.
    //
    find_item_in_document (instance, "invalid_pointer");

    cout << endl;
    cout << endl;
    cout << "(Read comments in examples/find_jvalue.cpp)" << endl;
    cout << endl;

    return 0;
}
