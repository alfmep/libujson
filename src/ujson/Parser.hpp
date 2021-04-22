/*
 * Copyright (C) 2017,2019-2021 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_PARSER_HH
#define UJSON_PARSER_HH

#include <string>
#include <mutex>
#include <stack>
#include <list>
#include <ujson/Analyzer.hpp>
#include <ujson/jvalue.hpp>


namespace ujson {

    class Parser {
    public:
        Parser (bool trace_scan=false, bool trace_parse=false);
        virtual ~Parser () = default;

        // The location of the current token.
        location& loc () {return pos;}

        const std::string& error () const;

        bool trace_scan () const;
        void trace_scan (bool onoff);

        bool trace_parse () const;
        void trace_parse (bool onoff);

        // Run the parser on file F.
        // Return 0 on success.
        jvalue parse_file (const std::string& f, bool strict_mode, bool allow_duplicates_in_obj);
        jvalue parse_string (const std::string& str, bool strict_mode, bool allow_duplicates_in_obj);
        jvalue parse_buffer (const char* buf, size_t length, bool strict_mode, bool allow_duplicates_in_obj);


    public:
        //friend Analyzer::symbol_type ujlex (Parser& parser, yyscan_t yyscanner);

        Analyzer::symbol_type on_lex_lcbrack ();
        Analyzer::symbol_type on_lex_rcbrack ();
        Analyzer::symbol_type on_lex_lbrack ();
        Analyzer::symbol_type on_lex_rbrack ();
        Analyzer::symbol_type on_lex_separator ();
        Analyzer::symbol_type on_lex_colon ();
        Analyzer::symbol_type on_lex_null ();
        Analyzer::symbol_type on_lex_true ();
        Analyzer::symbol_type on_lex_false ();
        Analyzer::symbol_type on_lex_identifier ();
        Analyzer::symbol_type on_lex_string ();
        Analyzer::symbol_type on_lex_number ();
        Analyzer::symbol_type on_lex_nan ();
        Analyzer::symbol_type on_lex_inf ();
        void on_lex_comment ();
        void on_lex_whitespace ();
        void on_lex_newline ();
        void on_lex_error ();
        Analyzer::symbol_type on_lex_eof ();


    private:
        friend class Analyzer;

        std::string parse_error;
        bool use_strict_mode;

        // Handling the lexer.
        int scan_begin ();
        void scan_end ();
        bool trace_scanning;


        // The name of the file being parsed.
        // Used later to pass the file name to the location tracker.
        std::string file;

        // Whether parser traces should be generated.
        bool trace_parsing;

        // Error handling.
        void error (const location& l, const std::string& m);
        void error (const std::string& m);

        void on_parse_root ();
        void on_parse_object ();
        void on_parse_member (bool relaxed=false);
        void on_parse_pair (const std::string& key, bool relaxed=false);
        void on_parse_array ();
        void on_parse_element (bool relaxed=false);
        void on_parse_string (const std::string& str, bool relaxed=false);
        void on_parse_str_value (bool root_entry=false);
        void on_parse_number (double num, bool root_entry=false);
        void on_parse_nan (double num, bool root_entry=false);
        void on_parse_inf (double num, bool root_entry=false);
        void on_parse_bool (const bool value, bool root_entry=false);
        void on_parse_null (bool root_entry=false);

        std::mutex parse_mutex;
        location pos;
        yyscan_t yyscanner;

        bool allow_key_duplicates;
        jvalue root;
        json_object members;
        std::list<jvalue> elements;
        std::stack<json_pair> pairs;
        std::stack<jvalue> values;
        std::string str_value;
    };

}


#define YY_DECL ujson::Analyzer::symbol_type ujlex (ujson::Parser& parser, yyscan_t yyscanner)
YY_DECL;


#endif
