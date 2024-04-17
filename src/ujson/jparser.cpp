/*
 * Copyright (C) 2017,2019-2024 Dan Arrhenius <dan@ultramarin.se>
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
#include <ujson/internal.hpp>
#include <ujson/jtokenizer.hpp>
#include <ujson/jparser.hpp>
#include <ujson/utils.hpp>
#include <fstream>
#include <algorithm>
#include <string_view>
#include <string>
#include <stack>
#include <list>
#include <set>
#include <cstdlib>
#include <cstdint>


#define PARSE_DEBUG 0

#if (PARSE_DEBUG)
#include <iostream>
#include <iomanip>
using std::cerr;
using std::endl;
#endif


#define CTX reinterpret_cast<parser_t*>(parse_context)


using namespace ujson::parser;


namespace ujson {


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static const std::string parser_err_to_str (jparser::err error)
    {
        switch (error) {
        case ujson::jparser::err::ok:
            return "ok";

        case ujson::jparser::err::invalid_string:
            return "Invalid string";

        case ujson::jparser::err::unterminated_string:
            return "Unterminated string";

        case ujson::jparser::err::invalid_escape_code:
            return "Invalid escape code in string";

        case ujson::jparser::err::invalid_utf8:
            return "Invalid UTF8 code in string";

        case ujson::jparser::err::invalid_number:
            return "Invalid number";

        case ujson::jparser::err::number_out_of_range:
            return "Number out of range";

        case ujson::jparser::err::invalid_token:
            return "Invalind token";

        case ujson::jparser::err::unexpected_character:
            return "Unexpected character";

        case ujson::jparser::err::eob:
            return "Unexpected end of file/buffer";

        case ujson::jparser::err::io:
            return "I/O error";

        case ujson::jparser::err::internal:
            return "Internal error";

        case ujson::jparser::err::misplaced_right_curly_bracket:
            return "Misplaced '}'";

        case ujson::jparser::err::misplaced_right_bracket:
            return "Misplaced ']";

        case ujson::jparser::err::misplaced_separator:
            return "Misplaced ','";

        case ujson::jparser::err::misplaced_colon:
            return "Misplaced ':'";

        case ujson::jparser::err::expected_separator_or_right_bracket:
            return "Expected ',' or ']'";

        case ujson::jparser::err::expected_separator_or_right_curly_bracket:
            return "Expected ',' or '}'";

        case ujson::jparser::err::expected_obj_member_name:
            return "Expected object member name";

        case ujson::jparser::err::expected_colon:
            return "Expected ':'";

        case ujson::jparser::err::duplicate_obj_member:
            return "Duplicate object member name found.";

        case ujson::jparser::err::unterminated_array:
            return "Unterminated array";

        case ujson::jparser::err::unterminated_object:
            return "Unterminated object";

        case ujson::jparser::err::max_depth_exceeded:
            return "Maximum nesting depth exceeded.";

        case ujson::jparser::err::max_array_size_exceeded:
            return "Maximum number of array items exceeded.";

        case ujson::jparser::err::max_obj_size_exceeded:
            return "Maximum number of object members exceeded.";

        default:
            return "(unkown error)";
        }
    }


    //--------------------------------------------------------------------------
    class parser_t {
    public:
        parser_t () {
            max_depth = 0;
            max_array_size = 0;
            max_object_size = 0;
            strict = true;
            allow_duplicates = true;
            reset ();
        }

        void limits (unsigned max_depth_arg,
                     unsigned max_array_size_arg,
                     unsigned max_object_size_arg);

        jvalue parse (const char* buffer,
                      const unsigned buffer_size,
                      bool strict_parsing=false,
                      bool allow_duplicates_in_obj=true);
        jvalue parse (const std::string& buffer,
                      bool strict_parsing=false,
                      bool allow_duplicates_in_obj=true);
        jvalue parse_file (const std::string& file_name,
                           bool strict_parsing=false,
                           bool allow_duplicates_in_obj=true);

        const jparser::err error_code () const {
            return err_code;
        }
        const unsigned error_row () const {
            return err_row;
        }
        const unsigned error_col () const {
            return err_col;
        }

    private:
        unsigned max_depth;
        unsigned max_array_size;
        unsigned max_object_size;
        unsigned row;
        unsigned col;
        unsigned err_row;
        unsigned err_col;
        jparser::err err_code;
        const char* buf_pos;
        const char* buf_end;

        jtokenizer tokenizer;
        bool strict;
        bool allow_duplicates;

        void reset () {
            row = 0;
            col = 0;
            err_row = 0;
            err_col = 0;
            err_code = jparser::err::ok;
            buf_pos = nullptr;
            buf_end = nullptr;
            while (!parse_state.empty())
                parse_state.pop ();
            while (!parse_values.empty())
                parse_values.pop ();
            while (!parse_objects.empty())
                parse_objects.pop ();
            while (!parse_pairs.empty())
                parse_pairs.pop ();
        }
        /*
        void error (const jparser::err_code_t code, const std::string& msg) {
            error (code, msg, token.row, token.col);
        }
        */
        void error (const jparser::err code, unsigned row_arg, unsigned col_arg) {
            err_row = row_arg;
            err_col = col_arg;
            err_code = code;
        }

        jvalue token_to_number (const jtoken& token);

        void on_parsed_value (const jtoken& token, jvalue&& value);
        jvalue parse_tokens ();
        jvalue post_parse_tokens (const jtoken* token);
        void parse_value_tokens (const jtoken& token);
        bool parse_str_value_tokens (const jtoken& token);
        void parse_array_tokens (const jtoken& token);
        void parse_elements_tokens (const jtoken& token);
        void parse_object_tokens (const jtoken& token);
        void parse_members_tokens (const jtoken& token);
        void on_object_member_name (const jtoken& token);
        void parse_pair_tokens (const jtoken& token);

        enum parse_state_t {
            ps_value,     // A JSON value of any type is being parsed
            ps_str_value, // A JSON string is being parsed

            ps_array,     // A JSON array is being parsed
            ps_elements,  // Elements in a JSON array is being parsed

            ps_object,    // A JSON object is being parsed
            ps_members,   // Members (attributes) of an object is being parsed
            ps_pair,      // A key-value pair of an object member is being parsed
        };

#if (PARSE_DEBUG)
        static std::string parse_state_to_string (parse_state_t ps) {
            switch (ps) {
            case ps_value:
                return "ps_value";
                break;
            case ps_str_value:
                return "ps_str_value";
                break;
            case ps_array:
                return "ps_array";
                break;
            case ps_elements:
                return "ps_elements";
                break;
            case ps_object:
                return "ps_object";
                break;
            case ps_members:
                return "ps_members";
                break;
            case ps_pair:
                return "ps_pair";
                break;
            default:
                return "(unknown)";
            }
        }
#endif

        struct obj_member_t {
            obj_member_t ()
                : has_name (false),
                  has_colon (false)
                {
                }
            void reset () {
                has_name = false;
                has_colon = false;
            }
            std::string name;
            bool has_name;
            bool has_colon;
        };

        std::stack<parse_state_t> parse_state;

        // The first json_array pushed to this stack is an array
        // with only a single element: The top level instance of
        // the parsed JSON document.
        // After this, the top of the stack contains an array
        // with either:
        //   The elements of the currently parsed JSON array.
        // or:
        //   A single element of the currently parsed object member value.
        std::stack<json_array> parse_values;

        // The top of the stack contains the currently parsed object.
        std::stack<jvalue> parse_objects;

        // The top of the stack contains the state
        // of the currently parsed object member.
        std::stack<obj_member_t> parse_pairs;

        // This is the currently parsed string value until
        // the complete string is parsed.
        // In relaxed mode, a string can be made up by
        // multiple strings divided by whitespaces and comments.
        std::string parsed_string;

#if (PARSE_DEBUG)
        void dump_parse_stack_sizes () {
            cerr << "Parse stack sizes:" << endl;
            cerr << "    parse_state  : " << parse_state.size() << endl;
            cerr << "    parse_values : " << parse_values.size() << endl;
            cerr << "    parse_objects: " << parse_objects.size() << endl;
            cerr << "    parse_pairs  : " << parse_pairs.size() << endl;
        }
#else
        inline void dump_parse_stack_sizes () {}
#endif
    };

#if 0
value:          str_value
        |       NUMBER
        |       object
        |       array
        |       TRUE
        |       FALSE
        |       NULL
                ;

str_value:	STRING
	|	STRING str_value          // Relaxed mode
		;

array:          LBRACK RBRACK
        |       LBRACK elements RBRACK
                ;

elements:       value SEPARATOR elements
        |       value SEPARATOR           // Relaxed mode
        |       value
                ;

object:         LCBRACK RCBRACK
        |       LCBRACK members RCBRACK
                ;

members:        pair SEPARATOR members
        |       pair SEPARATOR            // Relaxed mode
        |       pair
                ;

pair:           STRING COLON value
        |       IDENTIFIER COLON value    // Relaxed mode
                ;
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    static jparser::err token_error_to_parser_error (const jtoken::error_t token_error)
    {
        switch (token_error) {
        case jtoken::ok:
            return jparser::err::ok;

        case jtoken::err_string:
            return jparser::err::invalid_string;

        case jtoken::err_string_unterminated:
            return jparser::err::unterminated_string;

        case jtoken::err_string_escape:
            return jparser::err::invalid_escape_code;

        case jtoken::err_string_utf8:
            return jparser::err::invalid_utf8;

        case jtoken::err_number:
        case jtoken::err_number_lone_minus:
        case jtoken::err_number_no_frac:
        case jtoken::err_number_no_exp:
            return jparser::err::invalid_number;

        case jtoken::err_invalid:
            return jparser::err::invalid_token;

        case jtoken::err_unexpected_char:
            return jparser::err::unexpected_character;

        case jtoken::err_eob:
            return jparser::err::eob;
        }

#if (PARSE_DEBUG)
        cerr << "Internal error here: " << __LINE__ << endl;
#endif
        return jparser::err::internal;
    }


     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
    void parser_t::limits (unsigned max_depth_arg,
                           unsigned max_array_size_arg,
                           unsigned max_object_size_arg)
    {
        max_depth = max_depth_arg;
        max_array_size = max_array_size_arg;
        max_object_size = max_object_size_arg;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool parser_t::parse_str_value_tokens (const jtoken& token)
    {
        if (token.type != jtoken::tk_string) {
            parse_state.pop (); // pop ps_str_value
            on_parsed_value (token, jvalue(std::move(parsed_string)));
            return false; // Token not consumed
        }

        try {
            parsed_string.append (unescape(token.data));
        }
        catch (...) {
            error (jparser::err::invalid_string, token.row, token.col);
        }

        return true; // Token consumed
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue parser_t::token_to_number (const jtoken& token)
    {
        jvalue value (j_number);
        std::string str (token.data);
        try {
#if UJSON_HAVE_GMPXX
            // Lazy way to adjust the precision to fit the number.
            // 4 bits per decimal digit in the number string,
            // or 4 bits times the power of ten when the format is xEpow,
            // or mpf_get_default_prec(), whatever is higher.
            //std::string_view str (tk.ptr, tk.size);
            auto e_pos = str.find ("e");
            if (e_pos == std::string::npos)
                e_pos = str.find ("E");

            size_t precision;
            if (e_pos == std::string::npos) {
                precision = std::max (mpf_get_default_prec(),
                                      mp_bitcnt_t(str.size()*4));
            }else{
                precision = std::max (mpf_get_default_prec(),
                                      mp_bitcnt_t(std::abs(std::stod(str.substr(e_pos+1)))*4));
            }
            jvalue::num_t number (str, precision);
            value.num (std::move(number));
#else
            jvalue::num_t number = std::stod (str);
            value.num (number);
#endif
        }
        catch (std::invalid_argument&) {
            error (jparser::err::invalid_number, token.row, token.col);
#if (PARSE_DEBUG)
            cerr << "Error converting token to number, token.data: " << token.data << endl;
#endif
            value.type (j_invalid);
        }
        catch (std::out_of_range&) {
            error (jparser::err::number_out_of_range, token.row, token.col);
            value.type (j_invalid);
        }

        return value;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::on_parsed_value (const jtoken& token, jvalue&& value)
    {
        parse_values.top().emplace_back (std::forward<jvalue>(value));
        parse_state.pop ();
        if (max_array_size && !parse_state.empty() && parse_state.top()==ps_elements) {
            if (parse_values.top().size() > max_array_size) {
                error (jparser::err::max_array_size_exceeded,
                       token.row,
                       token.col);
            }
        }
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::parse_value_tokens (const jtoken& token)
    {
        switch (token.type) {
        case jtoken::tk_invalid:
            error (jparser::err::invalid_token, token.row, token.col);
            break;

        case jtoken::tk_lcbrack:
            // Start of object
            if (max_depth  &&  parse_values.size() > max_depth)
                error (jparser::err::max_depth_exceeded, token.row, token.col);
            else
                parse_state.push (ps_object);
            break;

        case jtoken::tk_rcbrack:
            error (jparser::err::misplaced_right_curly_bracket, token.row, token.col);
            break;

        case jtoken::tk_lbrack:
            // Start of array
            if (max_depth  &&  parse_values.size() > max_depth)
                error (jparser::err::max_depth_exceeded, token.row, token.col);
            else
                parse_state.push (ps_array);
            break;

        case jtoken::tk_rbrack:
            if (strict) {
                error (jparser::err::misplaced_right_bracket, token.row, token.col);
            }else{
                parse_state.pop ();
                if (!parse_state.empty() && parse_state.top()==ps_elements)
                    parse_elements_tokens (token);
                else
                    error (jparser::err::misplaced_right_bracket, token.row, token.col);
            }
            break;

        case jtoken::tk_separator:
            error (jparser::err::misplaced_separator, token.row, token.col);
            break;

        case jtoken::tk_colon:
            error (jparser::err::misplaced_colon, token.row, token.col);
            break;

        case jtoken::tk_null:
            on_parsed_value (token, nullptr);
            break;

        case jtoken::tk_true:
            on_parsed_value (token, true);
            break;

        case jtoken::tk_false:
            on_parsed_value (token, false);
            break;

        case jtoken::tk_string:
            try {
                if (strict) {
                    on_parsed_value (token, unescape(token.data));
                }else{
                    parsed_string = unescape (token.data);
                    parse_state.push (ps_str_value);
                }
            }
            catch (...) {
                error (jparser::err::invalid_string, token.row, token.col);
            }
            break;

        case jtoken::tk_number:
            on_parsed_value (token, token_to_number(token));
            break;

        case jtoken::tk_identifier:
            error (jparser::err::invalid_token, token.row, token.col);
            break;

        case jtoken::tk_comment:
            // Ignore comments
            break;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::parse_elements_tokens (const jtoken& token)
    {
        if (token.type == jtoken::tk_separator) {
            parse_state.push (ps_value);
        }
        else if (token.type == jtoken::tk_rbrack) {
            // Array done !
            jvalue array_value = std::move (parse_values.top());
            parse_values.pop ();

            parse_state.pop (); // pop ps_elements
            parse_state.pop (); // pop ps_array
            on_parsed_value (token, std::move(array_value));
        }
        else {
            error (jparser::err::expected_separator_or_right_bracket, token.row, token.col);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::parse_array_tokens (const jtoken& token)
    {
        if (token.type == jtoken::tk_rbrack) {
            // Array done - empty array
            parse_state.pop (); // pop ps_array
            on_parsed_value (token, jvalue(j_array));
        }else{
            // Start collecting array values
            parse_values.push (json_array());
            parse_state.push (ps_elements);
            parse_state.push (ps_value);

            parse_value_tokens (token);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::on_object_member_name (const jtoken& token)
    {
        try {
            parse_pairs.top().name = unescape (token.data);
            parse_pairs.top().has_name = true;
        }
        catch (...) {
            error (jparser::err::invalid_string, token.row, token.col);
            return;
        }

        // Check for duplicate name
        if (allow_duplicates == false) {
            if (parse_objects.top().has(parse_pairs.top().name)) {
                error (jparser::err::duplicate_obj_member,
                       token.row,
                       token.col);
                return;
            }
        }

        // Check object size limit
        auto& obj = parse_objects.top().obj ();
        if (max_object_size  &&  obj.size()+1 > max_object_size) {
            error (jparser::err::max_obj_size_exceeded,
                   token.row,
                   token.col);
            return;
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::parse_pair_tokens (const jtoken& token)
    {
        if (parse_pairs.top().has_name == false) {
            // We haven't got a member name yet
            if (token.type == jtoken::tk_string  ||  token.type == jtoken::tk_identifier) {

                on_object_member_name (token);

            }else{
                if (strict==false && token.type == jtoken::tk_rcbrack) {
                    // Object member list ended with a ','
                    parse_state.pop (); // ps_pair
                    parse_members_tokens (token);
                }else{
                    error (jparser::err::expected_obj_member_name, token.row, token.col);
                }
            }
        }
        else if (parse_pairs.top().has_colon == false) {
            // We haven't got a colon yet
            if (token.type == jtoken::tk_colon) {
                parse_pairs.top().has_colon = true;
                // We got a colon, now we expect a value
                parse_values.push (json_array());
                parse_state.push (ps_value);
            }else{
                error (jparser::err::expected_colon, token.row, token.col);
            }
        }
        else {
            // We have a key-value pair
            auto& obj = parse_objects.top().obj ();
            obj.emplace_back (std::move(parse_pairs.top().name),
                              std::move(parse_values.top()[0]));
            parse_values.pop ();
            parse_state.pop (); // ps_pair

            parse_members_tokens (token);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::parse_members_tokens (const jtoken& token)
    {
        if (token.type == jtoken::tk_separator) {
            // Next object member
            parse_pairs.top().reset ();
            parse_state.push (ps_pair);
        }
        else if (token.type == jtoken::tk_rcbrack) {
            // Object done !
            parse_state.pop (); // pop ps_members
            parse_state.pop (); // pop ps_object
            on_parsed_value (token, std::move(parse_objects.top()));

            parse_pairs.pop ();   // Done with current temporary object pair
            parse_objects.pop (); // Done with the current object
        }
        else {
            error (jparser::err::expected_separator_or_right_curly_bracket, token.row, token.col);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void parser_t::parse_object_tokens (const jtoken& token)
    {
        if (token.type == jtoken::tk_rcbrack) {
            // An empty object
            parse_state.pop (); // pop ps_object
            on_parsed_value (token, jvalue(j_object));
        }else{
            // Start parsing a new object member
            parse_objects.push (jvalue(j_object));
            parse_pairs.push (obj_member_t());

            parse_state.push (ps_members);
            parse_state.push (ps_pair);

            parse_pair_tokens (token);
        }
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue parser_t::parse_tokens ()
    {
        const jtoken* token;

        parse_state.push (ps_value);
        parse_values.push (json_array());

        // Fetch the first token
        //
        do {
            token = tokenizer.next_token ();
            if (!token || token->type == jtoken::tk_invalid) {
                if (!token) {
                    error (jparser::err::eob, 0, 0);
                }else if (token->err_code == jtoken::ok) {
                    // Is this even possible ?
                    error (jparser::err::eob, tokenizer.pos().first, tokenizer.pos().second);
                }else{
                    error (token_error_to_parser_error(token->err_code), token->row, token->col);
                }
                return jvalue (j_invalid);
            }
        }while (token->type == jtoken::tk_comment); // Ignore comments

        // Process tokens
        //
        do {
            bool token_consumed = true;

#if (PARSE_DEBUG)
            cerr << "Parse token " << jtoken_type_to_string(token->type) << endl;
#endif
            switch (parse_state.top()) {
            case ps_value:
                parse_value_tokens (*token);
                break;
            case ps_str_value:
                token_consumed = parse_str_value_tokens (*token);
                break;
            case ps_array:
                parse_array_tokens (*token);
                break;
            case ps_elements:
                parse_elements_tokens (*token);
                break;
            case ps_object:
                parse_object_tokens (*token);
                break;
            case ps_members:
                parse_members_tokens (*token);
                break;
            case ps_pair:
                parse_pair_tokens (*token);
                break;
            }

            if (token_consumed) {
                do {
                    token = tokenizer.next_token ();
                }while (token && token->type == jtoken::tk_comment);
            }
        }while (err_code == jparser::err::ok   &&
                token != nullptr               &&
                token->err_code == jtoken::ok  &&
                parse_state.empty() == false);

        return post_parse_tokens (token);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue parser_t::post_parse_tokens (const jtoken* token)
    {
        jvalue value (j_invalid);

        if (!parse_state.empty()  && parse_state.top()==ps_str_value) {
            // We were parsing a string in relaxed mode, finish it
            jtoken dummy_token;
            parse_str_value_tokens (dummy_token);
        }

#if (PARSE_DEBUG)
        if (token)
            cerr << "Parse ended when tokens left. Next token: " << jtoken_type_to_string(token->type) << endl;
        else
            cerr << "No more tokens to parse" << endl;
        dump_parse_stack_sizes ();
#endif

        if (err_code != jparser::err::ok) {
            //
            // Syntax error while parsing
            //
        }
        else if (token != nullptr) {
            //
            // Unexpected token(s) after successully parsing a JSON instance
            //
            error (jparser::err::unexpected_character, token->row, token->col );
        }
        else if (parse_state.empty() == false) {
            //
            // Parse state stack not empty
            //
            switch (parse_state.top()) {
            case ps_array:
            case ps_elements:
                // We have an unterminated array
                error (jparser::err::unterminated_array, row, col);
                break;

            case ps_object:
            case ps_members:
            case ps_pair:
                // We have an unterminated object
                error (jparser::err::unterminated_object, row, col);
                break;

            default:
#if (PARSE_DEBUG)
                cerr << "Internal error here: " << __LINE__ << endl;
#endif
                error (jparser::err::internal, row, col);
            }
        }

        if (err_code == jparser::err::ok) {
            value = std::move (parse_values.top().back());
            parse_values.top().pop_back ();
            parse_values.pop ();

            if (parse_values.empty()==false) {
#if (PARSE_DEBUG)
                cerr << "Internal error here: " << __LINE__ << endl;
#endif
                error (jparser::err::internal, row, col);
                value.type (j_invalid);
            }
        }

        return value;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue parser_t::parse (const char* buffer,
                            const unsigned buffer_size,
                            bool strict_parsing,
                            bool allow_duplicates_in_obj)
    {
        strict = strict_parsing;
        allow_duplicates = allow_duplicates_in_obj;
        reset ();
        tokenizer.reset (std::string_view(buffer, buffer_size), strict);

        return parse_tokens ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue parser_t::parse (const std::string& buffer,
                            bool strict_parsing,
                            bool allow_duplicates_in_obj)
    {
        return parse (buffer.c_str(),
                      buffer.size(),
                      strict_parsing,
                      allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue parser_t::parse_file (const std::string& file_name,
                                 bool strict_parsing,
                                 bool allow_duplicates_in_obj)
    {
        std::ifstream in (file_name);
        if (!in.good()) {
            error (jparser::err::io, 0, 0);
            return jvalue (j_invalid);
        }
        std::string json_doc ((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close ();

        return parse (json_doc.c_str(),
                      json_doc.size(),
                      strict_parsing,
                      allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jparser::jparser ()
    {
        parse_context = new parser_t;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jparser::jparser (unsigned max_depth,
                      unsigned max_array_size,
                      unsigned max_object_size)
    {
        parse_context = new parser_t;
        limits (max_depth, max_array_size, max_object_size);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jparser::~jparser ()
    {
        delete CTX;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jparser::limits (unsigned max_depth,
                          unsigned max_array_size,
                          unsigned max_object_size)
    {
        CTX->limits (max_depth, max_array_size, max_object_size);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jparser::parse_file (const std::string& f,
                                bool strict_mode,
                                bool allow_duplicates_in_obj)
    {
        return CTX->parse_file (f, strict_mode, allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jparser::parse_string (const std::string& str,
                                  bool strict_mode,
                                  bool allow_duplicates_in_obj)
    {
        return CTX->parse (str, strict_mode, allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue jparser::parse_buffer (const char* buf,
                                  size_t length,
                                  bool strict_mode,
                                  bool allow_duplicates_in_obj)
    {
        return CTX->parse (buf, length, strict_mode, allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    const jparser::error_t jparser::get_error () const
    {
        error_t err;
        err.code = CTX->error_code ();
        err.row  = CTX->error_row ();
        err.col  = CTX->error_col ();
        return err;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    const std::string& jparser::error () const
    {
        static std::string err_str;
        auto error_code = CTX->error_code ();

        if (error_code == jparser::err::ok) {
            err_str = "Ok.";
        }else{
            err_str = parser_err_to_str (error_code);
            err_str.append (" at line ");
            err_str.append (std::to_string(CTX->error_row()+1));
            err_str.append (", column ");
            err_str.append (std::to_string(CTX->error_col()));
        }
        return err_str;
    }


}
