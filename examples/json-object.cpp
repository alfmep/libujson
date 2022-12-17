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
#include <algorithm>
#include <ujson.hpp>

using namespace std;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    //----------------------------------
    // Construct a jvalue representing an empty JSON object
    //
    ujson::jvalue val (ujson::j_object);
    cout << val.describe(true) << endl << endl;

    //----------------------------------
    // Check the JSON type
    if (val.type() == ujson::j_object)
        cout << "val represents a JSON object" << endl << endl; // Yes, we already knew that

    //----------------------------------
    // Use operator[] to create attributes in the object
    //
    val["descr"]  = "Object name";
    val["answer"] = 42;
    val["on_off"] = true;
    cout << val.describe(true) << endl << endl;

    //----------------------------------
    // Use method add() to create an attribute in the object
    //
    val.add ("newval", "Added with add()");
    cout << val.describe(true) << endl << endl;

    //----------------------------------
    // Method add() will not add a duplicate but will
    // by default change an already added attribute
    val.add ("newval", "No duplicate but changed value");
    cout << val.describe(true) << endl << endl;

    //----------------------------------
    // Method add() can be told to not
    // change an already added attribute (if one exist)
    val.add ("newval", "Don't change the value again", false);
    cout << val.describe(true) << endl << endl;

    //----------------------------------
    // Use operator[] to change a value, and the type of JSON value, in the object
    //
    val["on_off"] = "button"; // The attribute was a JSON boolean, it is now a JSON string
    cout << val.describe(true) << endl << endl;

    //----------------------------------
    // Use operator[] to get a value in the object
    {
        auto& item = val["answer"];
        cout << "Value of 'answer' using operator[](const string&): " << item.describe() << endl << endl;
    }

    //----------------------------------
    // Use operator[] to get a value _not_ in the object
    //
    // This _creates_ an object attribute named 'noent' with JSON type null
    {
        auto& item = val["noent"];
        cout << "Value of 'noent' using operator[](const string&): " << item.describe(true) << endl << endl;
    }

    //----------------------------------
    // Use get() to get a value in the object (we already know the value exist)
    {
        auto& item = val.get ("answer");
        if (item.valid())
            cout << "Value of 'answer' using method get(): " << item.describe(true) << endl << endl;
    }

    //----------------------------------
    // Use get() to get a value _not_ in the object
    //
    // This returns an _invalid_ ujson::jvalue (the type() method will return ujson::j_invalid)
    // This invalid jvalue is not added to the object and can't be used for anything useful.
    {
        auto& item = val.get ("not_in_object");
        // We can be check if the value was found or not
        if (item.valid())
            cout << "Attribute 'not_in_object' was found in the object" << endl;
        else
            cout << "Attribute 'not_in_object' wasn't found in the object" << endl; // This is printed
        cout << endl;
    }

    //----------------------------------
    // Check if the object has an attribute with a specific name
    //
    cout << endl;
    if (val.has("descr"))
        cout << "The JSON object has an attribute named 'descr'" << endl;
    else
        cout << "The JSON object hasn't an attribute named 'descr'" << endl;

    if (val.has("no_way"))
        cout << "The JSON object has an attribute named 'no_way'" << endl;
    else
        cout << "The JSON object hasn't an attribute named 'no_way'" << endl;
    cout << endl;


    //----------------------------------
    // Use operator[](const string&) on a ujson::jvalue that is _not_ of type ujson::j_object
    //
    ujson::jvalue number (64);
    cout << "number: " << number.describe() << endl;
    cout << "number is a JSON " << ujson::jtype_to_str(number.type()) << endl;

    // Making object manipulations on a JSON number will not go too well
    try {
        cout << "Try to access number[\"name\"]" << endl;
        number["name"] = "A number";
        cout << "number: " << number.describe() << endl;
    }
    catch (std::logic_error& le) {
        cout << "Error accessing object attribute 'name': " << le.what() << endl;
    }

    // The type of JSON value a jvalue represents can easily be checked:
    if (number.type() == ujson::j_object)
        cout << "number _is_ a JSON object" << endl;
    else
        cout << "number is _not_ a JSON object" << endl; // This will be printed
    cout << endl;

    // But the jvalue can of course be assigned a JSON object and turn from a j_number into a j_object:
    number = {{"desc", "number is now a JSON object and not a JSON number"},
              {"value", 64}};
    cout << "number: " << number.describe(true) << endl;
    cout << "number is a JSON " << ujson::jtype_to_str(number.type()) << endl;
    cout << endl;



    //----------------------------------
    // Iterate on the object attributes
    //
    auto& jobj = val.obj (); // Simplify the code below by getting a reference to the JSON object value

    // Here, jobj is of type ujson::multimap_list which is a combination
    // of std::multimap and std::list in order to get the efficiency of
    // an std::multimap and still keep the natural insertion order when using
    // begin() and end().
    //
    // The choice of multimap instead of map was made since the
    // JSON specification allows multiple object attributes with
    // the same name.

    // A normal for-loop
    cout << "normal for-loop (iterates in natural order):" << endl;
    for (auto& attribute : jobj)
        cout << "\tname: " << attribute.first << ", \tvalue: " << attribute.second.describe() << endl;
    cout << endl;

    // A for-loop in natural order using begin() and end()
    cout << "for-loop using begin() and end() (natural order):" << endl;
    for (auto item=jobj.begin(); item!=jobj.end(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;
    cout << endl;

    // A for-loop in reversed natural order using rbegin() and rend()
    cout << "for-loop using rbegin() and rend() (reversed natural order):" << endl;
    for (auto item=jobj.rbegin(); item!=jobj.rend(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;
    cout << endl;

    // A for-loop in sorted order using sbegin() and send()
    cout << "for-loop using sbegin() and send() (sorted order):" << endl;
    for (auto item=jobj.sbegin(); item!=jobj.send(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;
    cout << endl;

    // A for-loop in reversed sorted order using rsbegin() and rsend()
    cout << "for-loop using rsbegin() and rsend() (reversed sorted order):" << endl;
    for (auto item=jobj.rsbegin(); item!=jobj.rsend(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;
    cout << endl;

    cout << endl;

    //
    // An example of using std::for_each to iterate on the object attributes
    //
    auto print_attrib = [](const ujson::json_pair& attribute) {
        cout << "\tname: " << attribute.first << ", \tvalue: " << attribute.second.describe() << endl;
    };
    // Natural order
    cout << "std::for_each in natural order:" << endl;
    std::for_each (jobj.begin(), jobj.end(), print_attrib);
    cout << endl;

    // Reversed natural order
    cout << "std::for_each in reversed natural order:" << endl;
    std::for_each (jobj.rbegin(), jobj.rend(), print_attrib);
    cout << endl;

    // Sorted order
    cout << "std::for_each in sorted order:" << endl;
    std::for_each (jobj.sbegin(), jobj.send(), print_attrib);
    cout << endl;

    // Reversed sorted order
    cout << "std::for_each in reversed sorted order:" << endl;
    std::for_each (jobj.rsbegin(), jobj.rsend(), print_attrib);
    cout << endl;

    cout << endl;


    //----------------------------------
    // Multiple attributes with the same name
    //
    // The JSON specification allows multiple attributes with the same name.
    // We don't encourage this, but if you _REALLY_ want to add multiple
    // attributes with the same name, you can do the following:
    //
    val.obj().emplace_back ("descr", ujson::jvalue("Another object attribute with the same name"));
    cout << val.describe(true) << endl;
    cout << endl;

    // When getting an attribute from an object, it will be the
    // _last_ attribute with that name that is returned.
    {
        auto& attrib_value = val.get ("descr");
        cout << "Attribute 'descr': " << attrib_value.describe() << endl;
        cout << endl;
    }

    // (advanced example that is probably never used in real life)
    //
    // So, putting an attribute with the same name as another at the front
    // will make it invisible when calling method get().
    //
    val.obj().emplace_front ("descr", ujson::jvalue("I am first"));
    {
        auto& av = val.get ("descr"); // Won't return the value just added with emplace_front()
        // This will print the value "Another object attribute with the same name"
        cout << "Attribute 'descr': " << av.describe() << endl;
        cout << endl;
    }

    // You can use equal_range() to find all attributes with the same name:
    //
    cout << "All attributes named 'descr':" << endl;
    auto range = val.obj().equal_range ("descr");
    for (auto i=range.first; i!=range.second; ++i) {
        cout << "\tname: " << i->first << ", value: " << i->second.describe() << endl;
    }

    cout << endl;


    //----------------------------------
    // Remove attributes from an object
    //
    // If an object attribute is removed using method jvalue::remove(),
    // then all (if more than one) attributes with the specified name are removed
    //
    cout << "Object before removing attribute 'descr':" << endl;
    cout << val.describe(true) << endl;
    // Remove attribute 'descr'
    val.remove ("descr");
    //
    cout << "Object after removing attribute 'descr':" << endl;
    cout << val.describe(true) << endl;

    // Accessing the ujson::json_object instance to remove all attributes
    val.obj().clear ();
    //
    cout << "Object after removing all attributes:" << endl;
    cout << val.describe(true) << endl;


    cout << endl;
    cout << endl;
    cout << "(Read comments in examples/json-object.cpp)" << endl;
    cout << endl;

    return 0;
}
