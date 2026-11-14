/*
 * Copyright (C) 2020,2022,2026 Dan Arrhenius <dan@ultramarin.se>
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
#include <algorithm>
#include <memory>

namespace ujson {

    /**
     * A multimap that keeps items in the order they were inserted.
     * When using an iterator, items are iterated the same order as they were added to the map.
     * There are also iterators that can be used to iterate in sorted order.<br/>
     * A <code>multimap_list</code> has the following iterators:
     *  - Normal iterators - Items are seen in the same order as they were inserted:
     *    - <b>begin()</b>  Iterate from the beginning to the end.
     *    - <b>end()</b>    Iterator to the end. Used with iterator <b>begin()</b>.
     *    - <b>cbegin()</b> Same as <b>begin()</b> but the iterated items are read-only.
     *    - <b>cend()</b>   Const Iterator to the end. Used with iterator <b>cbegin()</b>.
     *  - Normal reverse iterators - Items are seen in the same order as they were inserted:
     *    - <b>rbegin()</b>  Iterate from the end to the beginning.
     *    - <b>rend()</b>    Iterator to the end. Used with iterator <b>rbegin()</b>.
     *    - <b>crbegin()</b> Same as <b>rbegin()</b> but the iterated items are read-only.
     *    - <b>crend()</b>   Const Iterator to the end. Used with iterator <b>crbegin()</b>.
     *  - Sorted iterators - Items are seen in sorted order:
     *    - <b>sbegin()</b>  Iterate from the beginning to the end.
     *    - <b>send()</b>    Iterator to the end. Used with iterator <b>sbegin()</b>.
     *    - <b>csbegin()</b> Same as <b>sbegin()</b> but the iterated items are read-only.
     *    - <b>csend()</b>   Const Iterator to the end. Used with iterator <b>csbegin()</b>.
     *  - Sorted reverse iterators - Items are seen in sorted order:
     *    - <b>rsbegin()</b>  Iterate from the end to the beginning.
     *    - <b>rsend()</b>    Iterator to the end. Used with iterator <b>rsbegin()</b>.
     *    - <b>crsbegin()</b> Same as <b>rbegin()</b> but the iterated items are read-only.
     *    - <b>crsend()</b>   Const Iterator to the end. Used with iterator <b>crsbegin()</b>.
     */
    template<class Key,
             class T,
             class CompareKey = std::less<Key>,
             class CompareMapped = std::less<T>,
             class ValueAllocator = std::allocator<std::pair<const Key, T>>,
             class KeyRefAllocator = std::allocator<std::pair<const std::reference_wrapper<const Key>,
                                                              typename std::list<std::pair<const Key, T>>::iterator>>
             >
    class multimap_list {
    private:
        using ItemList = std::list<std::pair<const Key, T>, ValueAllocator>;
        using KeyRef = std::reference_wrapper<const Key>;
        using KeyRefMap = std::multimap<KeyRef,
                                        typename ItemList::iterator,
                                        CompareKey,
                                        KeyRefAllocator>;

    public:
        using key_type = Key;                       /**< Key type. */
        using mapped_type = T;                      /**< Mapped type. */
        using value_type = std::pair<const Key, T>; /**< Value type. */
        using size_type = size_t;                   /**< Size type. */
        using difference_type = ptrdiff_t;          /**< Difference type. */
        using key_compare = CompareKey;             /**< Key compare type. */
        using mapped_compare = CompareMapped;       /**< Mapped value compare type. */
        using reference = value_type&;              /**< Reference type to a value. */
        using const_reference = const value_type&;  /**< Const reference type to a value. */

        /**
         * Comparison functor that compares a key-value pair.
         */
        struct value_compare {
            key_compare key_comp;
            mapped_compare value_comp;
            /**
             * Return <code>true</code> is lhs is <em>less than</em> rhs (lhs < rhs).
             */
            bool operator() (const value_type& lhs, const value_type& rhs) {
                if (key_comp(lhs.first, rhs.first))
                    return true;
                else if (key_comp(rhs.first, lhs.first))
                    return false;
                else
                    return value_comp(lhs.second, rhs.second);
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
            std::scoped_lock lock (mutex, ml.mutex);
            for (auto& item : ml.items)
                push_back_impl (item);
        }


        /**
         * Move constructor.
         * Move the contents of another multimap_list to this container.
         * @param ml The multimap_list object to move.
         */
        multimap_list (multimap_list&& ml) {
            std::scoped_lock lock (mutex, ml.mutex);
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
                    push_back_impl (*i);
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
                    push_back_impl (entry);
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
        multimap_list& operator= (const multimap_list& rhs) {
            if (&rhs != this) {
                std::scoped_lock lock (mutex, rhs.mutex);
                items.clear ();
                keys.clear ();
                for (auto& item : rhs.items)
                    push_back_impl (item);
            }
            return *this;
        }


        /**
         * Move operator.
         * Move the contents of another multimap_list to this object.
         * @param rhs The object to move to this object.
         */
        multimap_list& operator= (multimap_list&& rhs) {
            if (&rhs != this) {
                std::scoped_lock lock (mutex, rhs.mutex);
                items.clear ();
                keys.clear ();
                items = std::move (rhs.items);
                keys  = std::move (rhs.keys);
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
                push_back_impl (entry);
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
         * Return the maximum number of entries the container is able to hold.
         * @return The maximum number of entries the container is able to hold.
         */
        size_type max_size () const noexcept {
            return std::min (items.max_size(), keys.max_size());
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
        //----------------------------------------------------------------------
        // Const Iterator
        //----------------------------------------------------------------------
        template<class VType, class ItemListConstIter, class KeyRefMapConstIter>
        class const_iterator_impl {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = const VType;
            using difference_type = ptrdiff_t;
            using pointer = const VType*;
            using reference = const VType&;

            const_iterator_impl () = default;
            ~const_iterator_impl () = default;
            //----------------------------------------------------------------------
            const_iterator_impl (const const_iterator_impl& iter) {
                sorted = iter.sorted;
                if (sorted)
                    si = iter.si;
                else
                    i = iter.i;
            }
            //----------------------------------------------------------------------
            const_iterator_impl (ItemListConstIter iter) {
                sorted = false;
                i = iter;
            }
            //----------------------------------------------------------------------
            const_iterator_impl (KeyRefMapConstIter sorted_iter) {
                sorted = true;
                si = sorted_iter;
            }
            //----------------------------------------------------------------------
            const_iterator_impl& operator= (const const_iterator_impl& iter) {
                if (this != &iter) {
                    sorted = iter.sorted;
                    if (sorted)
                        si = iter.si;
                    else
                        i = iter.i;
                }
                return *this;
            }
            //----------------------------------------------------------------------
            const_iterator_impl operator++ () {
                // ++obj
                if (sorted)
                    ++si;
                else
                    ++i;
                return *this;
            }
            //----------------------------------------------------------------------
            const_iterator_impl operator++ (int) {
                // obj++
                const_iterator_impl retval (*this);
                if (sorted)
                    ++si;
                else
                    ++i;
                return retval;
            }
            //----------------------------------------------------------------------
            const_iterator_impl operator-- () {
                // --obj
                if (sorted)
                    --si;
                else
                    --i;
                return *this;
            }
            //----------------------------------------------------------------------
            const_iterator_impl operator-- (int) {
                // obj--
                const_iterator_impl retval (*this);
                if (sorted)
                    --si;
                else
                    --i;
                return retval;
            }
            //----------------------------------------------------------------------
            reference operator*() const {
                if (sorted)
                    return *(si->second);
                else
                    return *i;
            }
            //----------------------------------------------------------------------
            pointer operator->() const {
                if (sorted)
                    return &(*(si->second));
                else
                    return &(*i);
            }
            //----------------------------------------------------------------------
            bool operator== (const const_iterator_impl& rhs) const {
                if (sorted != rhs.sorted) {
                    throw std::logic_error (
                            (sorted ?
                             "Can't compare sorted with unsorted iterator" :
                             "Can't compare unsorted with sorted iterator"));
                }
                return sorted ? si==rhs.si : i==rhs.i;
            }
            //----------------------------------------------------------------------
            bool operator!= (const const_iterator_impl& rhs) const {
                return ! this->operator==(rhs);
            }
        private:
            friend class multimap_list;
            ItemListConstIter i;
            KeyRefMapConstIter si;
            bool sorted;
        }; // class const_iterator_impl


        //----------------------------------------------------------------------
        // Iterator
        //----------------------------------------------------------------------
        template<class VType,
                 class ItemListConstIter,
                 class KeyRefMapConstIter,
                 class ItemListIter,
                 class KeyRefMapIter
                 >
        class iterator_impl {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = VType;
            using difference_type = ptrdiff_t;
            using pointer = VType*;
            using reference = VType&;
            using const_iterator = const_iterator_impl<VType, ItemListConstIter, KeyRefMapConstIter>;

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
            iterator_impl& operator= (const iterator_impl& iter) {
                if (this != &iter) {
                    sorted = iter.sorted;
                    if (sorted)
                        si = iter.si;
                    else
                        i = iter.i;
                }
                return *this;
            }
            //----------------------------------------------------------------------
            operator const_iterator() const {
                if (sorted)
                    return const_iterator ((KeyRefMapConstIter)si);
                else
                    return const_iterator ((ItemListConstIter)i);
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
            reference operator*() const {
                if (sorted)
                    return *(si->second);
                else
                    return *i;
            }
            //----------------------------------------------------------------------
            pointer operator->() const {
                if (sorted)
                    return &(*(si->second));
                else
                    return &(*i);
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
            ItemListIter i;
            KeyRefMapIter si;
            bool sorted;
        }; // class iterator_impl

    public:
        using iterator = iterator_impl<value_type,
                                       typename ItemList::const_iterator,
                                       typename KeyRefMap::const_iterator,
                                       typename ItemList::iterator,
                                       typename KeyRefMap::iterator
                                       >; /**< An iterator. */
        using const_iterator = const_iterator_impl<value_type,
                                                   typename ItemList::const_iterator,
                                                   typename KeyRefMap::const_iterator
                                                   >; /**< A const iterator. */
        using reverse_iterator = std::reverse_iterator<iterator>; /**< A reverse iterator. */
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
         * Return a const reference to the first item in the container.
         * The return value is undefined if the container is empty.
         * @return A const reference to the first item in the container.
         */
        const_reference front () const {
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


        /**
         * Return a const reference to the last item in the container.
         * The return value is undefined if the container is empty.
         * @return A const reference to the last item in the container.
         */
        const_reference back () const {
            std::lock_guard<std::mutex> lock (mutex);
            return *(rbegin());
        }


    private:
        inline void push_back_impl (const key_type& key, const mapped_type& value) {
            items.emplace_back (std::make_pair(key, value));
            auto i = --(items.end());
            keys.emplace_hint (keys.end(), std::cref(i->first), i);
        }
        inline void push_back_impl (const key_type& key, mapped_type&& value) {
            items.emplace_back (std::make_pair(key, std::forward<mapped_type>(value)));
            auto i = --(items.end());
            keys.emplace_hint (keys.end(), std::cref(i->first), i);
        }
        inline void push_back_impl (const value_type& entry) {
            items.push_back (entry);
            auto i = --(items.end());
            keys.emplace_hint (keys.end(), std::cref(i->first), i);
        }
        inline void push_back_impl (value_type&& entry) {
            items.emplace_back (std::forward<value_type>(entry));
            auto i = --(items.end());
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
            push_back_impl (key, value);
        }

        /**
         * Add an element to the end of the list.
         * @param key The key to add.
         * @param value The mapped value to add.
         */
        void push_back (const key_type& key, mapped_type&& value) {
            std::lock_guard<std::mutex> lock (mutex);
            push_back_impl (key, std::forward<mapped_type>(value));
        }


        /**
         * Add an element to the end of the list.
         * @param entry A key and a value to add to the list.
         */
        void push_back (const value_type& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            push_back_impl (entry);
        }


        /**
         * Add an element to the end of the list.
         * @param entry A key and a value to add to the list.
         */
        void push_back (value_type&& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            push_back_impl (std::forward<value_type>(entry));
        }


        /**
         * Prepend an element to beginning of the list.
         * @param key The key to add.
         * @param value The mapped value to add.
         */
        void push_front (const key_type& key, const mapped_type& value) {
            std::lock_guard<std::mutex> lock (mutex);
            items.emplace_front (std::make_pair(key, value));
            auto i = items.begin ();
            keys.emplace_hint (keys.begin(), std::cref(i->first), i);
        }


        /**
         * Prepend an element to beginning of the list.
         * @param key The key to add.
         * @param value The mapped value to add.
         */
        void push_front (const key_type& key, mapped_type&& value) {
            std::lock_guard<std::mutex> lock (mutex);
            items.emplace_front (key, std::forward<mapped_type>(value));
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
            items.emplace_front (std::forward<value_type>(entry));
            auto i = items.begin ();
            keys.emplace_hint (keys.begin(), std::cref(i->first), i);
        }

    private:
        typename KeyRefMap::iterator get_key_pos_hint (const key_type& key,
                                                       const_iterator& pos)
            {
                key_compare k_less;
                if (k_less(key, pos.i->first)) {
                    return keys.end ();
                }else if (k_less(pos.i->first, key)) {
                    return keys.begin ();
                }else{
                    auto kir = keys.equal_range (key);
                    for (auto ki=kir.first; ki!=kir.second; ++ki) {
                        if (ki->second == pos.i)
                            return ki;
                    }
                }
                return keys.begin ();
            }

    public:
        /**
         * Insert an element at a specified location in the container.
         * @param pos The element will be inserted before this position.
         * @param key The key to add.
         * @param value The mapped value to add.
         * @return Iterator pointing to the inserted entry.
         */
        iterator insert (const_iterator pos, const key_type& key, const mapped_type& value) {
            std::lock_guard<std::mutex> lock (mutex);
            if (pos.sorted) {
                auto ui = items.emplace (pos.si->second, key, value);
                auto si = keys.emplace_hint (pos.si, std::cref(ui->first), ui);
                return iterator (si);
            }else{
                typename ItemList::iterator i;
                if (pos.i == items.cbegin()) {
                    items.emplace_front (key, value);
                    i = items.begin ();
                    keys.emplace_hint (keys.begin(), std::cref(i->first), i);
                }else if (pos.i == items.cend()) {
                    push_back_impl (key, value);
                    i = --items.end();
                }else{
                    i = items.emplace (pos.i, key, value);
                    keys.emplace_hint (get_key_pos_hint(key, pos),
                                       std::cref(i->first), i);
                }
                return iterator (i);
            }
        }


        /**
         * Insert an element at a specified location in the container.
         * @param pos The element will be inserted before this position.
         * @param key The key to add.
         * @param value The mapped value to add.
         * @return Iterator pointing to the inserted entry.
         */
        iterator insert (const_iterator pos, const key_type& key, mapped_type&& value) {
            std::lock_guard<std::mutex> lock (mutex);
            if (pos.sorted) {
                auto ui = items.emplace (pos.si->second, key, std::forward<mapped_type>(value));
                auto si = keys.emplace_hint (pos.si, std::cref(ui->first), ui);
                return iterator (si);
            }else{
                typename ItemList::iterator i;
                if (pos.i == items.cbegin()) {
                    items.emplace_front (key, std::forward<mapped_type>(value));
                    i = items.begin ();
                    keys.emplace_hint (keys.begin(), std::cref(i->first), i);
                }else if (pos.i == items.cend()) {
                    push_back_impl (key, std::forward<mapped_type>(value));
                    i = --items.end();
                }else{
                    i = items.emplace (pos.i, key, std::forward<mapped_type>(value));
                    keys.emplace_hint (get_key_pos_hint(key, pos),
                                       std::cref(i->first), i);
                }
                return iterator (i);
            }
        }


        /**
         * Insert an element at a specified location in the container.
         * @param pos The element will be inserted before this position.
         * @param entry The key-value pair to insert.
         * @return Iterator pointing to the inserted entry.
         */
        iterator insert (const_iterator pos, const value_type& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            if (pos.sorted) {
                auto ui = items.emplace (pos.si->second, entry);
                auto si = keys.emplace_hint (pos.si, std::cref(ui->first), ui);
                return iterator (si);
            }else{
                typename ItemList::iterator i;
                if (pos.i == items.cbegin()) {
                    items.push_front (entry);
                    i = items.begin ();
                    keys.emplace_hint (keys.begin(), std::cref(i->first), i);
                }else if (pos.i == items.cend()) {
                    push_back_impl (entry);
                    i = --items.end();
                }else{
                    i = items.emplace (pos.i, entry);
                    keys.emplace_hint (get_key_pos_hint(entry.first, pos),
                                       std::cref(i->first), i);
                }
                return iterator (i);
            }
        }


        /**
         * Insert an element at a specified location in the container.
         * @param pos The element will be inserted before this position.
         * @param entry The key-value pair to insert.
         * @return Iterator pointing to the inserted entry.
         */
        iterator insert (const_iterator pos, value_type&& entry) {
            std::lock_guard<std::mutex> lock (mutex);
            if (pos.sorted) {
                auto ui = items.emplace (pos.si->second, std::forward<value_type>(entry));
                auto si = keys.emplace_hint (pos.si, std::cref(ui->first), ui);
                return iterator (si);
            }else{
                typename ItemList::iterator i;
                if (pos.i == items.cbegin()) {
                    items.emplace_front (std::forward<value_type>(entry));
                    i = items.begin ();
                    keys.emplace_hint (keys.begin(), std::cref(i->first), i);
                }else if (pos.i == items.cend()) {
                    push_back_impl (std::forward<value_type>(entry));
                    i = --items.end();
                }else{
                    i = items.emplace (pos.i, std::forward<value_type>(entry));
                    keys.emplace_hint (get_key_pos_hint(entry.first, pos),
                                       std::cref(i->first), i);
                }
                return iterator (i);
            }
        }


        /**
         * Insert elements from a range of elements.
         * @param pos The elements will be inserted before this position.
         * @param first An iterator to the first element to insert.
         * @param last The position after the last element to insert.
         * @return Iterator pointing to the first element inserted.
         */
        template<class InputIt>
        iterator insert (const_iterator pos, InputIt first, InputIt last) {
            std::lock_guard<std::mutex> lock (mutex);
            if (first == last) {
                if (pos.sorted)
                    return iterator (keys.end());
                else
                    return iterator (items.end());
            }

            if (pos.sorted) {
                auto ui = items.emplace (pos.si->second, *first);
                auto si = keys.emplace_hint (pos.si, std::cref(ui->first), ui);
                ++first;
                while (first != last) {
                    auto ui = items.emplace (pos.si->second, *first);
                    keys.emplace_hint (pos.si, std::cref(ui->first), ui);
                    ++first;
                }
                return iterator (si);
            }else{
                typename ItemList::iterator ui;
                if (pos.i == items.cend()) {
                    ui = items.insert (items.end(), first, last);
                    auto i = ui;
                    do {
                        keys.emplace_hint (keys.end(), std::cref(i->first), i);
                        ++first;
                        ++i;
                    }while (first != last);
                    return iterator (ui);
                }
                typename KeyRefMap::iterator hint;
                if (pos.i == items.cbegin()) {
                    ui = items.insert (items.cbegin(), first, last);
                    hint = keys.begin ();
                }else{
                    ui = items.insert (pos.i, first, last);
                    auto kir = keys.equal_range (pos->first);
                    for (auto ki=kir.first; ki!=kir.second; ++ki) {
                        if (ki->second == pos.i) {
                            hint = ki;
                            break;
                        }
                    }
                }
                auto i = ui;
                do {
                    keys.emplace_hint (hint, std::cref(i->first), i);
                    ++first;
                    ++i;
                }while (first != last);
                return iterator (ui);
            }
        }


        /**
         * Insert elements from an initializer list at a specified location.
         * @param pos The elements will be inserted before this position.
         * @param ilist Initializer list to insert values from.
         * @return Iterator pointing to the first element inserted,
         *         or <code>pos</code> if the initializer list is empty.
         */
        iterator insert (const_iterator pos, std::initializer_list<value_type> ilist) {
            return insert (pos, ilist.begin(), ilist.end());
        }


        /**
         * Create an element at a specified location in the container.
         * The element is constructed in-place at the specified location.
         * @param pos The element will be inserted before this position.
         * @param args Arguments to the key-value to insert.
         * @return Iterator pointing to the created element.
         */
        template<class ...Args>
        iterator emplace (const_iterator pos, Args&&... args) {
            std::lock_guard<std::mutex> lock (mutex);
            if (pos.sorted) {
                auto ui = items.emplace (pos.si->second, std::forward<Args>(args)...);
                auto si = keys.emplace_hint (pos.si, std::cref(ui->first), ui);
                return iterator (si);
            }else{
                typename ItemList::iterator i;
                if (pos.i == items.cbegin()) {
                    items.emplace_front (std::forward<Args>(args)...);
                    i = items.begin ();
                    keys.emplace_hint (keys.begin(), std::cref(i->first), i);
                }else if (pos.i == items.cend()) {
                    items.emplace_back (std::forward<Args>(args)...);
                    i = --items.end();
                    keys.emplace_hint (keys.end(), std::cref(i->first), i);
                }else{
                    i = items.emplace (pos.i, std::forward<Args>(args)...);
                    keys.emplace_hint (get_key_pos_hint(i->first, pos),
                                       std::cref(i->first), i);
                }
                return iterator (i);
            }
        }


        /**
         * Create an element at the beginning of the list.
         * @param args Arguments to the key-value to insert.
         * @return A reference to the created element.
         */
        template<class ...Args>
        reference emplace_front (Args&&... args) {
            std::lock_guard<std::mutex> lock (mutex);
            auto& retval = items.emplace_front (std::forward<Args>(args)...);
            auto items_begin = items.begin ();
            keys.emplace_hint (keys.begin(), std::cref(items_begin->first), items_begin);
            return retval;
        }


        /**
         * Create an element at the end of the list.
         * @param args Arguments to the key-value to insert.
         * @return A reference to the created element.
         */
        template<class ...Args>
        reference emplace_back (Args&&... args) {
            std::lock_guard<std::mutex> lock (mutex);
            auto& retval = items.emplace_back (std::forward<Args>(args)...);
            auto items_last = --(items.end());
            keys.emplace_hint (keys.end(), std::cref(items_last->first), items_last);
            return retval;
        }


    private:
        //--------------------------------------------------------------------------
        // Assume mutex is locked
        //--------------------------------------------------------------------------
        inline iterator erase_impl (const_iterator pos) {
            if (pos.sorted) {
                // KeyRefMap
                items.erase (pos.si->second);
                return iterator (keys.erase(pos.si));
            }else{
                // ItemList
                auto range = keys.equal_range (pos.i->first);
                for (auto i=range.first; i!=range.second; ++i) {
                    if (i->second == pos.i) {
                        keys.erase (i);
                        break;
                    }
                }
                return iterator (items.erase(pos.i));
            }
        }
    public:


        /**
         * Erase an entry at the specified position.
         * @param pos The position of the element to erase.
         * @return Iterator following the removed element.
         */
        iterator erase (const_iterator pos) {
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
         * Erase a range of entries from the container.
         * @param first The position of the first entry to erase.
         * @param last The position after the last entry to erase.
         * @return Iterator following the last removed element.
         */
        iterator erase (const_iterator first, const_iterator last) {
            std::lock_guard<std::mutex> lock (mutex);
            if (first.sorted) {
                for (auto i=first; i!=last; ++i)
                    items.erase (i.si->second);
                return iterator (keys.erase(first.si, last.si));
            }else{
                for (auto pos=first; pos!=last; ++pos) {
                    auto range = keys.equal_range (pos.i->first);
                    for (auto i=range.first; i!=range.second; ++i) {
                        if (i->second == pos.i) {
                            keys.erase (i);
                            break;
                        }
                    }
                }
                return iterator (items.erase(first.i, last.i));
            }
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
            std::scoped_lock lock (mutex, other.mutex);
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
         * Find the first entry with a specific key.
         * @param key The key to search for.
         * @return Iterator to the first entry with the key,
         *         or end() if no entry with the specified key.
         */
        const_iterator find (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            auto si = keys.find (key);
            if (si == keys.end())
                return end ();
            else
                return const_iterator (si->second);
        }


        /**
         * Check if there is at least one entry with a specific key.
         * @return <code>true</code> if there is an entry with the key.
         */
        bool contains (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
#if (__cplusplus >= 202002L)
            return keys.contains (key);
#else
            return keys.find(key) != keys.end();
#endif
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
         * @return A sorted iterator to an element,
         *         or <code>send()</code> if none found.
         */
        iterator lower_bound (const key_type& key) {
            std::lock_guard<std::mutex> lock (mutex);
            return iterator (keys.lower_bound(key));
        }


        /**
         * Return the position of the first element with
         * a key <em>not less</em> than the pecified key.
         * @return A sorted const_iterator to an element,
         *         or <code>csend()</code> if none found.
         */
        const_iterator lower_bound (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            return const_iterator (keys.lower_bound(key));
        }


        /**
         * Return the position of the first element with
         * a key <em>greater</em> than the pecified key.
         * @return A sorted iterator to an element,
         *         or <code>send()</code> if none found.
         */
        iterator upper_bound (const key_type& key) {
            std::lock_guard<std::mutex> lock (mutex);
            return iterator (keys.upper_bound(key));
        }


        /**
         * Return the position of the first element with
         * a key <em>greater</em> than the pecified key.
         * @return A sorted const_iterator to an element,
         *         or <code>csend()</code> if none found.
         */
        const_iterator upper_bound (const key_type& key) const {
            std::lock_guard<std::mutex> lock (mutex);
            return const_iterator (keys.upper_bound(key));
        }


    private:
        inline bool equals_impl (const multimap_list& rhs) const {
            // if (items.size() != rhs.items.size())
            //     return false;
            auto lhs_i = csbegin ();
            auto rhs_i = rhs.csbegin ();
            auto lhs_i_end = csend ();
            auto rhs_i_end = rhs.csend ();
            value_compare less;
            while (lhs_i != lhs_i_end  &&  rhs_i != rhs_i_end) {
                if (less(*lhs_i, *rhs_i) || less(*rhs_i, *lhs_i))
                    return false;
                ++lhs_i;
                ++rhs_i;
            }
            return lhs_i==lhs_i_end && rhs_i==rhs_i_end;
        }


    public:
        /**
         * Check if two containers are equal using sorted keys.
         * @param lhs The left-hand-side of the comparison (lhs == rhs).
         * @param rhs The right-hand-side of the comparison (lhs == rhs).
         * @return <code>true</code> if the left-hand-side
         *         is equal to the right-hand-side,
         *         otherwise <code>false</code>.
         */
        friend bool operator== (const multimap_list& lhs, const multimap_list& rhs) {
            if (&lhs == &rhs)
                return true;
            std::scoped_lock lock (lhs.mutex, rhs.mutex);
            return lhs.equals_impl (rhs);
        }


        /**
         * Check if two containers are not equal using sorted keys.
         * @param lhs The left-hand-side of the comparison (lhs != rhs).
         * @param rhs The right-hand-side of the comparison (lhs != rhs).
         * @return <code>true</code> if the left-hand-side
         *         is not equal to the right-hand-side,
         *         otherwise <code>false</code>.
         */
        inline friend bool operator!= (const multimap_list& lhs, const multimap_list& rhs) {
            return ! (lhs == rhs);
        }


        /**
         * Compare the contents of two containers
         * lexicographically using sorted keys.
         * @param lhs The left-hand-side of the comparison (lhs < rhs).
         * @param rhs The right-hand-side of the comparison (lhs < rhs).
         * @return <code>true</code> if <code>lhs</code> is
         *         lexicographically less than <code>rhs</code>.
         */
        friend bool operator< (const multimap_list& lhs, const multimap_list& rhs) {
            if (&lhs == &rhs)
                return false;
            std::scoped_lock lock (lhs.mutex, rhs.mutex);
            return std::lexicographical_compare (lhs.sbegin(), lhs.send(),
                                                 rhs.sbegin(), rhs.send(),
                                                 value_compare{});
        }


        /**
         * Compare the contents of two containers
         * lexicographically using sorted keys.
         * @param lhs The left-hand-side of the comparison (lhs <= rhs).
         * @param rhs The right-hand-side of the comparison (lhs <= rhs).
         * @return <code>true</code> if <code>lhs</code> is
         *         lexicographically less than or equal to <code>rhs</code>.
         */
        friend bool operator<= (const multimap_list& lhs, const multimap_list& rhs) {
            if (&lhs == &rhs)
                return true;
            std::scoped_lock lock (lhs.mutex, rhs.mutex);
            return lhs.equals_impl(rhs) ||
                std::lexicographical_compare (lhs.sbegin(), lhs.send(),
                                              rhs.sbegin(), rhs.send(),
                                              value_compare{});
        }


        /**
         * Compare the contents of two containers
         * lexicographically using sorted keys.
         * @param lhs The left-hand-side of the comparison (lhs > rhs).
         * @param rhs The right-hand-side of the comparison (lhs > rhs).
         * @return <code>true</code> if <code>lhs</code> is
         *         lexicographically greater than <code>rhs</code>.
         */
        friend bool operator> (const multimap_list& lhs, const multimap_list& rhs) {
            if (&lhs == &rhs)
                return false;
            std::scoped_lock lock (lhs.mutex, rhs.mutex);
            return std::lexicographical_compare (rhs.sbegin(), rhs.send(),
                                                 lhs.sbegin(), lhs.send(),
                                                 value_compare{});
        }


        /**
         * Compare the contents of two containers
         * lexicographically using sorted keys.
         * @param lhs The left-hand-side of the comparison (lhs >= rhs).
         * @param rhs The right-hand-side of the comparison (lhs >= rhs).
         * @return <code>true</code> if <code>lhs</code> is
         *         lexicographically greater than or equal to
         *         <code>rhs</code>.
         */
        friend bool operator>= (const multimap_list& lhs, const multimap_list& rhs) {
            if (&lhs == &rhs)
                return true;
            std::scoped_lock lock (lhs.mutex, rhs.mutex);
            return lhs.equals_impl(rhs) ||
                std::lexicographical_compare (rhs.sbegin(), rhs.send(),
                                              lhs.sbegin(), lhs.send(),
                                              value_compare{});
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
