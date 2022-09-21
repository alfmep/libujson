/*
 * Copyright (C) 2020,2022 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef UJSON_MULTIMAP_LIST_HPP
#define UJSON_MULTIMAP_LIST_HPP

#include <map>
#include <list>
#include <mutex>
#include <vector>
#include <stdexcept>
#include <functional>


namespace ujson {

    /**
     * A multimap that keeps items in the order they were inserted.
     * When using an iterator, items are iterated the same order as they were added to the map.
     * There are also iterators that can be used to iterate in a sorted order.<br/>
     * A <code>multimap_list</code> have the following iterators:
     *  - Normal iterators - Items are in the same order as they were added:
     *    - <b>begin</b>  Iterate from the beginning to the end.
     *    - <b>end</b>    Iterator to the end. Used with iterator <b>begin</b>.
     *    - <b>cbegin</b> Same as <b>begin</b> but the iterated items are read-only.
     *    - <b>cend</b>   Const Iterator to the end. Used with iterator <b>cbegin</b>.
     *  - Normal reverse iterators - Items are in the same order as they were added:
     *    - <b>rbegin</b>  Iterate from the end to the beginning.
     *    - <b>rend</b>    Iterator to the end. Used with iterator <b>rbegin</b>.
     *    - <b>crbegin</b> Same as <b>rbegin</b> but the iterated items are read-only.
     *    - <b>crend</b>   Const Iterator to the end. Used with iterator <b>crbegin</b>.
     *  - Sorted iterators - Items are in the sorted order:
     *    - <b>sbegin</b>  Iterate from the beginning to the end.
     *    - <b>send</b>    Iterator to the end. Used with iterator <b>sbegin</b>.
     *    - <b>csbegin</b> Same as <b>begin</b> but the iterated items are read-only.
     *    - <b>csend</b>   Const Iterator to the end. Used with iterator <b>csbegin</b>.
     *  - Sorted reverse iterators - Items are in the sorted order:
     *    - <b>rsbegin</b>  Iterate from the end to the beginning.
     *    - <b>rsend</b>    Iterator to the end. Used with iterator <b>rsbegin</b>.
     *    - <b>crsbegin</b> Same as <b>rbegin</b> but the iterated items are read-only.
     *    - <b>crsend</b>   Const Iterator to the end. Used with iterator <b>crsbegin</b>.
     */
    template<class Key, class T, class CompareKey=std::less<Key>, class CompareValue=std::less<T>>
    class multimap_list {
    private:
        using ItemList = std::list<std::pair<const Key, T>>;
        using KeyRef = std::reference_wrapper<const Key>;
        using KeyRefMap = std::multimap<KeyRef, typename ItemList::iterator, CompareKey>;

    public:
        using key_type = Key;                       /**< Key type. */
        using mapped_type = T;                      /**< Mapped type. */
        using value_type = std::pair<const Key, T>; /**< Value type. */
        using size_type = size_t;                   /**< Size type. */
        using difference_type = ptrdiff_t;          /**< Difference type. */
        using key_compare = CompareKey;             /**< Key compare type. */
        using mapped_compare = CompareValue;        /**< Mapped value compare type. */
        using reference = value_type&;              /**< Reference type to a value. */
        using const_reference = const value_type&;  /**< Const reference type to a value. */

        /**
         * Comparison functor that compares a key-value pair.
         */
        struct value_type_compare {
            /**
             * Return <code>true</code> is lhs is <em>less than</em> rhs (lhs < rhs).
             */
            bool operator() (const value_type& lhs, const value_type& rhs) {
                if (key_compare{}(lhs.first, rhs.first))
                    return true;
                else if (key_compare{}(rhs.first, lhs.first))
                    return false;
                else
                    return mapped_compare{}(lhs.second, rhs.second);
            }
        };


        /**
         * Default constructor.
         * Creates an empty multimap_list object.
         */
        multimap_list () = default;


        /**
         * Constructor.
         * Creates an empty container with a specific
         * function object for key comparisons.
         * @param comp function object to use for all comparisons of keys.
         */
        explicit multimap_list (const key_compare& comp)
            : keys (comp)
        {
        }

        /**
         * Copy constructor.
         * Copy the contents of another multimap_list to this container.
         * @param ml The multimap_list object to copy.
         */
        multimap_list (const multimap_list& ml) {
            std::lock (mutex, ml.mutex);
            std::lock_guard<std::mutex> lg1 (mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lg2 (ml.mutex, std::adopt_lock);
            for (auto& item : ml.items)
                push_back_impl (item.first, item.second);
        }


        /**
         * Move constructor.
         * Move the contents of another multimap_list to this container.
         * @param ml The multimap_list object to move.
         */
        multimap_list (multimap_list&& ml) {
            std::lock (mutex, ml.mutex);
            std::lock_guard<std::mutex> lg1 (mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lg2 (ml.mutex, std::adopt_lock);
            items = std::move (ml.items);
            keys  = std::move (ml.keys);
        }


        /**
         * Constructor.
         * Create a multimap_list objct and copy elements from a range of values.
         * @param first The first value in the range of values to copy.
         * @param last The position after the last value in the range of values to copy.
         * @param comp function object to use for all comparisons of keys.
         */
        template<class InputIt>
        multimap_list (InputIt first,
                       InputIt last,
                       const key_compare& comp=key_compare())
            : keys (comp)
            {
                for (auto i=first; i!=last; ++i)
                    push_back (*i);
            }


        /**
         * Initializer list constructor.
         * Create a multimap_list and initialize the content.
         * @param ilist An initializer list.
         * @param comp function object to use for all comparisons of keys.
         */
        multimap_list (std::initializer_list<value_type> ilist,
                       const key_compare& comp=key_compare())
            : keys (comp)
            {
                for (auto& entry : ilist)
                    push_back (entry);
            }


        /**
         * Destructor.
         */
        ~multimap_list () = default;


        /**
         * Assignment operator.
         * Make this object a copy of another multimap_list object.
         * @param rhs The object to copy.
         */
        multimap_list& operator= (const multimap_list& rhs)
            {
                if (&rhs != this) {
                    std::lock (mutex, rhs.mutex);
                    std::lock_guard<std::mutex> lg1 (mutex, std::adopt_lock);
                    std::lock_guard<std::mutex> lg2 (rhs.mutex, std::adopt_lock);
                    items.clear ();
                    keys.clear ();
                    for (auto& item : rhs.items)
                        push_back_impl (item.first, item.second);
                }
                return *this;
            }


        /**
         * Assignment operator.
         * Assign the contents on an ilitializer list to this object.
         * @param ilist The initializer list to copy.
         */
        multimap_list& operator= (std::initializer_list<value_type> ilist) {
            std::lock_guard<std::mutex> lock (mutex);
            items.clear ();
            keys.clear ();
            for (auto& entry : ilist)
                push_back_impl (entry.first, entry.second);
            return *this;
        }


        /**
         * Move operator.
         * Move the contents of another multimap_list to this object.
         * @param rhs The object to move to this object.
         */
        multimap_list& operator= (multimap_list&& rhs)
            {
                if (&rhs != this) {
                    std::lock (mutex, rhs.mutex);
                    std::lock_guard<std::mutex> lg1 (mutex, std::adopt_lock);
                    std::lock_guard<std::mutex> lg2 (rhs.mutex, std::adopt_lock);
                    items.clear ();
                    keys.clear ();
                    items = std::move (rhs.items);
                    keys  = std::move (rhs.keys);
                }
                return *this;
            }


        /**
         * Check if the container is empty.
         * @return <code>true</code> is this container is empty.
         */
        bool empty () const noexcept {
            std::lock_guard<std::mutex> lock (mutex);
            return items.empty ();
        }


        /**
         * Return the number of entries in the container.
         * @return The number of entries in the container.
         */
        size_type size () const noexcept {
            std::lock_guard<std::mutex> lock (mutex);
            return items.size ();
        }


        /**
         * Clear the container.
         */
        void clear () noexcept {
            std::lock_guard<std::mutex> lock (mutex);
            items.clear ();
            keys.clear ();
        }


    private:
        template<class VType, class ItemListIter, class KeyRefMapIter>
        class iterator_impl {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = VType;
            using difference_type = ptrdiff_t;
            using pointer = VType*;
            using reference = VType&;

            iterator_impl () = default;
            ~iterator_impl () = default;
            //----------------------------------------------------------------------
            iterator_impl (const iterator_impl& iter) {
                sorted = iter.sorted;
                if (sorted)
                    si = iter.si;
                else
                    i = iter.i;
            }
            //----------------------------------------------------------------------
            iterator_impl (ItemListIter iter) {
                sorted = false;
                i = iter;
            }
            //----------------------------------------------------------------------
            iterator_impl (KeyRefMapIter sorted_iter) {
                sorted = true;
                si = sorted_iter;
            }
            //----------------------------------------------------------------------
            iterator_impl operator++ () {
                // ++obj
                if (sorted)
                    ++si;
                else
                    ++i;
                return *this;
            }
            //----------------------------------------------------------------------
            iterator_impl operator++ (int) {
                // obj++
                iterator_impl retval (*this);
                if (sorted)
                    ++si;
                else
                    ++i;
                return retval;
            }
            //----------------------------------------------------------------------
            iterator_impl operator-- () {
                // --obj
                if (sorted)
                    --si;
                else
                    --i;
                return *this;
            }
            //----------------------------------------------------------------------
            iterator_impl operator-- (int) {
                // obj--
                iterator_impl retval (*this);
                if (sorted)
                    --si;
                else
                    --i;
                return retval;
            }
            //----------------------------------------------------------------------
            VType& operator*() {
                if (sorted)
                    return *(si->second);
                else
                    return *i;
            }
            //----------------------------------------------------------------------
            VType* operator->() {
                if (sorted)
                    return &(*(si->second));
                else
                    return &(*i);
            }
            //----------------------------------------------------------------------
            iterator_impl& operator= (const iterator_impl& rhs) {
                if (&rhs != this) {
                    sorted = rhs.sorted;
                    if (sorted)
                        si = rhs.si;
                    else
                        i = rhs.i;
                }
                return *this;
            }
            //----------------------------------------------------------------------
            bool operator== (const iterator_impl& rhs) const {
                if (sorted != rhs.sorted) {
                    throw std::logic_error (
                            (sorted ?
                             "Can't compare sorted with unsorted iterator" :
                             "Can't compare unsorted with sorted iterator"));
                }
                return sorted ? si==rhs.si : i==rhs.i;
            }
            //----------------------------------------------------------------------
            bool operator!= (const iterator_impl& rhs) const {
                return ! this->operator==(rhs);
            }
        private:
            friend class multimap_list;
            bool sorted;
            ItemListIter i;
            KeyRefMapIter si;
        }; // class iterator_impl

    public:
        using iterator               = iterator_impl<value_type,
                                                     typename ItemList::iterator,
                                                     typename KeyRefMap::iterator>; /**< An iterator. */
        using const_iterator         = iterator_impl<const value_type,
                                                     typename ItemList::const_iterator,
                                                     typename KeyRefMap::const_iterator>; /**< A const iterator. */
        using reverse_iterator       = std::reverse_iterator<iterator>; /**< A reverse iterator. */
        using const_reverse_iterator = std::reverse_iterator<const_iterator>; /**< A const reverse iterator. */


        /**
         * Return a reference to the first item in the container.
         * The return value is undefined if the container is empty.
         * @return A reference to the first item in the container.
         */
        reference front () {
            std::lock_guard<std::mutex> lock (mutex);
            return *(begin());
        }


        /**
         * Return a reference to the last item in the container.
         * The return value is undefined if the container is empty.
         * @return A reference to the last item in the container.
         */
        reference back () {
            std::lock_guard<std::mutex> lock (mutex);
            return *(rbegin());
        }


    private:
        void push_back_impl (const key_type& key, const mapped_type& value) {
            items.push_back (std::make_pair(key, value));
            auto i = (++items.rbegin()).base ();
            keys.emplace_hint (keys.end(), std::cref(i->first), i);
        }


    public:
        /**
         * Add an element to the end of the list.
         * @param key The key to add.
         * @param value The mapped value to add.
         */
        void push_back (const key_type& key, const mapped_type& value) {
            std::lock_guard<std::mutex> lock (mutex);
            push_back_impl (std::forward<const key_type&>(key),
                            std::forward<const mapped_type&>(value));
        }


        /**
         * Add an element to the end of the list.
         * @param entry A key and a value to add to the list.
         */
        void push_back (const value_type& entry) {
            push_back (entry.first, entry.second);
        }


        /**
         * Add an element to the end of the list.
         * @param entry A key and a value to add to the list.
         */
        void push_back (value_type&& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            items.push_back (std::forward<value_type&&>(entry));
            auto i = (++items.rbegin()).base ();
            keys.emplace_hint (keys.end(), std::cref(i->first), i);
        }


        /**
         * Prepend an element to beginning of the list.
         * @param key The key to add.
         * @param value The mapped value to add.
         */
        void push_front (const key_type& key, const mapped_type& value) {
            std::lock_guard<std::mutex> lock (mutex);
            items.push_front (std::make_pair(key, value));
            auto i = items.begin ();
            keys.emplace_hint (keys.begin(), std::cref(i->first), i);
        }


        /**
         * Prepend an element to beginning of the list.
         * @param entry A key and a value to add to the list.
         */
        void push_front (const value_type& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            items.push_front (entry);
            auto i = items.begin ();
            keys.emplace_hint (keys.begin(), std::cref(i->first), i);
        }


        /**
         * Prepend an element to beginning of the list.
         * @param entry A key and a value to add to the list.
         */
        void push_front (value_type&& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            items.push_front (std::forward<value_type&&>(entry));
            auto i = items.begin ();
            keys.emplace_hint (keys.begin(), std::cref(i->first), i);
        }


    private:
        typename KeyRefMap::iterator get_key_pos_hint (const key_type& key,
                                                       typename ItemList::iterator item_pos_hint)
            {
                typename KeyRefMap::iterator k_pos = keys.end ();
                auto range = keys.equal_range (key);
                for (auto pos=range.first; pos!=range.second; ++pos) {
                    if (pos->second == item_pos_hint) {
                        k_pos = pos;
                        break;
                    }
                }
                return k_pos;
            }


    public:
        /**
         * Insert an element at a specified location in the container.
         * @param pos The element will be inserted before this position.
         * @param key The key to add.
         * @param value The mapped value to add.
         * @return Iterator pointing to the inserted entry.
         */
        iterator insert (iterator pos, const key_type& key, const mapped_type& value) {
            std::lock_guard<std::mutex> lock (mutex);
            typename ItemList::iterator i_pos = pos.sorted ? pos.si->second : pos.i;
            auto i = items.insert (i_pos, std::make_pair(key, value));
            keys.emplace_hint (get_key_pos_hint(i->first, i_pos),
                               std::cref(i->first), i);
            return iterator (i);
        }


        /**
         * Insert an element at a specified location in the container.
         * @param pos The element will be inserted before this position.
         * @param entry The key-value pair to insert.
         * @return Iterator pointing to the inserted entry.
         */
        iterator insert (iterator pos, const value_type& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            typename ItemList::iterator i_pos = pos.sorted ? pos.si->second : pos.i;
            auto i = items.insert (i_pos, entry);
            keys.emplace_hint (get_key_pos_hint(i->first, i_pos),
                               std::cref(i->first), i);
            return iterator (i);
        }


        /**
         * Insert an element at a specified location in the container.
         * @param pos The element will be inserted before this position.
         * @param entry The key-value pair to insert.
         * @return Iterator pointing to the inserted entry.
         */
        iterator insert (iterator pos, value_type&& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            typename ItemList::iterator i_pos = pos.sorted ? pos.si->second : pos.i;
            auto i = items.insert (i_pos, std::forward<value_type&&>(entry));
            keys.emplace_hint (get_key_pos_hint(i->first, i_pos),
                               std::cref(i->first), i);
            return iterator (i);
        }


        /**
         * Insert elements from a range of elements.
         * @param pos The elements will be inserted before this position.
         * @param first An iterator to the first element to insert.
         * @param last The position after the last element to insert.
         * @return Iterator pointing to the first element inserted,
         *         or <code>pos</code> if the initializer list is empty.
         */
        template<class InputIt>
        iterator insert (iterator pos, InputIt first, InputIt last) {
            if (first == last)
                return pos;
            std::lock_guard<std::mutex> lock (mutex);
            typename ItemList::iterator i_pos = pos.sorted ? pos.si->second : pos.i;
            auto i = items.insert (i_pos, *first);
            auto retval = iterator (i);
            auto key_pos_hint = get_key_pos_hint (i->first, i_pos);
            ++first;
            for (auto entry=first; entry!=last; ++entry) {
                i = items.insert (i, *entry);
                keys.emplace_hint (key_pos_hint, std::cref(i->first), i);
                ++i;
            }
            return retval;
        }


        /**
         * Insert elements from an initializer list at a specified location.
         * @param pos The elements will be inserted before this position.
         * @param ilist Initializer list to insert values from.
         * @return Iterator pointing to the first element inserted,
         *         or <code>pos</code> if the initializer list is empty.
         */
        iterator insert (iterator pos, std::initializer_list<value_type> ilist) {
            return insert (pos, ilist.begin(), ilist.end());
        }


    private:
        //--------------------------------------------------------------------------
        // Assume mutex is locked
        //--------------------------------------------------------------------------
        template<class ...Args>
        iterator emplace_impl (iterator pos, Args&&... args) {
            typename ItemList::iterator i_pos = pos.sorted ? pos.si->second : pos.i;
            auto i = items.emplace (i_pos, std::make_pair(std::forward<Args&&>(args)...));
            keys.emplace_hint (get_key_pos_hint(i->first, i_pos),
                               std::cref(i->first), i);
            return iterator (i);
        }


    public:
        /**
         * Create an element at a specified location in the container.
         * The element is constructed in-place at the specified location.
         * @param pos The element will be inserted before this position.
         * @param args Arguments to the key-value to insert.
         * @return Iterator pointing to the created element.
         */
        template<class ...Args>
        iterator emplace (iterator pos, Args&&... args) {
            std::lock_guard<std::mutex> lock (mutex);
            return emplace_impl (pos, std::forward<Args&&>(args)...);
        }


        /**
         * Create an element at the beginning of the list.
         * @param args Arguments to the key-value to insert.
         * @return A reference to the created element.
         */
        template<class ...Args>
        reference emplace_front (Args&&... args) {
            std::lock_guard<std::mutex> lock (mutex);
            return *(emplace_impl(begin(), std::forward<Args&&>(args)...));
        }


        /**
         * Create an element at the end of the list.
         * @param args Arguments to the key-value to insert.
         * @return A reference to the created element.
         */
        template<class ...Args>
        reference emplace_back (Args&&... args) {
            std::lock_guard<std::mutex> lock (mutex);
            return *(emplace_impl(end(), std::forward<Args&&>(args)...));
        }


    private:
        //--------------------------------------------------------------------------
        // Assume mutex is locked
        //--------------------------------------------------------------------------
        template<class Iter>
        Iter erase_impl (Iter pos) {
            Iter next = pos;
            ++next;
            if (pos.sorted) {
                // KeyRefMap
                items.erase (pos.si->second);
                keys.erase (pos.si);
            }else{
                // ItemList
                auto range = keys.equal_range (pos.i->first);
                for (auto i=range.first; i!=range.second; ++i) {
                    if (i->second == pos.i) {
                        keys.erase (i);
                        break;
                    }
                }
                items.erase (pos.i);
            }
            return next;
        }
        //--------------------------------------------------------------------------
        // Assume mutex is locked
        //--------------------------------------------------------------------------
        template<class Iter>
        Iter erase_impl (Iter first, Iter last) {
            auto next = first;
            while (next != last)
                erase_impl (next++);
            return next;
        }
    public:


        /**
         * Erase an entry at the specified position.
         * @param pos The position of the element to erase.
         * @return Iterator following the removed element.
         */
        iterator erase (iterator pos) {
            std::lock_guard<std::mutex> lock (mutex);
            if (pos.sorted && pos == send()) {
                return send ();
            }
            else if (!pos.sorted && pos == end()) {
                return end ();
            }
            return erase_impl (pos);
        }


        /**
         * Erase an entry at the specified position.
         * @param pos The position of the element to erase.
         * @return Iterator following the removed element.
         */
        const_iterator erase (const_iterator pos) {
            std::lock_guard<std::mutex> lock (mutex);
            if (pos.sorted && pos == csend()) {
                return csend ();
            }
            else if (!pos.sorted && pos == cend()) {
                return cend ();
            }
            return erase_impl (pos);
        }


        /**
         * Erase a range of entries from the container.
         * @param first The position of the first entry to erase.
         * @param last The position after the last entry to erase.
         * @return Iterator following the last removed element.
         */
        iterator erase (iterator first, iterator last) {
            std::lock_guard<std::mutex> lock (mutex);
            return erase_impl (first, last);
        }


        /**
         * Erase a range of entries from the container.
         * @param first The position of the first entry to erase.
         * @param last The position after the last entry to erase.
         * @return Iterator following the last removed element.
         */
        const_iterator erase (const_iterator first, const_iterator last) {
            std::lock_guard<std::mutex> lock (mutex);
            return erase_impl (first, last);
        }


        /**
         * Erase all entries with a specific key.
         * @param key Entries with this key will be erased.
         * @return The number of erased elements.
         */
        size_type erase (const key_type& key) {
            std::lock_guard<std::mutex> lock (mutex);
            auto range = keys.equal_range (key);
            size_type num_erased = 0;
            for (auto si=range.first; si!=range.second; ) {
                items.erase (si->second);
                keys.erase (si++);
                ++num_erased;
            }
            return num_erased;
        }


        /**
         * Erase the first entry in the container.
         */
        void pop_front () {
            std::lock_guard<std::mutex> lock (mutex);
            if (!items.empty())
                erase_impl (begin());
        }


        /**
         * Erase the last entry in the container.
         */
        void pop_back () {
            std::lock_guard<std::mutex> lock (mutex);
            if (!items.empty())
                erase_impl (--(end()));
        }


        /**
         * Swap the content of two containers.
         * @param other The other container to swap content with.
         */
        void swap (multimap_list& other) {
            std::lock (mutex, other.mutex);
            std::lock_guard<std::mutex> lg1 (mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lg2 (other.mutex, std::adopt_lock);
            items.swap (other.items);
            keys.swap (other.keys);
        }


        /**
         * Return the number of elements with a specific key.
         * @return the number of elements with a specific key.
         */
        size_type count (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            return keys.count (key);
        }


        /**
         * Find the first entry with a specific key.
         * @param key The key to search for.
         * @return Iterator to the first entry with the key,
         *         or end() if no entry with the specified key.
         */
        iterator find (const key_type& key) {
            std::lock_guard<std::mutex> lock (mutex);
            auto si = keys.find (key);
            if (si == keys.end())
                return end ();
            else
                return iterator (si->second);
        }


        /**
         * Check if there is at least one entry with a specific key.
         * @return <code>true</code> if there is an entry with the key.
         */
        bool contains (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            return keys.find(key) != keys.end();
        }


        /**
         * Find all entries with a specific key.
         * @param key The key to search for.
         * @return A range of entries with the specific key.
         */
        std::pair<iterator, iterator> equal_range (const key_type& key) {
            std::lock_guard<std::mutex> lock (mutex);
            auto range = keys.equal_range (key);
            return std::make_pair (iterator(range.first), iterator(range.second));
        }


        /**
         * Find all entries with a specific key.
         * @param key The key to search for.
         * @return A range of entries with the specific key.
         */
        std::pair<const_iterator, const_iterator> equal_range (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            auto range = keys.equal_range (key);
            return std::make_pair (const_iterator(range.first), const_iterator(range.second));
        }


        /**
         * Return the position of the first element with
         * a key <em>not less</em> than the pecified key.
         * @return Iterator to an element,
         *         or <code>send()</code> if none found.
         */
        iterator lower_bound (const key_type& key) {
            std::lock_guard<std::mutex> lock (mutex);
            return iterator (keys.lower_bound(key));
        }


        /**
         * Return the position of the first element with
         * a key <em>not less</em> than the pecified key.
         * @return Iterator to an element,
         *         or <code>send()</code> if none found.
         */
        const_iterator lower_bound (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            return const_iterator (keys.lower_bound(key));
        }


        /**
         * Return the position of the first element
         * with a key <em>greater</em> than the pecified key.
         * @return Iterator to an element,
         *         or <code>send()</code> if none found.
         */
        iterator upper_bound (const key_type& key) {
            std::lock_guard<std::mutex> lock (mutex);
            return iterator (keys.upper_bound(key));
        }


        /**
         * Return the position of the first element
         * with a key <em>greater</em> than the pecified key.
         * @return Iterator to an element,
         *         or <code>send()</code> if none found.
         */
        const_iterator upper_bound (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            return const_iterator (keys.upper_bound(key));
        }


        /**
         * Check if two containers are equal.
         * Two containers are equal if:<br/>
         * <ul>
         *   <li>They have the same number of elements.</li>
         *   <li>All key-value elements in one container are present in the other.</li>
         * </ul>
         * The key-value elements need not be in the same order in the two containers.
         * @param rhs The other container to compare with.
         * @return <code>true</code> if the containers are equivalent.
         */
        bool operator== (const multimap_list& rhs) const {
            if (this == &rhs)
                return true;
            std::lock (mutex, rhs.mutex);
            std::lock_guard<std::mutex> lg1 (mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lg2 (rhs.mutex, std::adopt_lock);
            if (items.size() != rhs.items.size())
                return false;

            auto lhs_i = sbegin ();
            auto rhs_i = rhs.sbegin ();
            key_compare key_less;
            mapped_compare value_less;
            while (lhs_i != send()) {
                if ( key_less(lhs_i->first, rhs_i->first) ||
                     key_less(rhs_i->first, lhs_i->first) ||
                     value_less(lhs_i->second, rhs_i->second) ||
                     value_less(rhs_i->second, lhs_i->second))
                {
                    return false;
                }
                ++lhs_i;
                ++rhs_i;
            }
            return true;
        }


        /**
         * Check if two containers are not equal.
         * @param rhs The container to compare. The right-hand-side of '!='.
         * @return <code>true</code> if the other container
         *         is equal to this, otherwise <code>false</code>.
         */
        bool operator!= (const multimap_list& rhs) const {
            return ! this->operator==(rhs);
        }


        /**
         * Compare the contents of two containers lexicographically.<br/>
         * The two containers are compared using sorted keys.
         * @param rhs The other container to compare with. The right-hand side of '<'.
         * @return <code>true</code> if this container is lexicographically less than the other.
         */
        bool operator< (const multimap_list& rhs) const {
            if (this == &rhs)
                return false;
            std::lock (mutex, rhs.mutex);
            std::lock_guard<std::mutex> lg1 (mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lg2 (rhs.mutex, std::adopt_lock);
            return std::lexicographical_compare (sbegin(), send(),
                                                 rhs.sbegin(), rhs.send(),
                                                 value_type_compare{});
        }


        /**
         * Return an iterator to the first entry of the list of elements.
         */
        iterator begin () {
            return iterator (items.begin());
        }


        /**
         * Return a const iterator to the first entry of the list of elements.
         */
        const_iterator begin () const {
            return const_iterator (items.cbegin());
        }


        /**
         * Return a const iterator to the first entry of the list of elements.
         */
        const_iterator cbegin () const {
            return const_iterator (items.cbegin());
        }


        /**
         * Return a reverse iterator to the first entry of the reversed list of elements.
         */
        reverse_iterator rbegin () {
            return std::make_reverse_iterator (end());
        }


        /**
         * Return a const reverse iterator to the first entry of the reversed list of elements.
         */
        const_reverse_iterator rbegin () const {
            return std::make_reverse_iterator (cend());
        }


        /**
         * Return a const reverse iterator to the first entry of the reversed list of elements.
         */
        const_reverse_iterator crbegin () const {
            return std::make_reverse_iterator (cend());
        }


        /**
         * Return a sorted iterator to the first entry of the sorted list of elements.
         * The iterator is sorted by key.
         */
        iterator sbegin () {
            return iterator (keys.begin());
        }


        /**
         * Return a sorted const iterator to the first entry of the sorted list of elements.
         * The iterator is sorted by key.
         */
        const_iterator sbegin () const {
            return const_iterator (keys.cbegin());
        }


        /**
         * Return a sorted const iterator to the first entry of the sorted list of elements.
         * The iterator is sorted by key.
         */
        const_iterator csbegin () const {
            return const_iterator (keys.cbegin());
        }


        /**
         * Return a reversed sorted iterator to the first entry of the reversed sorted list of elements.
         * The iterator is sorted by key.
         */
        reverse_iterator rsbegin () {
            return std::make_reverse_iterator (send());
        }


        /**
         * Return a const reversed sorted iterator to the first entry of the reversed sorted list of elements.
         * The iterator is sorted by key.
         */
        const_reverse_iterator rsbegin () const {
            return std::make_reverse_iterator (csend());
        }


        /**
         * Return a const reversed sorted iterator to the first entry of the reversed sorted list of elements.
         * The iterator is sorted by key.
         */
        const_reverse_iterator crsbegin () const {
            return std::make_reverse_iterator (csend());
        }


        /**
         * Return an iterator to the element following the last entry of the list of elements.
         */
        iterator end () {
            return iterator (items.end());
        }


        /**
         * Return a const iterator to the element following the last entry of the list of elements.
         */
        const_iterator end () const {
            return const_iterator (items.cend());
        }


        /**
         * Return a const iterator to the element following the last entry of the list of elements.
         */
        const_iterator cend () const {
            return const_iterator (items.cend());
        }


        /**
         * Return a reverse iterator to the element following the last entry of the reversed list of elements.
         */
        reverse_iterator rend () {
            return std::make_reverse_iterator (begin());
        }


        /**
         * Return a const reverse iterator to the element following the last entry of the reversed list of elements.
         */
        const_reverse_iterator rend () const {
            return std::make_reverse_iterator (cbegin());
        }


        /**
         * Return a const reverse iterator to the element following the last entry of the reversed list of elements.
         */
        const_reverse_iterator crend () const {
            return std::make_reverse_iterator (cbegin());
        }


        /**
         * Return a sorted iterator to the element following the last sorted element.
         */
        iterator send () {
            return iterator (keys.end());
        }


        /**
         * Returns a sorted const iterator to the element following the last sorted element.
         */
        const_iterator send () const {
            return const_iterator (keys.cend());
        }


        /**
         * Returns a sorted const iterator to the element following the last sorted element.
         */
        const_iterator csend () const {
            return const_iterator (keys.cend());
        }


        /**
         * Returns a reversed sorted iterator to the element following the last reversed sorted element.
         */
        reverse_iterator rsend () {
            return std::make_reverse_iterator (sbegin());
        }


        /**
         * Returns a reversed sorted const iterator to the element following the last reversed sorted element.
         */
        const_reverse_iterator rsend () const {
            return std::make_reverse_iterator (csbegin());
        }


        /**
         * Returns a reversed sorted const iterator to the element following the last reversed sorted element.
         */
        const_reverse_iterator crsend () const {
            return std::make_reverse_iterator (csbegin());
        }


    private:
        ItemList items;
        KeyRefMap keys;
        mutable std::mutex mutex;
    };


}

#endif
