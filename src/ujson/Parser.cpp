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
#include <sstream>
#include <iostream>
#include <algorithm>
#include <ujson/Parser.hpp>
#include <ujson/Analyzer.hpp>
#include <ujson/utils.hpp>
#include <ujson/jvalue.hpp>
#include <Lexer.hh>

namespace ujson {

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Parser::Parser (bool trace_scan, bool trace_parse)
        : trace_scanning  {trace_scan},
          trace_parsing   {trace_parse}
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue Parser::parse_file (const std::string& f,
                               bool strict_mode,
                               bool allow_duplicates_in_obj)
    {
        std::lock_guard<std::mutex> lock (parse_mutex);

        use_strict_mode = strict_mode;
        allow_key_duplicates = allow_duplicates_in_obj;
        root.reset ();
        parse_error.clear ();
        file = f;
        ujlex_init (&yyscanner);
        Analyzer analyzer (*this, yyscanner);

        analyzer.set_debug_level (trace_parsing);
        if (!scan_begin()) {
            ujset_debug (trace_scanning, yyscanner);
            if (analyzer.parse())
                root.reset ();
            scan_end ();
            ujlex_destroy (yyscanner);
        }
        return std::move (root);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue Parser::parse_string (const std::string& str,
                                 bool strict_mode,
                                 bool allow_duplicates_in_obj)
    {
        return parse_buffer (str.c_str(),
                             str.length(),
                             strict_mode,
                             allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue Parser::parse_buffer (const char* buf,
                                 size_t length,
                                 bool strict_mode,
                                 bool allow_duplicates_in_obj)
    {
        std::lock_guard<std::mutex> lock (parse_mutex);

        use_strict_mode = strict_mode;
        allow_key_duplicates = allow_duplicates_in_obj;
        root.reset ();
        parse_error.clear ();
        ujlex_init (&yyscanner);
        auto scan_buf = uj_scan_bytes (buf, length, yyscanner);
        file = "";

        Analyzer analyzer (*this, yyscanner);
        analyzer.set_debug_level (trace_parsing);
        ujset_debug (trace_scanning, yyscanner);
        if (analyzer.parse())
            root.reset ();

        uj_delete_buffer (scan_buf, yyscanner);
        ujlex_destroy (yyscanner);

        return std::move (root);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::error (const location& l, const std::string& m)
    {
        std::stringstream ss;
        ss << l << ": " << m;
        parse_error = ss.str ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::error (const std::string& m)
    {
        parse_error = m;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    const std::string& Parser::error () const
    {
        return parse_error;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool Parser::trace_scan () const
    {
        return trace_scanning;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::trace_scan (bool onoff)
    {
        trace_scanning = onoff;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool Parser::trace_parse () const
    {
        return trace_parsing;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::trace_parse (bool onoff)
    {
        trace_parsing = onoff;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    int Parser::scan_begin ()
    {
        FILE* f;

        if (file.empty()) {
            ujset_in (stdin, yyscanner);
        }else{
            f = fopen (file.c_str (), "r");
            if (!f) {
                error (strerror(errno));
                return -1;
            }
            ujset_in (f, yyscanner);
        }
        return 0;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::scan_end ()
    {
        fclose (ujget_in(yyscanner));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_lcbrack ()
    {
        return Analyzer::make_LCBRACK (loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_rcbrack ()
    {
        return Analyzer::make_RCBRACK (loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_lbrack ()
    {
        return Analyzer::make_LBRACK (loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_rbrack ()
    {
        return Analyzer::make_RBRACK (loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_separator ()
    {
        return Analyzer::make_SEPARATOR (loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_colon ()
    {
        return Analyzer::make_COLON (loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_null ()
    {
        return Analyzer::make_NULL (0, loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_true ()
    {
        return Analyzer::make_TRUE (true, loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_false ()
    {
        return Analyzer::make_FALSE (false, loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_identifier ()
    {
        if (use_strict_mode)
            throw Analyzer::syntax_error (loc(), "syntax error, unexpected identifier, expecting string");
        std::string str (ujget_text(yyscanner), ujget_leng(yyscanner));
        return Analyzer::make_IDENTIFIER (str, loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_string ()
    {
        std::string str (ujget_text(yyscanner)+1, ujget_leng(yyscanner)-2);
        return Analyzer::make_STRING (unescape(str), loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_number ()
    {
        double num = strtod (ujget_text(yyscanner), nullptr);
        return Analyzer::make_NUMBER (num, loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_nan ()
    {
        if (use_strict_mode)
            throw Analyzer::syntax_error (loc(), "syntax error, NaN as number not allowed in strict mode");
        double num = strtod (ujget_text(yyscanner), nullptr);
        return Analyzer::make_NAN (num, loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_inf ()
    {
        if (use_strict_mode)
            throw Analyzer::syntax_error (loc(), "syntax error, Infinite as number not allowed in strict mode");
        double num = strtod (ujget_text(yyscanner), nullptr);
        return Analyzer::make_INF (num, loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_lex_whitespace ()
    {
        loc().step ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_lex_newline ()
    {
        loc().lines (ujget_leng(yyscanner));
        loc().step ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_lex_comment ()
    {
        if (use_strict_mode)
            throw Analyzer::syntax_error (loc(), "syntax error, c-style comments not allowed in strict mode");
        std::string comment (ujget_text(yyscanner));
        loc().lines (std::count(comment.begin(), comment.end(), '\n'));
        loc().step ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_lex_error ()
    {
        throw Analyzer::syntax_error (loc(), "invalid character");
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Analyzer::symbol_type Parser::on_lex_eof ()
    {
        return Analyzer::make_END (loc());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_root ()
    {
        if (!values.empty()) {
            root = std::move (values.top());
            while (!values.empty())
                values.pop ();
        }
        elements.clear ();
        while (!pairs.empty())
            pairs.pop ();
        members.clear ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_object ()
    {
        values.emplace (std::move(members));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_member (bool relaxed)
    {
        if (use_strict_mode && relaxed)
            throw Analyzer::syntax_error (loc(), "syntax error, object ending with separator");
        if (allow_key_duplicates || !members.contains(pairs.top().first))
            members.push_front (std::move(pairs.top()));
        pairs.pop ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_pair (const std::string& key, bool relaxed)
    {
        if (use_strict_mode && relaxed)
            throw Analyzer::syntax_error (loc(), "syntax error, unexpected identifier, expecting } or string");
        pairs.emplace (std::make_pair(key, std::move(values.top())));
        values.pop ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_array ()
    {
        jvalue a (j_array);
        for (auto& v : elements)
            a.add (std::move(v));
        elements.clear ();
        values.emplace (std::move(a));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_element (bool relaxed)
    {
        if (use_strict_mode && relaxed)
            throw Analyzer::syntax_error (loc(), "syntax error, array ending with separator");
        elements.emplace_front (std::move(values.top()));
        values.pop ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_string (const std::string& str, bool root_entry)
    {
        values.emplace (str);
        if (root_entry)
            on_parse_root ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_number (double num, bool root_entry)
    {
        values.emplace (num);
        if (root_entry)
            on_parse_root ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_nan (double num, bool root_entry)
    {
        if (use_strict_mode)
            throw Analyzer::syntax_error (loc(), "syntax error, NaN as number not allowed in strict mode");
        values.emplace (num);
        if (root_entry)
            on_parse_root ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_inf (double num, bool root_entry)
    {
        if (use_strict_mode)
            throw Analyzer::syntax_error (loc(), "syntax error, Infinite as number not allowed in strict mode");
        values.emplace (num);
        if (root_entry)
            on_parse_root ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_bool (const bool value, bool root_entry)
    {
        values.emplace (value);
        if (root_entry)
            on_parse_root ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Parser::on_parse_null (bool root_entry)
    {
        values.emplace (nullptr);
        if (root_entry)
            on_parse_root ();
    }

}
