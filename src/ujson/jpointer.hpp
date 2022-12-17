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
#ifndef UJSON_JPOINTER_HPP
#define UJSON_JPOINTER_HPP

#include <ujson/jvalue.hpp>
#include <string>
#include <list>


namespace ujson {


    /**
     * A JSON pointer.
     */
    class jpointer {
    public:
        using tokens_t = std::list<std::string>; /**< List of tokens in the JSON pointer. */

        jpointer () = default; /**< Default constructor. Create an empty JSON pointer. */
        jpointer (const jpointer& pointer) : tokens(pointer.tokens) {} /**< Copy constructor. */
        jpointer (jpointer&& pointer) : tokens(std::move(pointer.tokens)) {} /**< Move constructor. */

        /** Construct a JSON pointer by parsing a string.
            @throw std::invalid_argument if the pointer string isn't a valid JSON pointer.
         */
        jpointer (const std::string& pointer_string);

        jpointer (const json_array& array_of_tokens); /**< Construct a JSON pointer from an array of string tokens. */

        ~jpointer () = default; /**< Destructor. */

        jpointer& operator= (const jpointer& pointer); /**< Assignment operator. */
        jpointer& operator= (jpointer&& pointer); /**< Move operator. */

        /** Assign a JSON pointer by parsing a string.
            @throw std::invalid_argument if the pointer string isn't a valid JSON pointer.
         */
        jpointer& operator= (const std::string& pointer_string);

        jpointer& operator= (const json_array& array_of_tokens); /**< Assign a JSON pointer from an array of tokens. */

        /** Parse a string representation of a JSON pointer.
            @throw std::invalid_argument if the pointer string isn't a valid JSON pointer.
         */
        void parse (const std::string& pointer_string);

        std::string str () const; /**< Return a string representation of the JSON pointer. */
        inline operator std::string() const {return str();} /**< Return a string representation of the JSON pointer. */

        inline size_t size () const  {return tokens.size();} /**< Return the number of tokens in the JSON pointer. */
        inline bool empty () const   {return tokens.empty();} /**< Return <code>true</code> if the JSON pointer is empty. */
        inline void clear ()         {tokens.clear();} /**< Clear the JSON pointer. */

        inline std::string& front () {return tokens.front();} /**< Access the first token. */
        inline std::string& back ()  {return tokens.back();} /**< Access the last token. */

        inline void push_back (const std::string& token)  {tokens.push_back(token);} /**< Append a token.  */
        inline void emplace_back (std::string&& token)    {tokens.emplace_back(std::forward<std::string>(token));}  /**< Append a token. */
        inline void pop_back ()                           {tokens.pop_back();} /**< Remove the last token. */

        inline void push_front (const std::string& token) {tokens.push_front(token);} /**< Prepend a token. */
        inline void emplace_front (std::string&& token)   {tokens.emplace_front(std::forward<std::string>(token));} /**< Prepend a token. */
        inline void pop_front ()                          {tokens.pop_front();} /**< Remove the first token. */

        inline tokens_t::iterator               begin () noexcept         {return tokens.begin();} /**< Iterator to the first token. */
        inline tokens_t::const_iterator         begin () const noexcept   {return tokens.begin();} /**< Iterator to the first token. */
        inline tokens_t::const_iterator         cbegin () const noexcept  {return tokens.cbegin();} /**< Iterator to the first token. */
        inline tokens_t::iterator               end () noexcept           {return tokens.end();} /**< Iterator past the last token. */
        inline tokens_t::const_iterator         end () const noexcept     {return tokens.end();} /**< Iterator past the last token. */
        inline tokens_t::const_iterator         cend () const noexcept    {return tokens.cend();} /**< Iterator past the last token. */

        inline tokens_t::reverse_iterator       rbegin () noexcept        {return tokens.rbegin();} /**< Reverse iterator to the first token. */
        inline tokens_t::const_reverse_iterator rbegin () const noexcept  {return tokens.rbegin();} /**< Reverse iterator to the first token. */
        inline tokens_t::const_reverse_iterator crbegin () const noexcept {return tokens.crbegin();} /**< Reverse iterator to the first token. */
        inline tokens_t::reverse_iterator       rend () noexcept          {return tokens.rend();} /**< Reverse iterator past the last token. */
        inline tokens_t::const_reverse_iterator rend () const noexcept    {return tokens.rend();} /**< Reverse iterator past the last token. */
        inline tokens_t::const_reverse_iterator crend () const noexcept   {return tokens.crend();} /**< Reverse iterator past the last token. */

        inline tokens_t& token_list () {return tokens;} /**< Return the underlaying list of tokne strings. */


    private:
        tokens_t tokens;
    };


}
#endif
