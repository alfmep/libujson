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
static void show_item_in_document (ujson::jvalue& instance, const string& pointer)
{
    auto& value = ujson::find_jvalue (instance, pointer);
    if (value.valid())
        cout << pointer << ": " << value.describe() << endl;
    else
        cout << "Error not found: " << pointer << endl;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    auto instance = ujson::Json().parse_string (document);

    cout << "Document:" << endl;
    cout << instance.describe(true) << endl;
    cout << endl;

    show_item_in_document (instance, "");
    show_item_in_document (instance, "/number");
    show_item_in_document (instance, "/text");
    show_item_in_document (instance, "/array/2");
    show_item_in_document (instance, "/array/0");
    show_item_in_document (instance, "/noent");
    show_item_in_document (instance, "/array/4");
    show_item_in_document (instance, "/");

    return 0;
}
