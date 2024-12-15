# libujson - A JSON C++ library and utility applications for handling JSON documents, JSON pointers, JSON patches, and JSON Schema validation.

### Table of contents
- **[Features](#features)**
  - [JSON numbers with arbitrary precision](#json-numbers-with-arbitrary-precision)
  - [Limitations](#limitations)
- **[How to build and install](#how-to-build-and-install)**
- **[Utility applications for handling JSON documents](#utility-applications-for-handling-json-documents)**
  - [ujson-verify](#ujson-verify)
  - [ujson-print](#ujson-print)
  - [ujson-get](#ujson-get)
  - [ujson-patch](#ujson-patch)
  - [ujson-tool](#ujson-tool)
- **[Testing libujson](#testing-libujson)**
  - [Testing JSON parsing in libujson](#testing-json-parsing-in-libujson)
  - [Testing JSON patch support in libujson](#testing-json-patch-support-in-libujson)
  - [Testing JSON Schema support in libujson](#testing-json-schema-support-in-libujson)
- **[C++ API](#c-api)**
  - [Include file, C++ namespace, and linking](#include-file-c-namespace-and-linking)
  - [Parsing JSON documents](#parsing-json-documents)
  - [JSON instances](#json-instances)
  - [Working with JSON null values](#working-with-json-null-values)
  - [Working with JSON strings](#working-with-json-strings)
  - [Working with JSON numbers](#working-with-json-numbers)
  - [Working with JSON booleans](#working-with-json-booleans)
  - [Working with JSON objects](#working-with-json-objects)
  - [Using JSON pointers](#using-json-pointers)
  - [Using JSON patches](#using-json-patches)
  - [Using JSON Schema for validation](#using-json-schema)


## Features
- Efficient JSON parsing.
- Utility applications for handling JSON documents.
- Supports JSON numbers with arbitrary precision (if built with gmpxx, which is default).
- Simple to use C++ API to parse, create, access, and manage JSON documents and types.
- Use JSON pointers (RFC6901) to access data in JSON documents.
- Patch JSON documents with JSON patches as described in RFC6902.
- Supports JSON Schema validation, JSON schema version 2020-12.
- Test utility to run the JSON patch test cases defined at https://github.com/json-patch/json-patch-tests (if configured with `-DBUILD_TESTS=True`).
- Test utility to run the JSON parsing test cases defined at https://github.com/nst/JSONTestSuite (if configured with `-DBUILD_TESTS=True`).
- Test utility to run the JSON Schema test cases defined at https://github.com/json-schema-org/JSON-Schema-Test-Suite (if configured with `-DBUILD_TESTS=True`).
- Doxygen generated API documentation.
- Support for a "relaxed" format of JSON documents, but uses strict format (RFC8259) as default.
  In relaxed format, the following is allowed in JSON documents:
  - C-style comments.
  - String values can be split up into more than one string separated by whitespace or comments (like in C/C++).
  - Object definitions are allowed to end with a separator (,).
  - Array definitions are allowed to end with a separator (,).
  - In object definitions, an attribute name can be an 'identifier' instead of a string enclosed by double quotes. An identifier is a name with the following format: [_a-zA-Z][_a-zA-Z0-9]*
    Exceptions are: true, false, and null (case insensitive). Those names are reserved and not allowed to be used as identifiers (i.e. without enclosing double quotes).
    For an example of a JSON document in relaxed form, see file 'example-document-in-relaxed-form.json'.

### JSON numbers with arbitrary precision
When building with gmpxx (*default if gmpxx is found by cmake*), libujson supports numbers with arbitrary precision in JSON documents.
An example of a JSON document supported by libujson without losing precision:
```json
{
  "numbers": [
    {
      "name": "e",
      "value": 2.71828182845904523536028747135266249775724709369995957496696762772407663035354759457138217852516642742746
    },
    {
      "name": "pi",
      "value": 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798214
    }
  ]
}
```
### Limitations
  - Only UTF-8 encoded JSON documents are supported (escaped UNICODE characters are of course supported as described in RFC 8259).


## How to build and install

First run:
```
cmake [optional parameters]
```
then:
```
make
make install
```
If cmake finds doxygen, the generated API documentation can be viewed in a browser by opening file `[prefix]/share/doc/libujson/html/index.html`. To disable API documentation generation, use parameter `-DBUILD_DOC=False` when running cmake.

To enable example applications, run cmake with parameter `-DBUILD_EXAMPLES=True`. Example applications are *not* installed when running `make install`.

To enable applications and scripts to test JSON parsing and JSON patches, run cmake with parameter `-DBUILD_TESTS=True`. Test applications are *not* installed when running `make install`. 

If the precision of `double` is just fine for numbers and there's no need for arbitrary precision, but speed is more important; parsing JSON documents will be more efficient if configured without support for gmpxx (`-DDISABLE_GMPXX=True`).

To disable the utility applications and only build the library, run cmake with parameter `-DBUILD_UTILS=False`. The utility applications are built by default if not explicitly disabled.




# Utility applications for handling JSON documents
Unless configured with parameter `-DBUILD_UTILS=False`, the following utilities are built and installed:

- **ujson-verify** - Verify the syntax of one or more JSON documents.
- **ujson-print** - Print a JSON document to standard output in a few different ways.
- **ujson-get** - Get a specific value from a JSON document using a JSON pointer. JSON pointers are described in RFC 6901.
- **ujson-patch** - Patch JSON documents. JSON patches are described in RFC 6902.
- **ujson-tool** - A utility with several sub-commands to handle JSON documents in a variety of ways.

## ujson-verify
ujson-verify is a utility used for verifying that JSON documents are syntactically correct. And optionally verify the JSON document use a JSON schema. If all the files on the command line are successfully verified, ujson-verify exits with code 0. If any file fails verification, ujson-verify exits with code 1. If no file name is given, a JSON document is read from standard input.

**Synopsis:**

**ujson-verify [OPTIONS] [FILE ...]**

**Options:**

**-q, --quiet**		Silent mode, don't write anything to standard output.

**-c, --schema=SCHEMA_FILE**	Verify the JSON document using a JSON schema file. This option may be set multiple times. The first schema file is the main schema used to validate the JSON document. More schema files can then be added that can be referenced by the main and other schema files.

**-d, --verbose**	Verbose mode. Print verbose schema verification output.

**-s, --strict**	Parse JSON documents in strict mode.

**-n, --no-duplicates**	Don't allow objects with duplicate member names.

**--max-depth=DEPTH**   Set maximum nesting depth. Both objects and arrays increases the nesting depth. A value of 0 means no limit. Default is no limit.

**--max-asize=ITEMS**   Set the maximum allowed number of elements in a single JSON array. A value of 0 means no limit. Default is no limit.

**--max-osize=ITEMS**   Set the maximum allowed number of members in a single JSON object. A value of 0 means no limit. Default is no limit.

**-v, --version**	Print version and exit.

**-h, --help**		Print help and exit.


## ujson-print
ujson-print parses a JSON document and prints it to standard output. By default, ujson-print parses the JSON document in relaxed mode and prints it in strict mode. The document is also by default printed in a readable format. This tool can be used to convert JSON documents in relaxed form to strict form, to sort object members, and to make a JSON document more compact or more readable.

**Synopsis:**

**ujson-print [OPTIONS] [FILE]**

**Options:**

**-c, --compact** Compact output, print the JSON document without whitespaces.

**-e, --escape-slash** When printing JSON strings, forward slash characters("/") are escaped to "\\/".

**-t, --sort** Object members are printed in sorted order, not in natural order. Sorting is made on the member name.

**-a, --array-lines** For JSON arrays, print each array item on the same line. Ignored if option '-c,--compact' is used.

**-b, --tabs** Indent using tab characters instead of spaces. Ignored if option '-c,--compact' is used.

**-r, --relaxed** Print the JSON document in relaxed form. Object member names are printed without enclosing them in double quotes("") when the object member names are in the following format: [_a-zA-Z][_a-zA-Z0-9]*. The exceptions are the object member names "true", "false", and "null". Those object member names are always enclosed by double quotes("").

**-s, --strict** Parse the JSON document in strict mode.

**-n, --no-duplicates**	Don't allow objects with duplicate member names.

**-m, --multi-doc** Parse multiple JSON instances. The input is treated as a stream of JSON  instances, separated by line breaks.

**-o, --color** Print in color if the output is to a tty.

**-v, --version** Print version and exit.

**-h, --help** Print help and exit.


## ujson-get
ujson-get prints value in a JSON document pointed to by a JSON pointer. If the JSON document is parsed correctly and the value specified by the pointer is found, ujson-get prints the value (in JSON format) and exits with code 0. If not found, or on parse error, an error message is printed to standard error and the exit code is 1.

**Synopsis:**

**ujson-get [OPTIONS] [FILE] [POINTER]**

**Options:**

**-c, --compact** If the JSON value is an object or an array, print it without whitespace.

**-t, --type=TYPE** Require the found value to be of a specific type. TYPE can be one of the following: boolean, number, string, null, object, or array. If the found value is of a different type, exit with code 1.

**-u, --unescape** If the resulting value is a JSON string, print it as an unescaped string witout enclosing double quotes. Note that this will make the output an invalid JSON document.

**-s, --strict** Parse the JSON document in strict mode.

**-n, --no-duplicates**	Don't allow objects with duplicate member names.

**-o, --color** Print in color if the output is to a tty.

**-v, --version** Print version and exit.

**-h, --help** Print help and exit.


## ujson-patch
ujson-patch applies JSON patches as described by RFC 6902 and prints the resulting JSON document to standard output. If all patches are successful, the application exits with code 0. If any patch is unsuccessful, an error for each unsuccessful patch is written to standard error and the application exits with code 1.
If no patch file is given, the JSON patch definition is read from standard input.

**Synopsis:**

**ujson-patch [OPTIONS] JSON_FILE [JSON_PATCH_FILE]**

**Options:**

**-c, --compact** Print the resulting JSON document without whitespaces.

**-s, --strict** Parse JSON input files in strict mode.

**-n, --no-duplicates**	Don't allow objects with duplicate member names.

**-q, --quiet** No errors are written to standard error. On errors, or failed patch test operations, the application exits with code 1. If the patch definition only contains patch operations of type 'test', nothing is written to standard output. If the patch definition contains operations other than 'test', the resulting JSON document is still printed to standard output.

**-v, --version** Print version and exit.

**-h, --help** Print help and exit.


## ujson-tool
ujson-tool is a utility with several sub-commands to handle JSON documents in a variety of ways.

**Synopsis:**

**ujson-tool COMMAND [OPTIONS] [COMMAND_ARGUMENTS ...]**

All commands, except 'patch', reads a JSON document from standard input if no file is supplied.

**Common options:**

**-s, --strict** Parse JSON documents in strict mode.

**-n, --no-duplicates**	Don't allow objects with duplicate member names.

**-p, --pointer=POINTER** Use the JSON instance pointed to by the JSON pointer instead of the root of the input JSON document.

**-c, --compact** Any Resulting JSON output is printed without whitespaces.

**-e, --escape-slash** In any resulting JSON string output, forward slash characters("/") are escaped to "\\/".

**-a, --array-lines** In any resulting JSON output, print each array item on the same line.

**-o, --color** Print resulting JSON in color, if the output is to a tty.

**--sort** Any Resulting JSON output is printed with object members sorted by name.

**--max-depth=DEPTH**  Set maximum nesting depth.

**--max-asize=ITEMS**  Set the maximum allowed number of elements in a single JSON array.

**--max-osize=ITEMS**  Set the maximum allowed number of members in a single JSON object.

**-v, --version** Print version and exit.

**-h, --help** Print help and exit.


**Commands:**

### view [OPTIONS] [JSON_DOCUMENT]
Print the JSON instance to standard output.

**Options:**

**-t, --type=TYPE** Require that the viewed instance is of a specific JSON type. If the resulting instance is of another type, an error message is printed to standard error and 1 is returned. Valid types are: object, array, string, number, boolean, and null.

**-u, --unescape** Only if the resulting instance is a JSON string: print the string value, unescaped witout enclosing double quotes.

*Example - View a JSON document in a compact form without whitespaces:*
`ujson-tool view --compact document.json`

*Example - View a specific item in a JSON document using a JSON pointer:*
`ujson-tool view --pointer=/members/42/name document.json`

*Example - Sort object members when viewing a JSON document:*
`ujson-tool view --sort document.json`


### type [OPTIONS] [JSON_DOCUMENT]
Print or check the JSON type of the instance.
Default is to write the JSON type of the instance to standard output. But if option `--type=TYPE` is used, the command will check if the type of JSON instance matches the specified JSON type, and return 0 on succes and 1 on failure.

**Options:**

**-t, --type=TYPE** Check if the JSON instance is of a specific JSON type.
If so, print 'Yes' to standard output and return 0.
If not, print 'No' to standard output and return 1.
Valid JSON types are: object, array, string, number, boolean, and null.

**-q, --quiet** Only if option `--type=TYPE` is used: don't print anything, only return 0 on success and 1 on failure.

*Example - Print the name of the JSON type at index 42 in a JSON document containing an array;*
`ujson-tool type --pointer=/42 items.json`

*Example - Check that a JSON documents is a JSON object:*
```shell
    if ujson-tool type -t object -q document.json; then
        echo "The document is a JSON object"
    fi
```

### size [OPTIONS] [JSON_DOCUMENT]
Print the number of elements/members to standard output if the JSON instance is an *array* or *object*.
If the JSON instance isn't an array or object, an error message is printed to standard error and 1 is returned.

*Note:* It is not a recursive count. It is only the number of elements/members in the specified array/object, not including sub-items of the array/object.

*Example - Print the number of members in a JSON document containing an object:*
`ujson-tool size document.json`

*Example - Print the number of items in an array at a specific location in a JSON document:*
`ujson-tool size --pointer=/drawer/boxes document.json`


### members [OPTIONS] [JSON_DOCUMENT]
If the instance is a JSON *object*, print the object member names to standard output on separate lines.
If not a JSON object, print an error message to standard error and return 1.
Note that the member names are by default printed as unescaped string values, and a single member name can thus be printed on multiple lines if it contains one or more line breaks.

**Options:**

**-s, --sort** Sort the member names.

**-m, --escape-members** Print the member names as JSON formatted strings.
The names are printed JSON escaped, enclosed by double quotes. This will ensure that no member name is written on multiple lines since newline characters are escaped.
This option is not needed if option `--json-array` is used.

**-j, --json-array** Print the member names as a JSON formatted array. Option `--escape-members` is implied by this option.

*Example - Print the object member names in a JSON document:*
`ujson-tool members document.json`

*Example - Print the object member names of a JSON object at a specific location in a JSON document:*
`ujson-tool members --pointer=/drawer/boxes/42 document.json`

*Example - Create a JSON array containing the member names of a JSON document in sorted order:*
`ujson-tool members --json-array --sort document.json >member-names.json`


### patch [OPTIONS] JSON_DOCUMENT [JSON_PATCH_FILE]
Patch a JSON instance and print the result to standard output.
If option `--pointer=...` is used, the patch definition uses this position in
the input JSON document as the instance to patch, and the resulting output will
also be from this position. If no patch file is supplied, the patch definition
is read from standard input. Errors and failed patch operations are printed to
standard error. Returns 0 if all patches are successfully applied, and 1 if not.

**Options:**

**-q, --quiet** Don't print failed patch operations to standard error,
only return 1. Also, if all patch operations are of type 'test', don't
print the resulting JSON document to standard output.


### verify [OPTIONS] [JSON_DOCUMENT]
Verify the syntax of a JSON document.

Prints "Ok" to standard output and return 0 if the input is a valid JSON document.
Prints an error message to standard error and return 1 if the input isn't valid JSON document.
Common option `--pointer=POINTER` is ignored by this command.

**Options:**

**--schema=SCHEMA_FILE** Validate the JSON document using a JSON Schema. This option may be set multiple times. The first schema file is the main schema used to validate the JSON document. More schema files can then be added that can be referenced by the main and other schema files.

**-q, --quiet** Print nothing, only return 0 on success, and 1 on error.

**-d, --debug** Print verbose schema validation information. This option is ignored if option --quiet is set.

*Example - Verify that a file is indeed a JSON document:*
`ujson-tool verify document.json`

*Example - Validate a JSON document using a JSON schema:*
`ujson-tool verify --schema schema.json document.json`



# Testing libujson
If libujson is configured with option `-DBUILD_TESTS=True`, then test applications and test scripts are created to test JSON parsing and JSON patches using the test suites at https://github.com/nst/JSONTestSuite and  https://github.com/json-patch/json-patch-tests.

### Testing JSON parsing in libujson
In directory `test`, there is a script named `run-ujson-parse-test.sh` that makes a clone of project https://github.com/nst/JSONTestSuite, applies a patch to include testing libujson, and runs the tests.
When the test script is finished, the result is found in directory `test/result-parse-test`, see file `test/result-parse-test/parsing.html`.
For all options, run `run-ujson-parse-test.sh --help`

### Testing JSON patch support in libujson
If libujson was configured with parameter `-DBUILD_TESTS=True`, then a test application (`ujson-patch-test`) is built in directory `test` that can be used to test the JSON patch support in libujson. There is also a script named `run-ujson-patch-test.sh` to automate fetching test cases and run the test.

To download the test cases from https://github.com/json-patch/json-patch-tests and run all tests, do the following:
```shell
cd test
./run-ujson-patch-test.sh
```
This will (at the time of writing) give the following result:
```
# 
# Clone git repository https://github.com/json-patch/json-patch-tests.git
# 
Cloning into 'test-data/json-patch-tests'...
remote: Enumerating objects: 231, done.
remote: Counting objects: 100% (1/1), done.
remote: Total 231 (delta 0), reused 0 (delta 0), pack-reused 230
Receiving objects: 100% (231/231), 49.76 KiB | 999.00 KiB/s, done.
Resolving deltas: 100% (113/113), done.
# 
# Copy test file to 'result-patch-test/tests.json'
# 
cp test-data/json-patch-tests/tests.json result-patch-test/tests.json
# 
# Run JSON patch test application:
# ./ujson-patch-test -o -s result-patch-test/passed.json -f result-patch-test/failed.json -d result-patch-test/disabled.json -i result-patch-test/invalid.json result-patch-test/tests.json
# Results stored in directory 'result-patch-test'
# 
Passed tests   : 90
Failed tests   : 0
Disabled tests : 3
```

To see all options of the test script, go to directory test and run: `./run-ujson-patch-test.sh --help`
To see all options of the test application, go to directory test and run: `./ujson-patch-test --help`

### Testing JSON Schema in libujson
In directory `test`, there is a script named `run-ujson-schema-test.sh` that makes a clone of project https://github.com/json-schema-org/JSON-Schema-Test-Suite.git, and runs the tests.
When the test script is finished, the result is found in directory `test/result-schema-test`, see file `test/result-schema-test/test-schema-result.txt`.



# C++ API
*(This section is a work in progress.)*

This section contains some tutorial examples of how to use the libujson C++ API.
For a full reference of the API, see the doxygen documentation generated when building libujson. 


## Include file, C++ namespace, and linking

### Include file
For using the libujson API, the header file `ujson.hpp` needs to be included:
```c++
#include <ujson.hpp>
```

### C++ namespace
All types, classes, and functions in libujson are defined within namespace `ujson`.

### Linking

**Automatically link dependent libraries**

Use `pkg-config` to automatically get the linker flags used to link applications using libujson:
```bash
g++ -Wall -O2 -o application application.cpp `pkg-config --libs ujson`
```

**Manually link dependent libraries**

If libujson is configured *with* support for numbers with arbitrary precision (default if gmpxx is found by the configure script), applications using libujson will need to link libraries libujson, libgmpxx, and libgmp:
```bash
g++ -Wall -O2 -o application application.cpp -lujson -lgmpxx -lgmp
```
If libujson is configured *without* support for numbers with arbitrary precision (`-DDISABLE_GMPXX=True`), applications using libujson will only need to link library libujson:
```bash
g++ -Wall -O2 -o application application.cpp -lujson
```


## Parsing JSON documents
To parse JSON documents, use class `ujson::jparser`.

Example of parsing a file, here named document.json:
```c++
ujson::jparser p;
ujson::jvalue val = p.parse_file ("document.json");
```
or, to parse a string containing a JSON document:
```c++
ujson::jvalue val = p.parse_string (str);
```
If the parsing was successful, a valid JSON instance is returned. If there was an error, an invalid JSON instance is returned and an error description can be obtained with the `jparser::error()` method:
```c++
if (val.invalid()) {
    std::cerr << "Parse error: " << p.error() << std::endl;
}
```

## JSON instances
The class `ujson::jvalue` represents a JSON instance and is the central class in libujson.

#### JSON types
The class `ujson::jvalue` represents a JSON value that can be of one of the six JSON types:
- object
- array
- string
- number
- boolean
- null

To see what type it currently represents, use method `jvalue::type()`, it returns an enum (`ujson::jvalue_type`) with one of the following values:
|value|description|
|---|---|
|`ujson::j_invalid`|An invalid JSON type. This indicates an error, for example when failing to parse a JSON document.|
|`ujson::j_object`|A JSON object.|
|`ujson::j_array`|A JSON array.|
|`ujson::j_string`|A JSON string.|
|`ujson::j_number`|A JSON number.|
|`ujson::j_bool`|A JSON boolean.|
|`ujson::j_null`|A JSON null value.|

#### Print a ujson::jvalue
To print a JSON instance, use method `ujson::describe()`, it returns the JSON instance as a string:
```c++
ujson::jvalue val = ujson::jparser().parse_file ("document.json");
std::cout << val.describe() << std::endl;
```
or, to print in a more readable format, use parameter `true`:
```c++
std::cout << val.describe(true) << std::endl;
```
The method `jvalue::describe()` will *always* return the JSON instance as a string that is a valid JSON document that can be parsed successfully. This means that if the instance is a single JSON string, it will be JSON escaped where needed and enclosed by double quotes. Should the instance represent a JSON string and we want the actual unescaped string content, use method `jvalue::str()` instead.


### Assigning values to a ujson::jvalue
When assigning a value to an instance of class `ujson::jvalue`, it may change the type of JSON value it represents. If, for example, an instance of ujson::jvalue represents a JSON string and is assigned a number, it then represents a JSON number instead of a JSON string.
*Example:*
```c++
// Initially, create a JSON string:
ujson::jvalue val = "A JSON string";

// This will print: "val is of type string"
std::cout << "val is of type: << ujson::jtype_to_str(val) << std::endl;

// Now change val to represent a JSON number instead:
val = 42;

// This will print: "val is of type number"
std::cout << "val is of type: << ujson::jtype_to_str(val) << std::endl;
```
### Accessing values of a specific type in a ujson::jvalue
Since an instance of class `ujson::jvalue` can represent any JSON type, there are methods to access the data that the instance currently represents. For example, use method `jvalue::str()` to access the string data when the instance represents a JSON string, and method `jvalue::append()` to append an item when the instance represents a JSON array. To see what JSON type an instance of class `ujson::jvalue` currently represents, use method `jvalue::type()`, or use one of the methods `jvalue::is_number()`, `jvalue::is_string()`, `jvalue::is_boolean()`, `jvalue::is_object()`, `jvalue::is_array()`, or `jvalue::is_null()`.
*Example:*
```c++
// Using method type():
if (val.type() == ujson::j_string) {
    std::cout << "String content: " << val.str() << std::endl;
}
// or method is_string():
if (val.is_string()) {
    std::cout << "String content: " << val.str() << std::endl;
}
```
Using a method in class `ujson::jvalue` to access data of another JSON type that the instance currently represents will cause an exception to be thrown: `ujson::json_type_error`.
*Example:*
```c++
ujson::jvalue val ("Initially a JSON string"); // This creates a JSON string

// Now we change val to represent a JSON number instead:
val = 42; // val is now a JSON number

try {
    // use val as a JSON string:
    auto len = val.str().size();
    std::cout << "Size of string: " << len << std::endl;
}
catch (ujson::json_type_error& jte) {
    // This exception is caught since val is a JSON number, but used as a JSON string
    std::cerr << "Error getting string size: " << jte.what() << std::endl;
}
```


## Working with JSON null values
The default constructor of class `ujson::jvalue` will create a JSON null value.

**Construct a JSON null value:**
```c++
ujson::jvalue val; // The default constructor creates a JSON null value.
```
or, be specific:
```c++
ujson::jvalue val (ujson::j_null);
```
or, use the C++ pointer literal to create a JSON null value:
```c++
ujson::jvalue val (nullptr);
```
**Assigning a JSON null value to a ujson::value:**
```c++
val = nullptr;            // val is now a JSON null value
val.type (ujson::j_null); // Use method type() instead of a direct assignment
```
**Check if an instance of ujson::jvalue is a JSON null:**
```c++
// Use method is_null() to check for a JSON null
if (val.is_null()) {
    cout << "val is a JSON null value" << std::endl;
}

// or, use method type() to check for a JSON null
if (val.type() == ujson::j_null) {
    cout << "val is a JSON null value" << std::endl;
}

// or, use the C++ pointer literal to check for a JSON null
if (val == nullptr) {
    cout << "val is a JSON null value" << std::endl;
}
```


## Working with JSON strings
**Construct a JSON string:**
```c++
ujson::jvalue val (ujson::j_string); // Default is an empty string
```
or, initialize it directly with a value:
```c++
ujson::jvalue val ("Hello World!");
```
**Assigning a string to a ujson::value:**
```c++
val = "A string";   // val is now a JSON string
val.str ("Hello!"); // Method str() can be used instead of a direct assignment
```
**Getting the value of a JSON string**

To get the string value, use the `jvalue::str()` method without parameter:
```c++
std::string& s = val.str ();
s = "Hello World!";  // The string value can be manipulated directly with the reference 's'
```
**Making sure an instance of ujson::jvalue represents a JSON string**

Use method `jvalue::type()` to see if the ujson::jvalue instance represents a JSON string:
```c++
if (val.type() == ujson::j_string) {
    std::cout << "String value: " << val.str();
}
```
or use method `jvalue::is_string()`:
```c++
if (val.is_string()) {
    std::cout << "String value: " << val.str();
}
```

### Escaping/unescaping string values
When parsing a JSON document, the parser will unescape any escape sequence in the string before assigning the string value to the ujson::jvalue instance. All methods in class ujson::jvalue that operates on strings will assume the strings to be unescaped, and will neither escape nor unescape them. The only exception is method `jvalue::describe()` that will print strings escaped when needed and enclosed by double quotes, since that method always returns a valid JSON document.

#### Utility functions to escape/unescape strings
Most of the times there is no need to escape or unescape JSON strings since the parser and method jvalue::describe() takes care of it for us. But there are utility functions used to escape and unescape strings if the need arises:
```c++
// A string that has a newline character that will be JSON escaped in the returned string
std::string escaped_str = ujson::escape("Line One\nLine two.");
```
and:
```c++
// A string with a JSON escaped newline that will be unescaped in the returned string:
std::string unescaped_str = ujson::unescape("Line One\\nLine two.");
```


## Working with JSON numbers

**Construct a JSON number:**
```c++
ujson::jvalue val (ujson::j_number); // Default value is 0
```
or, initialize it directly with a value:
```c++
ujson::jvalue val (42);
```
**Assign a number to a ujson::jvalue:**
```c++
ujson::jvalue val;  // val is a JSON null value by default
val = 42;           // val is now a JSON number
val.num (3.1415);   // Method num() can be used instead of a direct assignment
```
**Getting the value of a JSON number**

To get the number value, use method `jvalud::num()` without parameter:
```c++
double n = val.num ();
std::cout << "Number value: " << n << std::endl;
```
### Numbers with arbitrary precision
When built with support for arbitrary number precision (requires gmpxx), method `jvalue::num()` will still return a `double` and may loose precision. To keep precision, use method `jvalue::mpf()`. This method returns a reference to an object of type `mpf_class` that can be used to calculate with arbitrary precision. Documentation on how to use `mpf_class` is out of scope for this documentation. See https://gmplib.org/manual/index for more info.

For more examples of how to use JSON numbers, see file examples/json-number.cpp


## Working with JSON booleans
**Construct a JSON boolean:**
```c++
ujson::jvalue val (ujson::j_bool); // Default value is false
```
or, initialize it directly with a value:
```c++
ujson::jvalue val (true);
```
**Assigning a boolean to a ujson::value:**
```c++
val = true;          // val is now a JSON boolean
val.boolean (false); // Method boolean() can be used instead of a direct assignment
```
**Getting the value of a JSON boolean**

To get the boolean value, use the `boolean()` method without parameter:
```c++
bool b = val.boolean ();
```


## Working with JSON arrays
A JSON array is by class `ujson::jvalue` internally represented by class `ujson::json_array`, an alias for class `std::vector<ujson::jvalue>`.

**Construct a JSON array:**
```c++
ujson::jvalue val (ujson::j_array); // Default value is an empty JSON array
```
or, initialize it directly with a value:
```c++
ujson::jvalue val ({  // A JSON array with:
        "Some text",  // Index 0: a JSON string
        42,           // Index 1: a JSON number
        true          // Index 2: a JSON boolean
    });
```
**Assigning an array to a ujson::value:**
```c++
val = ujson::json_array{0,1,2};        // val is now a JSON array
val.array (ujson::json_array{0,1,2});  // Method array() can be used instead of a direct assignment
```
**Modifying a JSON array**

Access an individual element in the array:
```c++
val[1] = "A string value";  // Item at index 1 in the array is now a JSON string
```
Append a value to the array:
```c++
val.append (true); // Append a JSON boolean to the array
```
Using the underlying `ujson::json_array` to access and modify the array:
```c++
ujson::json_array& array = val.array (); // Get a reference to the array object
std::cout << "A JSON array with " << array.size() << " items" << std::endl;

// Iterate on the array
for (ujson::jvalue& item : array) {
    std::cout << "Item: " << item.describe() << std::endl;
}

// Use STL iterators
for (auto i=array.begin(); i!=array.end(); ++i) {
    std::cout << "Item: " << i->describe() << std::endl;
}
```
For more examples of how to use JSON arrays, see file examples/json-array.cpp


## Working with JSON objects
A JSON object is by class `ujson::jvalue` internally represented by class `ujson::json_object`, an alias for class `ujson::multimap_list<std::string, ujson::jvalue>`. ujson::multimap_list is a combination of a std::list and std::multimap so that attributes in the object can be accessed fast, have multiple attributes with the same name, and keep the natural attribute order when using iterators.

**Construct a JSON object:**
```c++
ujson::jvalue val (ujson::j_object); // Default value is an empty JSON object
```
or, initialize it directly with a value:
```c++
ujson::jvalue val ({
        {"name", "A name"},
        {"value", 42}
    });
```
**Assigning an object to a ujson::value:**
```c++
val = {{"name":"Bob"},{"age":42}};       // val is now a JSON object
val.obj ({{"name":"Alice"},{"age":42}}); // Method obj() can be used instead of a direct assignment
```
**Accessing/modifying attributes in a JSON object**

Check if the JSON object has a specific attribute:
```c++
if (val.has("attribute_name")) {
    std::cout << "The object has attribute 'attribute_name'" << std::endl;
}
```
Add an attribute to the JSON object:
```c++
val.add ("num", 42);  // Adds the attribute 'num' with the JSON number value 42
```
Remove an attribute from the JSON object:
```c++
val.remove ("num");  // Removes the attribute 'num' from the JSON object
```
Get a reference to an attribute value:
```c++
ujson::jvalue& num = val.get ("num");
if (num.valid() == false) {
    std::cerr << "Attribute 'num' doesn't exist in the object" << std::endl;
}
```
***Important:*** The method `jvalue::get()` only returns a reterence to an instance with type `ujson::j_invalid` if the attribute doesn't exists in the JSON object. In this case the return value is a reference to a static ujson::jvalue that will be reset by libujson at any time and shouldn't be modified. So *always* check the return value of method `jvalue::get()`.

**Using operator[] on a JSON object**

The operator [] can be used to both access and add attributes to a JSON object. If the accessed attributes doesn't exist, it will be created with the default constructor (a JSON null value).

Accessing an attribute, creating it if it doesn't exist:
```c++
val["name"] = "Bob";  // Set attribute "name" to the value "Bob", or create it if it doesnt exist
```

**Iterate on attributes**

Use method `jvalue::obj()` to get a reference to the underlying `ujson::j_object` instance. With it, standard STL operations can be used on the JSON object.
```c++
ujson::json_object& o = val.obj ();
```
Iterate on attributes in natural insertion order:
```c++
for (auto& attrib : val.obj()) {
    std::cout << "Attribute name:  " << attrib.first << std::endl;
    std::cout << "Attribute value: " << attrib.second.describe() << std::endl;
}
```
Iterate on attributes in sorted order (sorted on attribute names):
```c++
ujson::json_object& jobj = val.obj ();
for (auto attrib=jobj.sbegin(); attrib!=jobj.send(); ++attrib) {
    std::cout << "Attribute name:  " << attrib->first << std::endl;
    std::cout << "Attribute value: " << attrib->second.describe() << std::endl;
}
```
For more examples of how to use JSON objects, see file examples/json-object.cpp


## Using JSON pointers
JSON pointers are used to access elements in a JSON documents using a string syntax as described in RFC 6901 (https://datatracker.ietf.org/doc/html/rfc6901).
To find an element in a JSON document, the utility function `ujson::find_jvalue()` is used. It takes a JSON instance and a JSON pointer as arguments and returns a reference to the JSON value inside the JSON instance. If the pointer doesn't point to a value inside the JSON instance, a reference to an invalid ujson::jvalue is returned.

***Important:*** If the function `ujson::find_jvalue()` doesn't find the value the JSON pointer points to, a *static invalid* instance of a ujson::jvalue is returned (`jvalue::type()` will return `ujson::j_invalid`). This static value should not be modified and will be reset by libujson at any time. So *always* check the return value of ujson::find_jvalue().

An example of using `ujson::find_jvalue()`:
```c++
ujson::jvalue doc = ujson::jparser().parse_file ("document.json");
ujson::jvalue& name = ujson::find_jvalue (doc, "/house/42/owner/name");

if (name.valid()) {
    std::cout << "Owner is " << name.str() << std::endl;
}else{
    std::cout << "Can't find owner information." << std::endl;
}
```


## Using JSON patches
JSON patches are used to modify JSON instances using one or more operations as described in RFC 6902 (https://datatracker.ietf.org/doc/html/rfc6902).
To use JSON patches to modify a JSON instance, the utility function `ujson::patch()` is used. It takes a JSON instance and a JSON patch definition as arguments and comes in two variants. One that patches the JSON instance in place, and one that supplies a resulting JSON instance and keeps the original intact. The function returns an `std::pair` with `first` being a boolean that is true if all patches were successfully applied, and `second` being a vector with each individual patch result.

An example of using `ujson::patch()`:
```c++
ujson::jparser parser;
ujson::jvalue doc = parser.parse_file ("instance.json");
ujson::jvalue patch = parser.parse_file ("patches.json");

auto result = ujson::patch (doc, patch);

if (result.first) {
    std::cout << "All patches ok" << std::endl;
}else{
    for (usigned i=0; i<result.second.size(); ++i) {
        std::cout << "Patch " << (i+1) << " of " << result.second.size() << ": ";
        switch (result.second[i]) {
            case ujson::patch_ok:
                std::cout << "Ok" << std::endl;
                break;
            case ujson::patch_fail:
                std::cout << "Test operation failed" << std::endl;
                break;
            case ujson::patch_noent:
                std::cout << "Value to patch not found in instance" << std::endl;
                break;
            case ujson::patch_invalid:
                std::cout << "Invalid patch definition" << std::endl;
                break;
        }
    }
}
```


## Using JSON Schema
libujson supports JSON Schema validation as described in https://json-schema.org/specification. Currently validation using version 2020-12 of the JSON Schema specification is supported.
A JSON Schema is represented by class `ujson::jschema`, and JSON instances can be validated using method `ujson::jschema::validate()`.
A schema object is created with a JSON schema definition. The JSON schema definition can be set in the constructor, or by calling method `ujson::jschema::reset()`. If the root schema definition references another external schema, this other schema definition can be added to the jschema object by calling method `ujson::jschema::add_referenced_schema()` Any number of schema definitions can be added to a jschema object so that it can be directly or indirectly referenced by the root schema.
Here is a simple example of how to use a JSON Schema to validate a JSON instance:
```c++
    ujson::jparser parser;
    ujson::jvalue schema_definition;
    ujson::jschema schema;

    // Load the root schema
    schema_definition = parser.parse_file (root_schema_file);
    schema.reset (schema_definition);

    // Load another schema that is referenced by the root schema    
    schema_definition = parser.parse_file (other_schema_referenced_by_the_root_schema_file);
    schema.add_referenced_schema (schema_definition);

    // Load the JSON instance we want to validate
    ujson::jvalue instance = parser.parse_file (json_instance_file_to_be_validated);

    // Validate the JSON instance
    ujson::jvalue output_unit = schema.validate (instance);

    // Check the result
    if (output_unit["valid"] == true)
        std::cout << "Instance is valid" << std::endl;
    else
        std::cout << "Instance is not valid" << std::endl;
```
