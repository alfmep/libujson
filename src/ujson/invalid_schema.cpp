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
#include <ujson/invalid_schema.hpp>


namespace ujson {


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    invalid_schema::invalid_schema (const std::string& base_uri_arg,
                                    const std::string& pointer_arg,
                                    const std::string& what_arg)
        : std::invalid_argument (what_arg),
          base_uri (base_uri_arg),
          pointer (pointer_arg)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    invalid_schema::invalid_schema (const std::string& base_uri_arg,
                                    const std::string& pointer_arg,
                                    const char* what_arg)
        : std::invalid_argument (what_arg),
          base_uri (base_uri_arg),
          pointer (pointer_arg)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    invalid_schema::invalid_schema (const std::string& what_arg)
        : std::invalid_argument (what_arg)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    invalid_schema::invalid_schema (const char* what_arg)
        : std::invalid_argument (what_arg)
    {
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    invalid_schema::invalid_schema (const invalid_schema& other) noexcept
        : std::invalid_argument (other)
    {
        base_uri = other.base_uri;
        pointer  = other.pointer;
    }


}
