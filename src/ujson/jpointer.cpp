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
#include <ujson/jpointer.hpp>
#include <ujson/utils.hpp>
#include <stdexcept>


namespace ujson {


    /**
     * Get (possible emtpy) token on the right hand side of a delimeter.
     * And, of course, all other tokens as well.
     *
     * "/token_1/token_2/"
     *                   ^ token 3 - empty string
     */
    class rhs_tokenizer {
    public:
        rhs_tokenizer (const std::string& str, const char delim);
        bool operator () (std::string& token); // Next token in 'tok', return false if no more tokens available.
    private:
        const char* ptr;
        const char delim;
    };
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    rhs_tokenizer::rhs_tokenizer (const std::string& str, const char delim)
        : ptr (str.c_str()),
          delim (delim)
    {
        if (str.empty())
            ptr = nullptr;
        else if (ptr[0] == delim)
            ++ptr;
    }
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool rhs_tokenizer::operator () (std::string& token)
    {
        token.clear ();
        if (!ptr)
            return false;
        while (*ptr!=delim && *ptr!='\0')
            token.push_back (*ptr++);
        if (*ptr == '\0')
            ptr = nullptr;
        else
            ++ptr;
        return true;
    }




    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jpointer::jpointer (const std::string& pointer)
    {
        parse (pointer);
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jpointer::jpointer (const json_array& array_of_tokens)
    {
        for (auto& token : array_of_tokens)
            push_back (const_cast<jvalue&>(token).str());
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jpointer& jpointer::operator= (const jpointer& pointer)
    {
        if (&pointer != this)
            tokens = pointer.tokens;
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jpointer& jpointer::operator= (jpointer&& pointer)
    {
        if (&pointer != this)
            tokens = std::move (pointer.tokens);
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jpointer& jpointer::operator= (const std::string& pointer_string)
    {
        parse (pointer_string);
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    jpointer& jpointer::operator= (const json_array& array_of_tokens)
    {
        clear ();
        for (auto& token : array_of_tokens)
            push_back (const_cast<jvalue&>(token).str());
        return *this;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void jpointer::parse (const std::string& pointer_string)
    {
        if (!pointer_string.empty()  &&  pointer_string[0] != '/') {
            // A non empty pointer must start with '/'
            throw std::invalid_argument ("Invalid JSON pointer");
        }
        clear ();
        rhs_tokenizer get_next_token (pointer_string, '/');
        std::string token;
        while (get_next_token(token))
            emplace_back (unescape_pointer_token(token));
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::string jpointer::str () const
    {
        std::string pointer;
        for (auto& token : tokens) {
            pointer.append ("/");
            pointer.append (escape_pointer_token(token));
        }
        return pointer;
    }


}
