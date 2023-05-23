#include "option-parser.hpp"
#include <algorithm>
#include <cstring>


enum option_variant_t {
    optvar_argument = 0,
    optvar_short,
    optvar_long
};


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
option_parser::option_parser (int argc_arg, char* argv_arg[])
    : argc (argc_arg),
      argv (argv_arg),
      argc_index (0),
      short_opt_ptr (nullptr),
      current_short_opt ('\0')
{
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string option_parser::optarg ()
{
    return std::string (opt_arg);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
char option_parser::short_opt ()
{
    return current_short_opt;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string option_parser::long_opt ()
{
    return current_long_opt;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string option_parser::opt ()
{
    if (!current_long_opt.empty())
        return std::string (current_long_opt);
    else if (current_short_opt)
        return std::string (1, current_short_opt);
    else
        return std::string ("");
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::vector<std::string>& option_parser::arguments ()
{
    return args;
}


//------------------------------------------------------------------------------
// Return: optvar_argument - Not an option
//         optvar_short    - A short option
//         optvar_long     - A long option
//------------------------------------------------------------------------------
static option_variant_t option_variant (char* arg)
{
    if (arg[0] == '-') {
        if (arg[1] == '-') {
            if (arg[2] != '\0') {
                // '--option[=| ]value'
                return optvar_long;
            }
        }
        else if (arg[1] != '\0') {
            // '-?[?...][ ]value
            return optvar_short;
        }
    }
    return optvar_argument;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int option_parser::operator() (const optlist_t& option_descriptor)
{
    opt_arg.clear ();
    current_long_opt.clear ();
    current_short_opt = '\0';

    if (short_opt_ptr) {
        // Handle next short option
        return handle_short_opt (option_descriptor);
    }

    if (argc_index < argc)
        ++argc_index;
    if (argc_index >= argc)
        return 0;

    option_variant_t optvar;
    while ((optvar = option_variant(argv[argc_index])) == optvar_argument) {
        // This is an argument, not an option
        args.emplace_back (std::string(argv[argc_index++]));
        if (argc_index >= argc)
            return 0;
    }

    if (optvar == optvar_short) {
        // A short option
        short_opt_ptr = &argv[argc_index][1];
        return handle_short_opt (option_descriptor);
    }
    // A long option
    return handle_long_opt (option_descriptor);
}


//------------------------------------------------------------------------------
// return >0 An option was found. Option ID is returned.
//        -1 If the found option isn't in the option descriptor.
//        -2 If the found option is missing a required option argument.
//------------------------------------------------------------------------------
int option_parser::handle_short_opt (const optlist_t& option_descriptor)
{
    current_short_opt = *short_opt_ptr;

    // Find the short option in the option description
    //
    auto is_opt = [this](const opt_t& opt)->bool {return opt.short_opt && opt.short_opt == current_short_opt;};
    auto result = std::find_if (option_descriptor.begin(), option_descriptor.end(), is_opt);
    if (result == option_descriptor.end()) {
        // The short option wasn't found in the option description
        return -1;
    }

    // Get the option ID
    int id = result->id==0 ? (int)result->short_opt : result->id;

    // Check for option argument
    if (result->arg==opt_t::optional || result->arg==opt_t::required) {
        bool have_arg = false;
        if (short_opt_ptr[1] != '\0') {
            opt_arg = std::string (&short_opt_ptr[1]);
            have_arg = true;
        }else{
            if (argc_index+1 < argc) {
                if (option_variant(argv[argc_index+1]) == optvar_argument) {
                    opt_arg = std::string (argv[++argc_index]);
                    have_arg = true;
                }
            }
        }
        short_opt_ptr = nullptr;
        if (!have_arg && result->arg==opt_t::required)
            return -2;
    }else{
        ++short_opt_ptr;
        if (*short_opt_ptr == '\0')
            short_opt_ptr = nullptr;
    }

    return id;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int option_parser::handle_long_opt (const optlist_t& option_descriptor)
{
    // Find '='
    size_t equal_pos = 0;
    char* ptr_equal = strchr (argv[argc_index]+3, '=');
    if (ptr_equal)
        equal_pos = ptr_equal - argv[argc_index];

    // Get the name of the long option
    //
    if (equal_pos) {
        current_long_opt = std::string (argv[argc_index]+2, equal_pos-2);
    }else{
        current_long_opt = std::string (argv[argc_index]+2);
    }

    // Find the long option in the option description
    //
    auto is_opt = [this](const opt_t& opt)->bool {return !opt.long_opt.empty() && opt.long_opt == current_long_opt;};
    auto result = std::find_if (option_descriptor.begin(), option_descriptor.end(), is_opt);
    if (result == option_descriptor.end()) {
        // The option wasn't found in the option description
        return -1;
    }

    // Get the option ID
    int id = result->id==0 ? (int)result->short_opt : result->id;

    // Check for option argument
    if (result->arg==opt_t::optional || result->arg==opt_t::required) {
        bool have_arg = false;
        if (equal_pos) {
            opt_arg = std::string (argv[argc_index]+equal_pos+1);
            have_arg = true;
        }else{
            if (argc_index+1 < argc) {
                if (option_variant(argv[argc_index+1]) == optvar_argument) {
                    opt_arg = std::string (argv[++argc_index]);
                    have_arg = true;
                }
            }
        }
        if (!have_arg && result->arg==opt_t::required)
            return -2;
    }

    return id;
}
