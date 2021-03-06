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
#include <ujson/Json.hpp>
#include <ujson/Parser.hpp>


namespace ujson {


    #define CTX reinterpret_cast<Parser*>(parse_context)


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Json::Json ()
    {
        parse_context = new Parser;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    Json::~Json ()
    {
        delete CTX;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue Json::parse_file (const std::string& f,
                             bool strict_mode,
                             bool allow_duplicates_in_obj)
    {
        return CTX->parse_file (f, strict_mode, allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue Json::parse_string (const std::string& str,
                               bool strict_mode,
                               bool allow_duplicates_in_obj)
    {
        return CTX->parse_string (str, strict_mode, allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jvalue Json::parse_buffer (const char* buf,
                               size_t length,
                               bool strict_mode,
                               bool allow_duplicates_in_obj)
    {
        return CTX->parse_buffer (buf, length, strict_mode, allow_duplicates_in_obj);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    const std::string& Json::error () const
    {
        return CTX->error ();
    }


#if (PARSER_DEBUGGING)
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool Json::trace_parsing ()
    {
        return CTX->trace_parse ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Json::trace_parsing (bool trace)
    {
        CTX->trace_parse (trace);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool Json::trace_scanning ()
    {
        return CTX->trace_scan ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void Json::trace_scanning (bool trace)
    {
        CTX->trace_scan (trace);
    }
#endif

}
