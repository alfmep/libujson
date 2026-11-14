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
#include <algorithm>
#include <ujson.hpp>

using namespace std;

#define PRINT_EXAMPLE_HEADER cout << endl << "==== Example at examples/json-object.cpp:" << __LINE__ << endl

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    //----------------------------------
    // Construct a jvalue representing an empty JSON object
    //
    ujson::jvalue val (ujson::j_object);
    PRINT_EXAMPLE_HEADER;
    cout << val.describe(ujson::fmt_pretty) << endl;

    //----------------------------------
    // Check the JSON type
    PRINT_EXAMPLE_HEADER;
    if (val.type() == ujson::j_object)
        cout << "val represents a JSON object" << endl; // Yes, we already knew that

    //----------------------------------
    // Use operator[] to create attributes in the object
    //
    PRINT_EXAMPLE_HEADER;
    val["descr"]  = "Object name"; // Add a string
    val["answer"] = 42;            // Add a number
    val["on_off"] = true;          // Add a boolean
    cout << val.describe(ujson::fmt_pretty) << endl;

    //----------------------------------
    // Use method add() to create an attribute in the object
    //
    PRINT_EXAMPLE_HEADER;
    val.add ("newval", "Added with add()"); // Add a string
    cout << val.describe(ujson::fmt_pretty) << endl;

    //----------------------------------
    // Method add() will not add a duplicate but will
    // by default change an already added attribute
    PRINT_EXAMPLE_HEADER;
    val.add ("newval", "Changed attribute value with add()");
    cout << val.describe(ujson::fmt_pretty) << endl;

    //----------------------------------
    // Method add() can be told to not
    // change an already existing attribute (if one exist)
    PRINT_EXAMPLE_HEADER;
    val.add ("newval", "Added an attribute with the same name using add()", false);
    cout << val.describe(ujson::fmt_pretty) << endl;

    //----------------------------------
    // If using add when there are more than one attribute
    // with the same name, then the value of the last added
    // attribute is modified
    PRINT_EXAMPLE_HEADER;
    val.add ("newval", "Using add() to overwrite when multiple \"newval\" exists");
    cout << val.describe(ujson::fmt_pretty) << endl;

    //----------------------------------
    // Use operator[] to change a value, and the type of JSON value, in the object
    //
    PRINT_EXAMPLE_HEADER;
    val["on_off"] = "button"; // The attribute was a JSON boolean, it is now a JSON string
    cout << val.describe(ujson::fmt_pretty) << endl;

    //----------------------------------
    // Use operator[] to get a value in the object
    {
        PRINT_EXAMPLE_HEADER;
        auto& item = val["answer"];
        cout << "Value of 'answer' using operator[](const string&): " << item.describe() << endl;
    }

    //----------------------------------
    // Use operator[] to get a value _not_ in the object
    //
    // This _creates_ an object attribute named 'noent' with JSON type null
    {
        PRINT_EXAMPLE_HEADER;
        auto& item = val["noent"];
        cout << "Value of 'noent' using operator[](const string&): " << item.describe(ujson::fmt_pretty) << endl;
    }

    //----------------------------------
    // Use get() to get a value in the object (we already know the value exist)
    {
        PRINT_EXAMPLE_HEADER;
        auto& item = val.get ("answer");
        if (item.valid())
            cout << "Value of 'answer' using method get(): " << item.describe(ujson::fmt_pretty) << endl;
    }

    //----------------------------------
    // Use get() to get a value _not_ in the object
    //
    // This returns an _invalid_ ujson::jvalue (the type() method will return ujson::j_invalid)
    // This invalid jvalue is not added to the object and can't be used for anything useful.
    {
        PRINT_EXAMPLE_HEADER;
        auto& item = val.get ("not_in_object");
        // We can be check if the value was found or not
        if (item.valid())
            cout << "Attribute 'not_in_object' was found in the object" << endl;
        else
            cout << "Attribute 'not_in_object' wasn't found in the object" << endl; // This is printed
    }

    //----------------------------------
    // Check if the object has an attribute with a specific name
    //
    PRINT_EXAMPLE_HEADER;
    if (val.has("descr"))
        cout << "The JSON object has an attribute named 'descr'" << endl;
    else
        cout << "The JSON object doesn't have an attribute named 'descr'" << endl;

    if (val.has("no_way"))
        cout << "The JSON object has an attribute named 'no_way'" << endl;
    else
        cout << "The JSON object doesn't have an attribute named 'no_way'" << endl;


    //----------------------------------
    // Use operator[](const string&) on a ujson::jvalue that is _not_ of type ujson::j_object
    //
    PRINT_EXAMPLE_HEADER;
    ujson::jvalue number (64);
    cout << "number: " << number.describe() << endl;
    cout << "number is a JSON " << ujson::jtype_to_str(number.type()) << endl;

    // Making object manipulations on a JSON number will not go too well
    PRINT_EXAMPLE_HEADER;
    try {
        cout << "Try to access number[\"name\"] when number is not a JSON object" << endl;
        number["name"] = "A number";
        cout << "number: " << number.describe() << endl;
    }
    catch (ujson::json_type_error& jte) {
        cout << "Exception ujson::json_type_error: " << jte.what() << endl;
    }

    PRINT_EXAMPLE_HEADER;
    try {
        cout << "Try to access number.get(\"name\") when number is not a JSON object" << endl;
        number.get("name") = "A number";
        cout << "number: " << number.describe() << endl;
    }
    catch (ujson::json_type_error& jte) {
        cout << "Exception ujson::json_type_error: " << jte.what() << endl;
    }

    // The type of JSON value a jvalue represents can easily be checked:
    PRINT_EXAMPLE_HEADER;
    if (number.type() == ujson::j_object)
        cout << "number _is_ a JSON object" << endl;
    else
        cout << "number is _not_ a JSON object" << endl; // This will be printed

    // But the jvalue can of course be assigned a JSON object and turn from a j_number into a j_object:
    PRINT_EXAMPLE_HEADER;
    number = {{"desc", "number is now a JSON object and not a JSON number"},
              {"value", 64}};
    cout << "number: " << number.describe(ujson::fmt_pretty) << endl;
    cout << "number is a JSON " << ujson::jtype_to_str(number.type()) << endl;



    //----------------------------------
    // Iterate on the object attributes
    //

    // Simplify the code below by getting a reference to the JSON object value
    // A JSON object value is represented by a ujson::multimap_list.
    auto& jobj = val.obj ();

    // Here, jobj is of type ujson::multimap_list which is a combination
    // of std::multimap and std::list in order to get the efficiency of
    // an std::multimap and still keep the natural insertion order when using
    // begin() and end().
    //
    // The choice of multimap instead of map was made since the
    // JSON specification allows multiple object attributes with
    // the same name.

    // A normal for-loop
    PRINT_EXAMPLE_HEADER;
    cout << "normal for-loop (iterates in natural order):" << endl;
    for (auto& attribute : jobj)
        cout << "\tname: " << attribute.first << ", \tvalue: " << attribute.second.describe() << endl;

    // A for-loop in natural order using begin() and end()
    PRINT_EXAMPLE_HEADER;
    cout << "for-loop using begin() and end() (natural order):" << endl;
    for (auto item=jobj.begin(); item!=jobj.end(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;

    // A for-loop in reversed natural order using rbegin() and rend()
    PRINT_EXAMPLE_HEADER;
    cout << "for-loop using rbegin() and rend() (reversed natural order):" << endl;
    for (auto item=jobj.rbegin(); item!=jobj.rend(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;

    // A for-loop in sorted order using sbegin() and send()
    PRINT_EXAMPLE_HEADER;
    cout << "for-loop using sbegin() and send() (sorted order):" << endl;
    for (auto item=jobj.sbegin(); item!=jobj.send(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;

    // A for-loop in reversed sorted order using rsbegin() and rsend()
    PRINT_EXAMPLE_HEADER;
    cout << "for-loop using rsbegin() and rsend() (reversed sorted order):" << endl;
    for (auto item=jobj.rsbegin(); item!=jobj.rsend(); ++item)
        cout << "\tname: " << item->first << ", \tvalue: " << item->second.describe() << endl;

    //
    // An example of using std::for_each to iterate on the object attributes
    //
    auto print_attrib = [](const ujson::json_pair& attribute) {
        cout << "\tname: " << attribute.first << ", \tvalue: " << attribute.second.describe() << endl;
    };
    // Natural order
    PRINT_EXAMPLE_HEADER;
    cout << "std::for_each in natural order:" << endl;
    std::for_each (jobj.begin(), jobj.end(), print_attrib);

    // Reversed natural order
    PRINT_EXAMPLE_HEADER;
    cout << "std::for_each in reversed natural order:" << endl;
    std::for_each (jobj.rbegin(), jobj.rend(), print_attrib);

    // Sorted order
    PRINT_EXAMPLE_HEADER;
    cout << "std::for_each in sorted order:" << endl;
    std::for_each (jobj.sbegin(), jobj.send(), print_attrib);

    // Reversed sorted order
    PRINT_EXAMPLE_HEADER;
    cout << "std::for_each in reversed sorted order:" << endl;
    std::for_each (jobj.rsbegin(), jobj.rsend(), print_attrib);


    //----------------------------------
    // Multiple attributes with the same name
    //
    // The JSON specification allows multiple attributes with the same name.
    // We don't encourage this, but if you _REALLY_ want to add multiple
    // attributes with the same name, you can do the following:
    //
    PRINT_EXAMPLE_HEADER;
    val.obj().emplace_back ("descr", ujson::jvalue("Another object attribute with the same name"));
    cout << val.describe(ujson::fmt_pretty) << endl;

    // When getting an attribute from an object, it will be the
    // _last_ attribute with that name that is returned.
    {
        PRINT_EXAMPLE_HEADER;
        auto& attrib_value = val.get ("descr");
        cout << "Attribute 'descr': " << attrib_value.describe() << endl;
    }

    // (advanced example that is probably never used in real life)
    //
    // So, putting an attribute with the same name as another at the front
    // will make it invisible when calling method get().
    // This is because method get() will return the _last_ attribute
    // with a given name in the list of object attributes.
    //
    val.obj().emplace_front ("descr", ujson::jvalue("I am first"));
    {
        PRINT_EXAMPLE_HEADER;
        auto& av = val.get ("descr"); // Won't return the value just added with emplace_front()
        // This will print the value "Another object attribute with the same name"
        cout << "Attribute 'descr': " << av.describe() << endl;
    }

    // You can use equal_range() to find all attributes with the same name:
    //
    PRINT_EXAMPLE_HEADER;
    cout << "All attributes named 'descr':" << endl;
    auto range = val.obj().equal_range ("descr");
    for (auto i=range.first; i!=range.second; ++i) {
        cout << "\tname: " << i->first << ", value: " << i->second.describe() << endl;
    }


    //----------------------------------
    // Remove attributes from an object
    //
    // If an object attribute is removed using method jvalue::remove(),
    // then all (if more than one) attributes with the specified name are removed
    //
    PRINT_EXAMPLE_HEADER;
    cout << "Object before removing attribute 'descr':" << endl;
    cout << val.describe(ujson::fmt_pretty) << endl;
    // Remove attribute 'descr'
    val.remove ("descr");
    //
    cout << "Object after removing attribute 'descr':" << endl;
    cout << val.describe(ujson::fmt_pretty) << endl;

    // Accessing the ujson::json_object instance to remove all attributes
    val.obj().clear ();
    //
    cout << "Object after removing all attributes:" << endl;
    cout << val.describe(ujson::fmt_pretty) << endl;


    //----------------------------------
    //
    // Compare JSON objects
    //
    {
        //
        // Compare two equal JSON objects
        //
        PRINT_EXAMPLE_HEADER;
        ujson::jvalue obj1 = {{
                {"value", 42},
                {"descr", "A JSON objet"},
            }};
        ujson::jvalue obj2 = {{
                {"value", 42},
                {"descr", "A JSON objet"},
            }};
        if (obj1 == obj2)
            cout << "obj1 and obj2 are equal" << endl; // The objects are equal
        else
            cout << "obj1 and obj2 are not equal" << endl;
    }
    {
        //
        // Compare two not equal JSON objects
        //
        PRINT_EXAMPLE_HEADER;
        ujson::jvalue obj1 = {{
                {"value", 42},
                {"descr", "A JSON objet"},
            }};
        ujson::jvalue obj2 = {{
                {"value", 64},
                {"descr", "Another JSON objet"},
            }};
        if (obj1 == obj2)
            cout << "obj1 and obj2 are equal" << endl;
        else
            cout << "obj1 and obj2 are not equal" << endl; // The objects are not equal
    }
    {
        //
        // Compare two JSON objects with the same
        // attributes but not in the same natural order.
        //
        // The JSON objects are considered equal if they have the
        // same attributes with the same values, even if the attributes
        // are not in the same natural order.
        //
        // (there is an exception to this is the object has multiple
        //  attributes with the same name. see below.)
        //
        PRINT_EXAMPLE_HEADER;
        ujson::jvalue obj1 = {{
                {"value", 42},
                {"boolean", true},
                {"descr", "A JSON objet"},
            }};
        ujson::jvalue obj2 = {{
                {"descr", "A JSON objet"},
                {"value", 42},
                {"boolean", true},
            }};
        if (obj1 == obj2)
            cout << "obj1 and obj2 are equal" << endl; // The objects are not equal
        else
            cout << "obj1 and obj2 are not equal" << endl;
    }
    {
        //
        // Compare two JSON objects with duplicate
        // attributes, and all attributes are in
        // the same natural order
        //
        PRINT_EXAMPLE_HEADER;
        ujson::jvalue obj1 = {{
                {"value", 42},
                {"boolean", true},
                {"descr", "A JSON objet"},
                {"descr", "This is a duplicate attribute"},
            }};
        ujson::jvalue obj2 = {{
                {"value", 42},
                {"boolean", true},
                {"descr", "A JSON objet"},
                {"descr", "This is a duplicate attribute"},
            }};
        if (obj1 == obj2)
            cout << "obj1 and obj2 are equal" << endl; // The objects are equal
        else
            cout << "obj1 and obj2 are not equal" << endl;
    }
    {
        //
        // Compare two JSON objects with duplicate
        // attributes where the duplicate attributes
        // are in the same order but the other
        // attributes are not.
        //
        PRINT_EXAMPLE_HEADER;
        ujson::jvalue obj1 = {{
                {"value", 42},
                {"boolean", true},
                {"descr", "A JSON objet"},
                {"descr", "This is a duplicate attribute"},
            }};
        ujson::jvalue obj2 = {{
                {"descr", "A JSON objet"},
                {"boolean", true},
                {"descr", "This is a duplicate attribute"},
                {"value", 42},
            }};
        if (obj1 == obj2)
            cout << "obj1 and obj2 are equal" << endl; // The objects are equal
        else
            cout << "obj1 and obj2 are not equal" << endl;
    }
    {
        //
        // Compare two JSON objects with duplicate
        // attributes where the duplicate attributes
        // are not in the same order
        //
        // This is an odd case and the objects are considered not equal since
        // the JSON object is represented by a ujson::multimap_list which in turn
        // is base on a std::multimap.
        // It is also a good thing that they are considered not equal since the
        // method get("descr") wouldn't return the same attribute value for the
        // two objects below.
        //
        PRINT_EXAMPLE_HEADER;
        ujson::jvalue obj1 = {{
                {"value", 42},
                {"boolean", true},
                {"descr", "A JSON objet"},
                {"descr", "This is a duplicate attribute"},
            }};
        ujson::jvalue obj2 = {{
                {"value", 42},
                {"boolean", true},
                {"descr", "This is a duplicate attribute"},
                {"descr", "A JSON objet"},
            }};
        if (obj1 == obj2)
            cout << "obj1 and obj2 are equal" << endl;
        else
            cout << "obj1 and obj2 are not equal" << endl; // The objects are not equal
    }


    cout << endl;
    cout << endl;
    cout << "(Read comments in examples/json-object.cpp)" << endl;
    cout << endl;

    return 0;
}
