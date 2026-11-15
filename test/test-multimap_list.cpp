/*
 * Copyright (C) 2026 Dan Arrhenius <dan@ultramarin.se>
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
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <ujson/multimap_list.hpp>
#include <algorithm>
#include <iostream>
#include <string>
#include <typeinfo>
#include <cstdio>
#include <cassert>

namespace uj = ujson;

using mml_t = uj::multimap_list<int, std::string>;
using std::cout;
using std::cerr;
using std::endl;


namespace { // Anonymous namespace


    std::string check_line;


#define check(arg)                              \
    if(!(arg)) {                                \
        check_line = std::to_string(__LINE__);  \
        check_line.append (": ");               \
        check_line.append (#arg);               \
        return false;                           \
    }

    bool print_check_line ()
    {
        cerr << check_line << endl;
        return false;
    }

#define chk_assert(arg) assert(arg || print_check_line())


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
#if 0
    void view_mml (const mml_t& mml)
    {
        if (mml.empty()) {
            std::cout << "(empty)" << std::endl;
            std::cout << std::endl;
            return;
        }

        auto i = mml.begin ();
        auto si = mml.sbegin ();
        std::cout << "unsorted\tsorted" << std::endl;
        for (unsigned n=0; n<mml.size(); ++n) {
            std::cout << i->first << ": " << i->second;
            std::cout << "\t\t";
            std::cout << si->first << ": " << si->second;
            std::cout << std::endl;
            ++i; ++si;
        }
        std::cout << std::endl;
    }
#endif


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool check_equal (mml_t& lhs, mml_t& rhs, bool check_natural_order=true)
    {
        check (lhs.empty() == rhs.empty());
        check (lhs.size() == rhs.size());

        // Check using operator==
        check (lhs == rhs);

        // Check using operator!=
        check ( ! (lhs != rhs));

        if (check_natural_order) {
            // Check that the natural order is the same using iterators
            auto lhs_i = lhs.begin ();
            auto rhs_i = rhs.begin ();
            while (lhs_i != lhs.end()) {
                check (*lhs_i == *rhs_i);
                check (lhs_i->first == rhs_i->first);
                check (lhs_i->second == rhs_i->second);
                ++lhs_i; ++rhs_i;
            }
            check (lhs_i == lhs.end());
            check (rhs_i == rhs.end());
            while (lhs_i != lhs.begin()) {
                --lhs_i; --rhs_i;
                check (*lhs_i == *rhs_i);
            }

            // Check that the natural order is the same using const iterators
            auto lhs_ci = lhs.cbegin ();
            auto rhs_ci = rhs.cbegin ();
            while (lhs_ci != lhs.cend()) {
                check (*lhs_ci == *rhs_ci);
                check (lhs_ci->first == rhs_ci->first);
                check (lhs_ci->second == rhs_ci->second);
                ++lhs_ci; ++rhs_ci;
            }
            check (lhs_ci == lhs.cend());
            check (rhs_ci == rhs.cend());
            while (lhs_ci != lhs.cbegin()) {
                --lhs_ci; --rhs_ci;
                check (*lhs_ci == *rhs_ci);
            }

            // Check that the natural order is the same using reverse iterators
            auto lhs_ri = lhs.rbegin ();
            auto rhs_ri = rhs.rbegin ();
            while (lhs_ri != lhs.rend()) {
                check (*lhs_ri == *rhs_ri);
                check (lhs_ri->first == rhs_ri->first);
                check (lhs_ri->second == rhs_ri->second);
                ++lhs_ri; ++rhs_ri;
            }
            check (lhs_ri == lhs.rend());
            check (rhs_ri == rhs.rend());
            while (lhs_ri != lhs.rbegin()) {
                --lhs_ri; --rhs_ri;
                check (*lhs_ri == *rhs_ri);
            }

            // Check that the natural order is the same using const reverse iterators
            auto lhs_cri = lhs.crbegin ();
            auto rhs_cri = rhs.crbegin ();
            while (lhs_cri != lhs.crend()) {
                check (*lhs_cri == *rhs_cri);
                check (lhs_cri->first == rhs_cri->first);
                check (lhs_cri->second == rhs_cri->second);
                ++lhs_cri; ++rhs_cri;
            }
            check (lhs_cri == lhs.crend());
            check (rhs_cri == rhs.crend());
            while (lhs_cri != lhs.crbegin()) {
                --lhs_cri; --rhs_cri;
                check (*lhs_cri == *rhs_cri);
            }
        }


        // Check that the sorted order is the same using iterators
        auto lhs_si = lhs.sbegin ();
        auto rhs_si = rhs.sbegin ();
        while (lhs_si != lhs.send()) {
            check (*lhs_si == *rhs_si);
            check (lhs_si->first == rhs_si->first);
            check (lhs_si->second == rhs_si->second);
            ++lhs_si; ++rhs_si;
        }
        check (lhs_si == lhs.send());
        check (rhs_si == rhs.send());
        while (lhs_si != lhs.sbegin()) {
            --lhs_si; --rhs_si;
            check (*lhs_si == *rhs_si);
        }

        // Check that the sorted order is the same using const iterators
        auto lhs_sci = lhs.csbegin ();
        auto rhs_sci = rhs.csbegin ();
        while (lhs_sci != lhs.csend()) {
            check (*lhs_sci == *rhs_sci);
            check (lhs_sci->first == rhs_sci->first);
            check (lhs_sci->second == rhs_sci->second);
            ++lhs_sci; ++rhs_sci;
        }
        check (lhs_sci == lhs.csend());
        check (rhs_sci == rhs.csend());
        while (lhs_sci != lhs.csbegin()) {
            --lhs_sci; --rhs_sci;
            check (*lhs_sci == *rhs_sci);
        }

        // Check that the sorted order is the same using reverse iterators
        auto lhs_sri = lhs.rsbegin ();
        auto rhs_sri = rhs.rsbegin ();
        while (lhs_sri != lhs.rsend()) {
            check (*lhs_sri == *rhs_sri);
            check (lhs_sri->first == rhs_sri->first);
            check (lhs_sri->second == rhs_sri->second);
            ++lhs_sri; ++rhs_sri;
        }
        check (lhs_sri == lhs.rsend());
        check (rhs_sri == rhs.rsend());
        while (lhs_sri != lhs.rsbegin()) {
            --lhs_sri; --rhs_sri;
            check (*lhs_sri == *rhs_sri);
        }

        // Check that the sorted order is the same using const reverse iterators
        auto lhs_scri = lhs.crsbegin ();
        auto rhs_scri = rhs.crsbegin ();
        while (lhs_scri != lhs.crsend()) {
            check (*lhs_scri == *rhs_scri);
            check (lhs_scri->first == rhs_scri->first);
            check (lhs_scri->second == rhs_scri->second);
            ++lhs_scri; ++rhs_scri;
        }
        check (lhs_scri == lhs.crsend());
        check (rhs_scri == rhs.crsend());
        while (lhs_scri != lhs.crsbegin()) {
            --lhs_scri; --rhs_scri;
            check (*lhs_scri == *rhs_scri);
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_constructors ()
    {
        // Reference object
        mml_t ref; // Default constructor
        chk_assert (ref.size() == 0);
        chk_assert (ref.empty() == true);
        chk_assert (ref.begin() == ref.end());
        ref.emplace_back (1, "bbb0");
        ref.emplace_back (0, "aaa0");
        ref.emplace_back (0, "aaa1");
        ref.emplace_back (2, "ccc0");
        ref.emplace_back (0, "aaa2");
        chk_assert (ref.size() == 5);
        chk_assert (ref.empty() == false);

        // Check that the content is correct
        auto i = ref.begin ();
        chk_assert (*i == mml_t::value_type(1, "bbb0"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa0"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa1"));
        ++i; chk_assert (*i == mml_t::value_type(2, "ccc0"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa2"));
        ++i; chk_assert (i == ref.end());

        // Check that the sorted content is correct
        i = ref.sbegin ();
        chk_assert (*i == mml_t::value_type(0, "aaa0"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa1"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa2"));
        ++i; chk_assert (*i == mml_t::value_type(1, "bbb0"));
        ++i; chk_assert (*i == mml_t::value_type(2, "ccc0"));
        ++i; chk_assert (i == ref.send());


        mml_t sorted_ref; // Default constructor
        sorted_ref.emplace_back (0, "aaa0");
        sorted_ref.emplace_back (0, "aaa1");
        sorted_ref.emplace_back (0, "aaa2");
        sorted_ref.emplace_back (1, "bbb0");
        sorted_ref.emplace_back (2, "ccc0");
        i = sorted_ref.begin ();
        chk_assert (*i == mml_t::value_type(0, "aaa0"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa1"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa2"));
        ++i; chk_assert (*i == mml_t::value_type(1, "bbb0"));
        ++i; chk_assert (*i == mml_t::value_type(2, "ccc0"));
        ++i; chk_assert (i == sorted_ref.end());
        i = sorted_ref.sbegin ();
        chk_assert (*i == mml_t::value_type(0, "aaa0"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa1"));
        ++i; chk_assert (*i == mml_t::value_type(0, "aaa2"));
        ++i; chk_assert (*i == mml_t::value_type(1, "bbb0"));
        ++i; chk_assert (*i == mml_t::value_type(2, "ccc0"));
        ++i; chk_assert (i == sorted_ref.send());


        // Copy Constructor
        //
        mml_t mml1 (ref);
        chk_assert (check_equal(mml1, ref));

        // Move Constructor
        //
        mml_t mml2 ((mml_t&&)mml1);
        chk_assert (check_equal(mml2, ref));
        chk_assert (mml1 != mml2);
        chk_assert (mml1.size() == 0);
        chk_assert (mml1.empty() == true);

        // Constructor with iterators
        //
        mml_t mml3 (ref.begin(), ref.end());
        chk_assert (check_equal(mml3, ref));

        // Constructor with sorted iterators
        //
        mml_t mml4 (ref.sbegin(), ref.send());
        chk_assert (check_equal(mml4, ref) == false); // Natural order isn't the same

        chk_assert (check_equal(mml4, ref, false));   // Only check that they are equal using sorted iterators
        chk_assert (check_equal(mml4, sorted_ref));   // Both should have sorted natural order

        // Constructor with initializer list
        //
        mml_t mml5 {{1, "bbb0"},
                    {0, "aaa0"},
                    {0, "aaa1"},
                    {2, "ccc0"},
                    {0, "aaa2"}};
        chk_assert (check_equal(mml5, ref));

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_assignment ()
    {
        // Reference object
        mml_t ref;
        ref.emplace_back (1, "bbb0");
        ref.emplace_back (0, "aaa0");
        ref.emplace_back (0, "aaa1");
        ref.emplace_back (2, "ccc0");
        ref.emplace_back (0, "aaa2");
        chk_assert (ref.size() == 5);


        // Assignment operator
        //
        mml_t mml1;
        mml1 = ref;
        chk_assert (mml1.size() == 5);
        chk_assert (check_equal(mml1, ref));

        // Move operator
        //
        mml_t mml2;
        mml2 = (mml_t&&) mml1;
        chk_assert (mml2.size() == 5);
        chk_assert (check_equal(mml2, ref));
        chk_assert (mml1.size() == 0);
        chk_assert (mml1.empty() == true);

        // Assignment with initializer list
        //
        mml_t mml3;
        mml3 = {
            {1, "bbb0"},
            {0, "aaa0"},
            {0, "aaa1"},
            {2, "ccc0"},
            {0, "aaa2"},
        };
        chk_assert (mml3.size() == 5);
        chk_assert (check_equal(mml3, ref));

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_iterators ()
    {
        // Reference objects
        mml_t ref;
        chk_assert (ref.begin() == ref.end());
        chk_assert (ref.cbegin() == ref.cend());
        chk_assert (ref.cbegin() == ref.end());
        chk_assert (ref.rbegin() == ref.rend());
        chk_assert (ref.crbegin() == ref.crend());
        chk_assert (ref.crbegin() == ref.rend());
        chk_assert (ref.sbegin() == ref.send());
        chk_assert (ref.csbegin() == ref.csend());
        chk_assert (ref.csbegin() == ref.send());
        chk_assert (ref.rsbegin() == ref.rsend());
        chk_assert (ref.crsbegin() == ref.crsend());
        chk_assert (ref.crsbegin() == ref.rsend());
        ref.emplace_back (1, "bbb0");
        ref.emplace_back (0, "aaa0");
        ref.emplace_back (0, "aaa1");
        ref.emplace_back (2, "ccc0");
        ref.emplace_back (0, "aaa2");
        chk_assert (ref.size() == 5);
        chk_assert (ref.begin() != ref.end());
        chk_assert (ref.cbegin() != ref.cend());
        chk_assert (ref.cbegin() != ref.end());
        chk_assert (ref.rbegin() != ref.rend());
        chk_assert (ref.crbegin() != ref.crend());
        chk_assert (ref.crbegin() != ref.rend());
        chk_assert (ref.sbegin() != ref.send());
        chk_assert (ref.csbegin() != ref.csend());
        chk_assert (ref.csbegin() != ref.send());
        chk_assert (ref.rsbegin() != ref.rsend());
        chk_assert (ref.crsbegin() != ref.crsend());
        chk_assert (ref.crsbegin() != ref.rsend());

        //
        // iterator and const_iterator
        //
        {
            auto i = ref.begin ();
            mml_t::const_iterator ci = i;
            chk_assert (ci == ref.cbegin());
            // operator++
            chk_assert (i == ref.begin());
            chk_assert (ci == ref.cbegin());
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            chk_assert (i->first == 1);       // Make sure operator->() works as well
            chk_assert (i->second == "bbb0");
            chk_assert (ci->first == 1);
            chk_assert (ci->second == "bbb0");
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            ++i; ++ci;
            chk_assert (i == ref.end());
            chk_assert (ci == ref.cend());
            // operator--
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            chk_assert (i == ref.begin());
            chk_assert (ci == ref.cbegin());
            // operator++(int)
            chk_assert (*i++ == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci++ == mml_t::value_type(1, "bbb0"));
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa0"));
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            chk_assert (*i++ == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci++ == mml_t::value_type(2, "ccc0"));
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa2"));
            chk_assert (i == ref.end());
            chk_assert (ci == ref.cend());
            // operator--(int)
            chk_assert (i-- == ref.end());
            chk_assert (ci-- == ref.cend());
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa2"));
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            chk_assert (*i-- == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci-- == mml_t::value_type(2, "ccc0"));
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa0"));
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            chk_assert (i == ref.begin());
            chk_assert (ci == ref.cbegin());
            // chk_assert (*i-- == mml_t::value_type(1, "bbb0"));
            // chk_assert (*ci-- == mml_t::value_type(1, "bbb0"));
            // chk_assert (i == ref.end());
            // chk_assert (ci == ref.cend());
        }

        //
        // reverse_iterator and const_reverse_iterator
        //
        {
            auto ri = ref.rbegin ();
            mml_t::const_reverse_iterator cri = ri;
            chk_assert (cri == ref.crbegin());
            // operator++
            chk_assert (*ri == mml_t::value_type(0, "aaa2"));
            chk_assert (*cri == mml_t::value_type(0, "aaa2"));
            chk_assert (ri->first == 0);       // Make sure operator->() works as well
            chk_assert (ri->second == "aaa2");
            chk_assert (cri->first == 0);
            chk_assert (cri->second == "aaa2");
            chk_assert (*++ri == mml_t::value_type(2, "ccc0"));
            chk_assert (*++cri == mml_t::value_type(2, "ccc0"));
            ++ri; ++cri;
            chk_assert (*ri == mml_t::value_type(0, "aaa1"));
            chk_assert (*cri == mml_t::value_type(0, "aaa1"));
            ++ri; ++cri;
            chk_assert (*ri == mml_t::value_type(0, "aaa0"));
            chk_assert (*cri == mml_t::value_type(0, "aaa0"));
            ++ri; ++cri;
            chk_assert (*ri == mml_t::value_type(1, "bbb0"));
            chk_assert (*cri == mml_t::value_type(1, "bbb0"));
            ++ri; ++cri;
            chk_assert (ri == ref.rend());
            chk_assert (cri == ref.crend());
            // operator--
            chk_assert (*--ri == mml_t::value_type(1, "bbb0"));
            chk_assert (*--cri == mml_t::value_type(1, "bbb0"));
            --ri; --cri;
            chk_assert (*ri == mml_t::value_type(0, "aaa0"));
            chk_assert (*cri == mml_t::value_type(0, "aaa0"));
            --ri; --cri;
            chk_assert (*ri == mml_t::value_type(0, "aaa1"));
            chk_assert (*cri == mml_t::value_type(0, "aaa1"));
            --ri; --cri;
            chk_assert (*ri == mml_t::value_type(2, "ccc0"));
            chk_assert (*cri == mml_t::value_type(2, "ccc0"));
            --ri; --cri;
            chk_assert (*ri == mml_t::value_type(0, "aaa2"));
            chk_assert (*cri == mml_t::value_type(0, "aaa2"));
            chk_assert (ri == ref.rbegin());
            chk_assert (cri == ref.crbegin());
            // operator++(int)
            chk_assert (*ri++ == mml_t::value_type(0, "aaa2"));
            chk_assert (*cri++ == mml_t::value_type(0, "aaa2"));
            chk_assert (*ri == mml_t::value_type(2, "ccc0"));
            chk_assert (*cri == mml_t::value_type(2, "ccc0"));
            chk_assert (*ri++ == mml_t::value_type(2, "ccc0"));
            chk_assert (*cri++ == mml_t::value_type(2, "ccc0"));
            chk_assert (*ri == mml_t::value_type(0, "aaa1"));
            chk_assert (*cri == mml_t::value_type(0, "aaa1"));
            chk_assert (*ri++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*cri++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*ri == mml_t::value_type(0, "aaa0"));
            chk_assert (*cri == mml_t::value_type(0, "aaa0"));
            chk_assert (*ri++ == mml_t::value_type(0, "aaa0"));
            chk_assert (*cri++ == mml_t::value_type(0, "aaa0"));
            chk_assert (*ri == mml_t::value_type(1, "bbb0"));
            chk_assert (*cri == mml_t::value_type(1, "bbb0"));
            chk_assert (*ri++ == mml_t::value_type(1, "bbb0"));
            chk_assert (*cri++ == mml_t::value_type(1, "bbb0"));
            chk_assert (ri == ref.rend());
            chk_assert (cri == ref.crend());
            // operator--(int)
            chk_assert (ri-- == ref.rend());
            chk_assert (cri-- == ref.crend());
            chk_assert (*ri == mml_t::value_type(1, "bbb0"));
            chk_assert (*cri == mml_t::value_type(1, "bbb0"));
            chk_assert (*ri-- == mml_t::value_type(1, "bbb0"));
            chk_assert (*cri-- == mml_t::value_type(1, "bbb0"));
            chk_assert (*ri == mml_t::value_type(0, "aaa0"));
            chk_assert (*cri == mml_t::value_type(0, "aaa0"));
            chk_assert (*ri-- == mml_t::value_type(0, "aaa0"));
            chk_assert (*cri-- == mml_t::value_type(0, "aaa0"));
            chk_assert (*ri == mml_t::value_type(0, "aaa1"));
            chk_assert (*cri == mml_t::value_type(0, "aaa1"));
            chk_assert (*ri-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*cri-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*ri == mml_t::value_type(2, "ccc0"));
            chk_assert (*cri == mml_t::value_type(2, "ccc0"));
            chk_assert (*ri-- == mml_t::value_type(2, "ccc0"));
            chk_assert (*cri-- == mml_t::value_type(2, "ccc0"));
            chk_assert (*ri == mml_t::value_type(0, "aaa2"));
            chk_assert (*cri == mml_t::value_type(0, "aaa2"));
            chk_assert (ri == ref.rbegin());
            chk_assert (cri == ref.crbegin());
            // chk_assert (*ri-- == mml_t::value_type(0, "aaa2"));
            // chk_assert (*cri-- == mml_t::value_type(0, "aaa2"));
            // chk_assert (ri == ref.rend());
            // chk_assert (cri == ref.crend());
        }

        //
        // sorted iterator and sorted const_iterator
        //
        {
            auto i = ref.sbegin ();
            mml_t::const_iterator ci = i;
            chk_assert (ci == ref.csbegin());
            // operator++
            chk_assert (i == ref.sbegin());
            chk_assert (ci == ref.csbegin());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            chk_assert (i->first == 0);       // Make sure operator->() works as well
            chk_assert (i->second == "aaa0");
            chk_assert (ci->first == 0);
            chk_assert (ci->second == "aaa0");
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            ++i; ++ci;
            chk_assert (i == ref.send());
            chk_assert (ci == ref.csend());
            // operator--
            --i; --ci;
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            chk_assert (i == ref.sbegin());
            chk_assert (ci == ref.csbegin());
            // operator++(int)
            chk_assert (*i++ == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa0"));
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa2"));
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            chk_assert (*i++ == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci++ == mml_t::value_type(1, "bbb0"));
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            chk_assert (*i++ == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci++ == mml_t::value_type(2, "ccc0"));
            chk_assert (i == ref.send());
            chk_assert (ci == ref.csend());
            // operator--(int)
            chk_assert (i-- == ref.send());
            chk_assert (ci-- == ref.csend());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            chk_assert (*i-- == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci-- == mml_t::value_type(2, "ccc0"));
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            chk_assert (*i-- == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci-- == mml_t::value_type(1, "bbb0"));
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa2"));
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            chk_assert (i == ref.sbegin());
            chk_assert (ci == ref.csbegin());
            // chk_assert (*i-- == mml_t::value_type(0, "aaa0"));
            // chk_assert (*ci-- == mml_t::value_type(0, "aaa0"));
            // chk_assert (i == ref.send());
            // chk_assert (ci == ref.csend());
        }

        //
        // sorted reverse_iterator and sorted const_reverse_iterator
        //
        {
            auto i = ref.rsbegin ();
            mml_t::const_reverse_iterator ci = i;
            chk_assert (ci == ref.crsbegin());
            // operator++
            chk_assert (i == ref.rsbegin());
            chk_assert (ci == ref.crsbegin());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            chk_assert (i->first == 2);       // Make sure operator->() works as well
            chk_assert (i->second == "ccc0");
            chk_assert (ci->first == 2);
            chk_assert (ci->second == "ccc0");
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            ++i; ++ci;
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            ++i; ++ci;
            chk_assert (i == ref.rsend());
            chk_assert (ci == ref.crsend());
            // operator--
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            --i; --ci;
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            chk_assert (i == ref.rsbegin());
            chk_assert (ci == ref.crsbegin());
            // operator++(int)
            chk_assert (*i++ == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci++ == mml_t::value_type(2, "ccc0"));
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            chk_assert (*i++ == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci++ == mml_t::value_type(1, "bbb0"));
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa2"));
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa1"));
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            chk_assert (*i++ == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci++ == mml_t::value_type(0, "aaa0"));
            chk_assert (i == ref.rsend());
            chk_assert (ci == ref.crsend());
            // operator--(int)
            chk_assert (i-- == ref.rsend());
            chk_assert (ci-- == ref.crsend());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci == mml_t::value_type(0, "aaa0"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa0"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa0"));
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci == mml_t::value_type(0, "aaa1"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa1"));
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci == mml_t::value_type(0, "aaa2"));
            chk_assert (*i-- == mml_t::value_type(0, "aaa2"));
            chk_assert (*ci-- == mml_t::value_type(0, "aaa2"));
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci == mml_t::value_type(1, "bbb0"));
            chk_assert (*i-- == mml_t::value_type(1, "bbb0"));
            chk_assert (*ci-- == mml_t::value_type(1, "bbb0"));
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (*ci == mml_t::value_type(2, "ccc0"));
            chk_assert (i == ref.rsbegin());
            chk_assert (ci == ref.crsbegin());
            // chk_assert (*i-- == mml_t::value_type(2, "ccc0"));
            // chk_assert (*ci-- == mml_t::value_type(2, "ccc0"));
            // chk_assert (i == ref.rsend());
            // chk_assert (ci == ref.crsend());
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_front_back ()
    {
        // Reference object
        mml_t ref;
        ref.emplace_back (1, "bbb0");
        ref.emplace_back (0, "aaa0");
        ref.emplace_back (0, "aaa1");
        ref.emplace_back (2, "ccc0");
        ref.emplace_back (0, "aaa2");
        chk_assert (ref.size() == 5);

        chk_assert (ref.front() == mml_t::value_type(1, "bbb0"));
        chk_assert (ref.back()  == mml_t::value_type(0, "aaa2"));
        {
            auto& val = ref.front ();
            chk_assert (val.first == 1);
            chk_assert (val.second == "bbb0");
            val.second = "BBB0";
            chk_assert (ref.front() == mml_t::value_type(1, "BBB0"));
        }
        {
            auto& val = ref.back ();
            chk_assert (val.first == 0);
            chk_assert (val.second == "aaa2");
            val.second = "AAA2";
            chk_assert (ref.back() == mml_t::value_type(0, "AAA2"));
        }

        ref.pop_front (); // Remove first element
        ref.pop_back ();  // Remove last element

        chk_assert (ref.front() == mml_t::value_type(0, "aaa0"));
        chk_assert (ref.back()  == mml_t::value_type(2, "ccc0"));
        {
            auto& val = ref.front ();
            chk_assert (val.first == 0);
            chk_assert (val.second == "aaa0");
            val.second = "AAA0";
            chk_assert (ref.front() == mml_t::value_type(0, "AAA0"));
        }
        {
            auto& val = ref.back ();
            chk_assert (val.first == 2);
            chk_assert (val.second == "ccc0");
            val.second = "CCC0";
            chk_assert (ref.back() == mml_t::value_type(2, "CCC0"));
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_push_pop_front_back ()
    {
        // Reference objects
        mml_t ref1;
        ref1.emplace_back (1, "bbb0");
        ref1.emplace_back (0, "aaa0");
        ref1.emplace_back (0, "aaa1");
        ref1.emplace_back (2, "ccc0");
        ref1.emplace_back (0, "aaa2");
        chk_assert (ref1.size() == 5);
        chk_assert (ref1.empty() == false);

        mml_t ref2;
        chk_assert (ref2.size() == 0);
        chk_assert (ref2.empty() == true);


        // key value
        // push_back
        // pop_back
        {
            mml_t mml;
            std::string v1 = "bbb0";
            std::string v2 = "aaa0";
            std::string v3 = "aaa1";
            std::string v4 = "ccc0";
            std::string v5 = "aaa2";
            mml.push_back (1, v1);
            chk_assert (v1 == "bbb0");
            mml.push_back (0, (std::string&&)v2);
            chk_assert (v2.empty());
            mml.push_back (0, v3);
            chk_assert (v3 == "aaa1");
            mml.push_back (2, (std::string&&)v4);
            chk_assert (v4.empty());
            mml.push_back (0, v5);
            chk_assert (v5 == "aaa2");
            chk_assert (check_equal(ref1, mml));
            mml.pop_back ();
            chk_assert (mml.back() == mml_t::value_type(2, "ccc0"));
            mml.pop_back ();
            chk_assert (mml.back() == mml_t::value_type(0, "aaa1"));
            mml.pop_back ();
            chk_assert (mml.back() == mml_t::value_type(0, "aaa0"));
            mml.pop_back ();
            chk_assert (mml.back() == mml_t::value_type(1, "bbb0"));
            mml.pop_back ();
            chk_assert (mml.size() == 0);
            chk_assert (mml.empty() == true);
            chk_assert (check_equal(ref2, mml));
        }

        // key value
        // push_front
        // pop_front
        {
            mml_t mml;
            std::string v1 = "aaa2";
            std::string v2 = "ccc0";
            std::string v3 = "aaa1";
            std::string v4 = "aaa0";
            std::string v5 = "bbb0";
            mml.push_front (0, v1);
            chk_assert (v1 == "aaa2");
            mml.push_front (2, (std::string&&)v2);
            chk_assert (v2.empty());
            mml.push_front (0, v3);
            chk_assert (v3 == "aaa1");
            mml.push_front (0, (std::string&&)v4);
            chk_assert (v4.empty());
            mml.push_front (1, v5);
            chk_assert (v5 == "bbb0");
            chk_assert (check_equal(ref1, mml));
            mml.pop_front ();
            chk_assert (mml.front() == mml_t::value_type(0, "aaa0"));
            mml.pop_front ();
            chk_assert (mml.front() == mml_t::value_type(0, "aaa1"));
            mml.pop_front ();
            chk_assert (mml.front() == mml_t::value_type(2, "ccc0"));
            mml.pop_front ();
            chk_assert (mml.front() == mml_t::value_type(0, "aaa2"));
            chk_assert (mml.front() == mml.back());
            mml.pop_front ();
            chk_assert (mml.empty());
            chk_assert (mml.size() == 0);
            chk_assert (check_equal(ref2, mml));
        }

        // key value
        // push_front push_back
        // pop_back pop_front
        {
            mml_t mml;
            mml.push_front (0, "aaa1");
            mml.push_back (2, "ccc0");
            mml.push_front (0, "aaa0");
            mml.push_back (0, "aaa2");
            mml.push_front (1, "bbb0");
            chk_assert (check_equal(ref1, mml));
            mml.pop_back ();
            chk_assert (mml.front() == mml_t::value_type(1, "bbb0"));
            mml.pop_front ();
            chk_assert (mml.back() == mml_t::value_type(2, "ccc0"));
            mml.pop_back ();
            chk_assert (mml.front() == mml_t::value_type(0, "aaa0"));
            mml.pop_front ();
            chk_assert (mml.front() == mml_t::value_type(0, "aaa1"));
            chk_assert (mml.back() == mml_t::value_type(0, "aaa1"));
            chk_assert (mml.front() == mml.back());
            mml.pop_back ();
            chk_assert (mml.empty());
            chk_assert (check_equal(ref2, mml));
        }

        // value_type, &&value_type
        // push_back
        // pop_back
        {
            mml_t mml;
            mml_t::value_type v1 = std::make_pair (1, "bbb0");
            mml.push_back (v1);
            chk_assert (mml.back() == v1);
            chk_assert (mml.front() == v1);
            mml_t::value_type v2 = std::make_pair (0, "aaa0");
            mml.push_back (v2);
            chk_assert (mml.back() == v2);
            mml_t::value_type v3 = std::make_pair (0, "aaa1");
            mml.push_back (v3);
            chk_assert (mml.back() == v3);
            mml_t::value_type v4 = std::make_pair (2, "ccc0");
            mml.push_back (v4);
            chk_assert (mml.back() == v4);
            mml_t::value_type v5 = std::make_pair (0, "aaa2");
            mml.push_back ((mml_t::value_type&&)v5);
            chk_assert (mml.back() != v5);
            chk_assert (v5.second.empty());
            chk_assert (check_equal(ref1, mml));
        }

        // value_type, &&value_type
        // push_front
        // pop_front
        {
            mml_t mml;
            mml_t::value_type v1 = std::make_pair (0, "aaa2");
            mml.push_front (v1);
            chk_assert (mml.back() == v1);
            chk_assert (mml.front() == v1);
            mml_t::value_type v2 = std::make_pair (2, "ccc0");
            mml.push_front (v2);
            chk_assert (mml.front() == v2);
            mml_t::value_type v3 = std::make_pair (0, "aaa1");
            mml.push_front (v3);
            chk_assert (mml.front() == v3);
            mml_t::value_type v4 = std::make_pair (0, "aaa0");
            mml.push_front (v4);
            chk_assert (mml.front() == v4);
            mml_t::value_type v5 = std::make_pair (1, "bbb0");
            mml.push_front ((mml_t::value_type&&)v5);
            chk_assert (mml.front() != v5);
            chk_assert (v5.second.empty());
            chk_assert (check_equal(ref1, mml));
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_insert ()
    {
        // Reference objects
        mml_t ref;
        ref.emplace_back (1, "bbb0");
        ref.emplace_back (0, "aaa0");
        ref.emplace_back (0, "aaa1");
        ref.emplace_back (2, "ccc0");
        ref.emplace_back (0, "aaa2");
        ref.emplace_back (3, "ddd0");
        chk_assert (ref.size() == 6);
        chk_assert (ref.empty() == false);

        //
        // insert pos(iterator), key, value
        //
        {
            mml_t mml;
            mml_t::iterator i;
            std::string v1 = "ddd0";
            std::string v2 = "aaa2";
            std::string v3 = "ccc0";
            std::string v4 = "aaa1";
            std::string v5 = "aaa0";
            std::string v6 = "bbb0";
            i = mml.insert (mml.begin(), 3, v1);
            chk_assert (i->second == v1);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 0, v2);
            chk_assert (i->second == v2);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 2, v3);
            chk_assert (i->second == v3);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 0, v4);
            chk_assert (i->second == v4);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 0, v5);
            chk_assert (i->second == v5);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 1, v6);
            chk_assert (i->second == v6);
            chk_assert (i == mml.begin());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            std::string v1 = "bbb0";
            std::string v2 = "aaa0";
            std::string v3 = "aaa1";
            std::string v4 = "ccc0";
            std::string v5 = "aaa2";
            std::string v6 = "ddd0";
            i = mml.insert (mml.end(), 1, v1);
            chk_assert (i->second == v1);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 0, v2);
            chk_assert (i->second == v2);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 0, v3);
            chk_assert (i->second == v3);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 2, v4);
            chk_assert (i->second == v4);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 0, v5);
            chk_assert (i->second == v5);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 3, v6);
            chk_assert (i->second == v6);
            chk_assert (i == --mml.end());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t ref_1 ({{2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            mml.emplace_back (2, "a");
            mml.emplace_back (4, "b");
            mml.emplace_back (4, "c");
            mml.emplace_back (6, "d");
            chk_assert (check_equal(mml, ref_1));

            std::string v1 = "a";
            i = mml.insert (mml.begin(), 1, v1);
            mml_t ref_2 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            chk_assert (i->second == v1);
            chk_assert (check_equal(mml, ref_2));

            std::string v2 = "a";
            i = mml.insert (mml.end(), 7, v2);
            mml_t ref_3 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (i->second == v2);
            chk_assert (check_equal(mml, ref_3));

            i = mml.begin ();
            ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            std::string v3 = "x";
            i = mml.insert (i, 2, v3);
            mml_t ref_4 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (i->second == v3);
            chk_assert (check_equal(mml, ref_4));

            i = mml.begin ();
            ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            std::string v4 = "y";
            i = mml.insert (i, 6, v4);
            mml_t ref_5 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "y"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (i->second == v4);
            chk_assert (check_equal(mml, ref_5));

            i = mml.begin ();
            ++i; ++i; ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "c"));
            std::string v5 = "z";
            i = mml.insert (i, 4, v5);
            mml_t ref_6 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "y"},
                          {4, "b"},
                          {4, "z"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (i->second == v5);
            chk_assert (check_equal(mml, ref_6));
        }


        //
        // insert pos(iterator), key, &&value
        //
        {
            mml_t mml;
            mml_t::iterator i;
            std::string v1 = "ddd0";
            std::string v2 = "aaa2";
            std::string v3 = "ccc0";
            std::string v4 = "aaa1";
            std::string v5 = "aaa0";
            std::string v6 = "bbb0";
            i = mml.insert (mml.begin(), 3, (std::string&&)v1);
            chk_assert (i->second == "ddd0");
            chk_assert (v1.empty());
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 0, (std::string&&)v2);
            chk_assert (i->second == "aaa2");
            chk_assert (v2.empty());
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 2, (std::string&&)v3);
            chk_assert (i->second == "ccc0");
            chk_assert (v3.empty());
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 0, (std::string&&)v4);
            chk_assert (i->second == "aaa1");
            chk_assert (v4.empty());
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 0, (std::string&&)v5);
            chk_assert (i->second == "aaa0");
            chk_assert (v5.empty());
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), 1, (std::string&&)v6);
            chk_assert (i->second == "bbb0");
            chk_assert (v6.empty());
            chk_assert (i == mml.begin());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            std::string v1 = "bbb0";
            std::string v2 = "aaa0";
            std::string v3 = "aaa1";
            std::string v4 = "ccc0";
            std::string v5 = "aaa2";
            std::string v6 = "ddd0";
            i = mml.insert (mml.end(), 1, (std::string&&)v1);
            chk_assert (i->second == "bbb0");
            chk_assert (v1.empty());
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 0, (std::string&&)v2);
            chk_assert (i->second == "aaa0");
            chk_assert (v2.empty());
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 0, (std::string&&)v3);
            chk_assert (i->second == "aaa1");
            chk_assert (v3.empty());
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 2, (std::string&&)v4);
            chk_assert (i->second == "ccc0");
            chk_assert (v4.empty());
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 0, (std::string&&)v5);
            chk_assert (i->second == "aaa2");
            chk_assert (v5.empty());
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), 3, (std::string&&)v6);
            chk_assert (i->second == "ddd0");
            chk_assert (v6.empty());
            chk_assert (i == --mml.end());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t ref_1 ({{2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            mml.emplace_back (2, "a");
            mml.emplace_back (4, "b");
            mml.emplace_back (4, "c");
            mml.emplace_back (6, "d");
            chk_assert (check_equal(mml, ref_1));

            mml.insert (mml.begin(), 1, "a");
            mml_t ref_2 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            chk_assert (check_equal(mml, ref_2));

            mml.insert (mml.end(), 7, "a");
            mml_t ref_3 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_3));

            i = mml.begin ();
            ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml.insert (i, 2, "x");
            mml_t ref_4 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_4));

            i = mml.begin ();
            ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml.insert (i, 6, "x");
            mml_t ref_5 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_5));

            i = mml.begin ();
            ++i; ++i; ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "c"));
            mml.insert (i, 4, "x");
            mml_t ref_6 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "x"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_6));
        }


        //
        // insert pos(iterator), value_type
        //
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t::value_type v1 (3, "ddd0");
            mml_t::value_type v2 (0, "aaa2");
            mml_t::value_type v3 (2, "ccc0");
            mml_t::value_type v4 (0, "aaa1");
            mml_t::value_type v5 (0, "aaa0");
            mml_t::value_type v6 (1, "bbb0");
            i = mml.insert (mml.begin(), v1);
            chk_assert (*i == v1);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), v2);
            chk_assert (*i == v2);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), v3);
            chk_assert (*i == v3);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), v4);
            chk_assert (*i == v4);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), v5);
            chk_assert (*i == v5);
            chk_assert (i == mml.begin());
            i = mml.insert (mml.begin(), v6);
            chk_assert (*i == v6);
            chk_assert (i == mml.begin());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t::value_type v1 (1, "bbb0");
            mml_t::value_type v2 (0, "aaa0");
            mml_t::value_type v3 (0, "aaa1");
            mml_t::value_type v4 (2, "ccc0");
            mml_t::value_type v5 (0, "aaa2");
            mml_t::value_type v6 (3, "ddd0");
            i = mml.insert (mml.end(), v1);
            chk_assert (*i == v1);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), v2);
            chk_assert (*i == v2);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), v3);
            chk_assert (*i == v3);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), v4);
            chk_assert (*i == v4);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), v5);
            chk_assert (*i == v5);
            chk_assert (i == --mml.end());
            i = mml.insert (mml.end(), v6);
            chk_assert (*i == v6);
            chk_assert (i == --mml.end());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t ref_1 ({{2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            mml.emplace_back (2, "a");
            mml.emplace_back (4, "b");
            mml.emplace_back (4, "c");
            mml.emplace_back (6, "d");
            chk_assert (check_equal(mml, ref_1));

            mml_t::value_type v1 (1, "a");
            i = mml.insert (mml.begin(), v1);
            mml_t ref_2 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            chk_assert (*i == v1);
            chk_assert (check_equal(mml, ref_2));

            mml_t::value_type v2 (7, "a");
            i = mml.insert (mml.end(), v2);
            mml_t ref_3 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (*i == v2);
            chk_assert (check_equal(mml, ref_3));

            i = mml.begin ();
            ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml_t::value_type v3 (2, "x");
            i = mml.insert (i, v3);
            mml_t ref_4 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (*i == v3);
            chk_assert (check_equal(mml, ref_4));

            i = mml.begin ();
            ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml_t::value_type v4 (6, "x");
            i = mml.insert (i, v4);
            mml_t ref_5 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (*i == v4);
            chk_assert (check_equal(mml, ref_5));

            i = mml.begin ();
            ++i; ++i; ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "c"));
            mml_t::value_type v5 (4, "x");
            i = mml.insert (i, v5);
            mml_t ref_6 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "x"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (*i == v5);
            chk_assert (check_equal(mml, ref_6));
        }


        //
        // insert pos(iterator), &&value_type
        //
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t::value_type v1 (3, "ddd0");
            mml_t::value_type v2 (0, "aaa2");
            mml_t::value_type v3 (2, "ccc0");
            mml_t::value_type v4 (0, "aaa1");
            mml_t::value_type v5 (0, "aaa0");
            mml_t::value_type v6 (1, "bbb0");
            i = mml.insert (mml.begin(), (mml_t::value_type&&)v1);
            chk_assert (i == mml.begin());
            chk_assert (*i == mml_t::value_type(3, "ddd0"));
            chk_assert (v1.second.empty());
            i = mml.insert (mml.begin(), (mml_t::value_type&&)v2);
            chk_assert (i == mml.begin());
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (v2.second.empty());
            i = mml.insert (mml.begin(), (mml_t::value_type&&)v3);
            chk_assert (i == mml.begin());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (v3.second.empty());
            i = mml.insert (mml.begin(), (mml_t::value_type&&)v4);
            chk_assert (i == mml.begin());
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (v4.second.empty());
            i = mml.insert (mml.begin(), (mml_t::value_type&&)v5);
            chk_assert (i == mml.begin());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (v5.second.empty());
            i = mml.insert (mml.begin(), (mml_t::value_type&&)v6);
            chk_assert (i == mml.begin());
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (v6.second.empty());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t::value_type v1 (1, "bbb0");
            mml_t::value_type v2 (0, "aaa0");
            mml_t::value_type v3 (0, "aaa1");
            mml_t::value_type v4 (2, "ccc0");
            mml_t::value_type v5 (0, "aaa2");
            mml_t::value_type v6 (3, "ddd0");
            i = mml.insert (mml.end(), (mml_t::value_type&&)v1);
            chk_assert (i == --mml.end());
            chk_assert (*i == mml_t::value_type(1, "bbb0"));
            chk_assert (v1.second.empty());
            i = mml.insert (mml.end(), (mml_t::value_type&&)v2);
            chk_assert (i == --mml.end());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            chk_assert (v2.second.empty());
            i = mml.insert (mml.end(), (mml_t::value_type&&)v3);
            chk_assert (i == --mml.end());
            chk_assert (*i == mml_t::value_type(0, "aaa1"));
            chk_assert (v3.second.empty());
            i = mml.insert (mml.end(), (mml_t::value_type&&)v4);
            chk_assert (i == --mml.end());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            chk_assert (v4.second.empty());
            i = mml.insert (mml.end(), (mml_t::value_type&&)v5);
            chk_assert (i == --mml.end());
            chk_assert (*i == mml_t::value_type(0, "aaa2"));
            chk_assert (v5.second.empty());
            i = mml.insert (mml.end(), (mml_t::value_type&&)v6);
            chk_assert (i == --mml.end());
            chk_assert (*i == mml_t::value_type(3, "ddd0"));
            chk_assert (v6.second.empty());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t ref_1 ({{2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            mml.emplace_back (2, "a");
            mml.emplace_back (4, "b");
            mml.emplace_back (4, "c");
            mml.emplace_back (6, "d");
            chk_assert (check_equal(mml, ref_1));

            mml_t::value_type v1 (1, "a");
            i = mml.insert (mml.begin(), (mml_t::value_type&&)v1);
            mml_t ref_2 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            chk_assert (*i == mml_t::value_type(1, "a"));
            chk_assert (v1.second.empty());
            chk_assert (check_equal(mml, ref_2));

            mml_t::value_type v2 (7, "a");
            i = mml.insert (mml.end(), (mml_t::value_type&&)v2);
            mml_t ref_3 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_3));
            chk_assert (*i == mml_t::value_type(7, "a"));
            chk_assert (v2.second.empty());

            i = mml.begin ();
            ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml_t::value_type v3 (2, "x");
            i = mml.insert (i, (mml_t::value_type&&)v3);
            mml_t ref_4 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_4));
            chk_assert (*i == mml_t::value_type(2, "x"));
            chk_assert (v3.second.empty());

            i = mml.begin ();
            ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml_t::value_type v4 (6, "x");
            i = mml.insert (i, (mml_t::value_type&&)v4);
            mml_t ref_5 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_5));
            chk_assert (*i == mml_t::value_type(6, "x"));
            chk_assert (v4.second.empty());

            i = mml.begin ();
            ++i; ++i; ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "c"));
            mml_t::value_type v5 (4, "x");
            i = mml.insert (i, (mml_t::value_type&&)v5);
            mml_t ref_6 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "x"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_6));
            chk_assert (*i == mml_t::value_type(4, "x"));
            chk_assert (v5.second.empty());
        }


        // insert pos, first, last
        {
            mml_t mml;
            mml.insert (mml.begin(), ref.begin(), ref.end());
            chk_assert (check_equal(mml, ref));
        }
        {
            mml_t ref_2;
            ref_2.emplace_back (1, "bbb1");
            ref_2.emplace_back (4, "eee0");
            ref_2.emplace_back (0, "aaa3");
            mml_t ref_3;
            ref_3.emplace_back (1, "bbb0");
            ref_3.emplace_back (0, "aaa0");
            ref_3.emplace_back (0, "aaa1");
            ref_3.emplace_back (2, "ccc0");
            ref_3.emplace_back (0, "aaa2");
            ref_3.emplace_back (3, "ddd0");
            ref_3.emplace_back (1, "bbb1");
            ref_3.emplace_back (4, "eee0");
            ref_3.emplace_back (0, "aaa3");
            mml_t mml;
            auto i1 = mml.insert (mml.begin(), ref.begin(), ref.end());
            chk_assert (*i1 == *ref.begin());
            chk_assert (check_equal(mml, ref));
            auto i2 = mml.insert (mml.end(), ref_2.begin(), ref_2.end());
            auto mml_i = mml.end ();
            --mml_i; --mml_i; --mml_i;
            chk_assert (i2 == mml_i);
            chk_assert (*i2 == *ref_2.begin());
            chk_assert (check_equal(mml, ref_3));
        }
        {
            mml_t ref_2;
            ref_2.emplace_back (1, "bbb1");
            ref_2.emplace_back (4, "eee0");
            ref_2.emplace_back (0, "aaa3");
            mml_t ref_3;
            ref_3.emplace_back (1, "bbb1"); // inserted
            ref_3.emplace_back (4, "eee0"); // inserted
            ref_3.emplace_back (0, "aaa3"); // inserted
            ref_3.emplace_back (1, "bbb0");
            ref_3.emplace_back (0, "aaa0");
            ref_3.emplace_back (0, "aaa1");
            ref_3.emplace_back (2, "ccc0");
            ref_3.emplace_back (0, "aaa2");
            ref_3.emplace_back (3, "ddd0");
            mml_t mml;
            auto i1 = mml.insert (mml.begin(), ref.begin(), ref.end());
            chk_assert (i1 == mml.begin());
            chk_assert (*i1 == *ref.begin());
            chk_assert (check_equal(mml, ref));

            auto i2 = mml.insert (mml.begin(), ref_2.begin(), ref_2.end());
            chk_assert (i2 == mml.begin());
            chk_assert (*i2 == *ref_3.begin());
            chk_assert (check_equal(mml, ref_3));
        }
        {
            mml_t ref_2;
            ref_2.emplace_back (1, "bbb1");
            ref_2.emplace_back (4, "eee0"); // insert
            ref_2.emplace_back (0, "aaa3"); // insert
            ref_2.emplace_back (5, "fff0"); // insert
            ref_2.emplace_back (6, "ggg0");
            mml_t ref_3;
            ref_3.emplace_back (1, "bbb0");
            ref_3.emplace_back (0, "aaa0");
            ref_3.emplace_back (0, "aaa1");
            ref_3.emplace_back (2, "ccc0");
            ref_3.emplace_back (0, "aaa2");
            ref_3.emplace_back (3, "ddd0");
            ref_3.emplace_back (4, "eee0"); // inserted
            ref_3.emplace_back (0, "aaa3"); // inserted
            ref_3.emplace_back (5, "fff0"); // inserted
            mml_t mml;
            mml.insert (mml.begin(), ref.begin(), ref.end());
            chk_assert (check_equal(mml, ref));

            auto b = ref_2.begin ();
            auto e = ref_2.end ();
            ++b; --e;
            auto i = mml.insert (mml.end(), b, e);
            auto mml_i = mml.end ();
            --mml_i; --mml_i; --mml_i;
            chk_assert (i == mml_i);
            chk_assert (*i == *mml_i);
            chk_assert (check_equal(mml, ref_3));
        }
        {
            mml_t ref_2;
            ref_2.emplace_back (1, "bbb1");
            ref_2.emplace_back (4, "eee0"); // insert
            ref_2.emplace_back (0, "aaa3"); // insert
            ref_2.emplace_back (5, "fff0"); // insert
            ref_2.emplace_back (6, "ggg0");
            mml_t ref_3;
            ref_3.emplace_back (1, "bbb0");
            ref_3.emplace_back (0, "aaa0");
            ref_3.emplace_back (4, "eee0"); // inserted
            ref_3.emplace_back (0, "aaa3"); // inserted
            ref_3.emplace_back (5, "fff0"); // inserted
            ref_3.emplace_back (0, "aaa1");
            ref_3.emplace_back (2, "ccc0");
            ref_3.emplace_back (0, "aaa2");
            ref_3.emplace_back (3, "ddd0");

            mml_t mml;
            mml.insert (mml.begin(), ref.begin(), ref.end());
            chk_assert (check_equal(mml, ref));

            auto pos = mml.begin ();
            ++pos; ++pos;
            auto b = ref_2.begin ();
            auto e = ref_2.end ();
            ++b; --e;
            auto i = mml.insert (pos, b, e);
            auto mml_i = mml.begin ();
            ++mml_i; ++mml_i;
            chk_assert (i == mml_i);
            chk_assert (*i == *mml_i);
            chk_assert (check_equal(mml, ref_3));
        }

        // insert pos, initializer_list
        {
            mml_t mml;
            mml.insert (mml.begin(), ref.begin(), ref.end());
            chk_assert (check_equal(mml, ref));

            auto pos = mml.begin ();
            ++pos; ++pos;
            auto i = mml.insert (pos, {
                    {4, "eee0"},
                    {0, "aaa3"},
                    {5, "fff0"}
                });
            auto mml_i = mml.begin ();
            ++mml_i; ++mml_i;
            chk_assert (i == mml_i);
            chk_assert (*i == *mml_i);
            mml_t ref_3;
            ref_3.emplace_back (1, "bbb0");
            ref_3.emplace_back (0, "aaa0");
            ref_3.emplace_back (4, "eee0"); // inserted
            ref_3.emplace_back (0, "aaa3"); // inserted
            ref_3.emplace_back (5, "fff0"); // inserted
            ref_3.emplace_back (0, "aaa1");
            ref_3.emplace_back (2, "ccc0");
            ref_3.emplace_back (0, "aaa2");
            ref_3.emplace_back (3, "ddd0");
            chk_assert (check_equal(mml, ref_3));
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_emplace ()
    {
        // Reference objects
        mml_t ref;
        ref.emplace_back (1, "bbb0");
        ref.emplace_back (0, "aaa0");
        ref.emplace_back (0, "aaa1");
        ref.emplace_back (2, "ccc0");
        ref.emplace_back (0, "aaa2");
        ref.emplace_back (3, "ddd0");
        chk_assert (ref.size() == 6);
        chk_assert (ref.empty() == false);

        //
        // emplace pos(iterator), key, &&value
        //
        {
            mml_t mml;
            mml_t::iterator i;
            std::string v1 = "ddd0";
            std::string v2 = "aaa2";
            std::string v3 = "ccc0";
            std::string v4 = "aaa1";
            std::string v5 = "aaa0";
            std::string v6 = "bbb0";
            i = mml.emplace (mml.begin(), 3, (std::string&&)v1);
            chk_assert (i->second == "ddd0");
            chk_assert (v1.empty());
            chk_assert (i == mml.begin());
            i = mml.emplace (mml.begin(), 0, (std::string&&)v2);
            chk_assert (i->second == "aaa2");
            chk_assert (v2.empty());
            chk_assert (i == mml.begin());
            i = mml.emplace (mml.begin(), 2, (std::string&&)v3);
            chk_assert (i->second == "ccc0");
            chk_assert (v3.empty());
            chk_assert (i == mml.begin());
            i = mml.emplace (mml.begin(), 0, (std::string&&)v4);
            chk_assert (i->second == "aaa1");
            chk_assert (v4.empty());
            chk_assert (i == mml.begin());
            i = mml.emplace (mml.begin(), 0, (std::string&&)v5);
            chk_assert (i->second == "aaa0");
            chk_assert (v5.empty());
            chk_assert (i == mml.begin());
            i = mml.emplace (mml.begin(), 1, (std::string&&)v6);
            chk_assert (i->second == "bbb0");
            chk_assert (v6.empty());
            chk_assert (i == mml.begin());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            std::string v1 = "bbb0";
            std::string v2 = "aaa0";
            std::string v3 = "aaa1";
            std::string v4 = "ccc0";
            std::string v5 = "aaa2";
            std::string v6 = "ddd0";
            i = mml.emplace (mml.end(), 1, (std::string&&)v1);
            chk_assert (i->second == "bbb0");
            chk_assert (v1.empty());
            chk_assert (i == --mml.end());
            i = mml.emplace (mml.end(), 0, (std::string&&)v2);
            chk_assert (i->second == "aaa0");
            chk_assert (v2.empty());
            chk_assert (i == --mml.end());
            i = mml.emplace (mml.end(), 0, (std::string&&)v3);
            chk_assert (i->second == "aaa1");
            chk_assert (v3.empty());
            chk_assert (i == --mml.end());
            i = mml.emplace (mml.end(), 2, (std::string&&)v4);
            chk_assert (i->second == "ccc0");
            chk_assert (v4.empty());
            chk_assert (i == --mml.end());
            i = mml.emplace (mml.end(), 0, (std::string&&)v5);
            chk_assert (i->second == "aaa2");
            chk_assert (v5.empty());
            chk_assert (i == --mml.end());
            i = mml.emplace (mml.end(), 3, (std::string&&)v6);
            chk_assert (i->second == "ddd0");
            chk_assert (v6.empty());
            chk_assert (i == --mml.end());
            chk_assert (check_equal(ref, mml));
        }
        {
            mml_t mml;
            mml_t::iterator i;
            mml_t ref_1 ({{2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            mml.emplace_back (2, "a");
            mml.emplace_back (4, "b");
            mml.emplace_back (4, "c");
            mml.emplace_back (6, "d");
            chk_assert (check_equal(mml, ref_1));

            mml.emplace (mml.begin(), 1, "a");
            mml_t ref_2 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"}});
            chk_assert (check_equal(mml, ref_2));

            mml.emplace (mml.end(), 7, "a");
            mml_t ref_3 ({{1, "a"},
                          {2, "a"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_3));

            i = mml.begin ();
            ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml.emplace (i, 2, "x");
            mml_t ref_4 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_4));

            i = mml.begin ();
            ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "b"));
            mml.emplace (i, 6, "x");
            mml_t ref_5 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_5));

            i = mml.begin ();
            ++i; ++i; ++i; ++i; ++i;
            chk_assert (*i == mml_t::value_type(4, "c"));
            mml.emplace (i, 4, "x");
            mml_t ref_6 ({{1, "a"},
                          {2, "a"},
                          {2, "x"},
                          {6, "x"},
                          {4, "b"},
                          {4, "x"},
                          {4, "c"},
                          {6, "d"},
                          {7, "a"}});
            chk_assert (check_equal(mml, ref_6));
        }


        {
            mml_t mml;
            std::string v1 = "bbb0";
            std::string v2 = "aaa0";
            std::string v3 = "aaa1";
            std::string v4 = "ccc0";
            std::string v5 = "aaa2";
            std::string v6 = "ddd0";
            auto& r1 = mml.emplace_back (1, (std::string&&)v1);
            chk_assert (r1.second == "bbb0");
            chk_assert (v1.empty());
            auto& r2 = mml.emplace_back (0, (std::string&&)v2);
            chk_assert (r2.second == "aaa0");
            chk_assert (v2.empty());
            auto& r3 = mml.emplace_back (0, (std::string&&)v3);
            chk_assert (r3.second == "aaa1");
            chk_assert (v3.empty());
            auto& r4 = mml.emplace_back (2, (std::string&&)v4);
            chk_assert (r4.second == "ccc0");
            chk_assert (v4.empty());
            auto& r5 = mml.emplace_back (0, (std::string&&)v5);
            chk_assert (r5.second == "aaa2");
            chk_assert (v5.empty());
            auto& r6 = mml.emplace_back (3, (std::string&&)v6);
            chk_assert (r6.second == "ddd0");
            chk_assert (v6.empty());
            chk_assert (check_equal(ref, mml));
        }


        {
            mml_t mml;
            std::string v1 = "ddd0";
            std::string v2 = "aaa2";
            std::string v3 = "ccc0";
            std::string v4 = "aaa1";
            std::string v5 = "aaa0";
            std::string v6 = "bbb0";
            auto& r1 = mml.emplace_front (3, (std::string&&)v1);
            chk_assert (r1.second == "ddd0");
            chk_assert (v1.empty());
            auto& r2 = mml.emplace_front (0, (std::string&&)v2);
            chk_assert (r2.second == "aaa2");
            chk_assert (v2.empty());
            auto& r3 = mml.emplace_front (2, (std::string&&)v3);
            chk_assert (r3.second == "ccc0");
            chk_assert (v3.empty());
            auto& r4 = mml.emplace_front (0, (std::string&&)v4);
            chk_assert (r4.second == "aaa1");
            chk_assert (v4.empty());
            auto& r5 = mml.emplace_front (0, (std::string&&)v5);
            chk_assert (r5.second == "aaa0");
            chk_assert (v5.empty());
            auto& r6 = mml.emplace_front (1, (std::string&&)v6);
            chk_assert (r6.second == "bbb0");
            chk_assert (v6.empty());
            chk_assert (check_equal(ref, mml));
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_erase ()
    {
        // Reference objects
        mml_t ref {{
                {1, "bbb0"},
                {0, "aaa0"},
                {0, "aaa1"},
                {2, "ccc0"},
                {0, "aaa2"},
                {3, "ddd0"},
            }};
        chk_assert (ref.size() == 6);
        chk_assert (ref.empty() == false);

        // erase begin
        {
            mml_t ref_1 {{
                    {0, "aaa2"},
                    {3, "ddd0"},
                }};
            auto mml = ref;
            auto i = mml.erase (mml.begin());
            chk_assert (i == mml.begin());
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa0");
            i = mml.erase (mml.begin());
            chk_assert (i == mml.begin());
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa1");
            i = mml.erase (mml.begin());
            chk_assert (i == mml.begin());
            chk_assert (i->first == 2);
            chk_assert (i->second == "ccc0");
            i = mml.erase (mml.begin());
            chk_assert (i == mml.begin());
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa2");
            chk_assert (check_equal(mml, ref_1));
        }
        // erase sbegin
        {
            mml_t ref_1 {{
                    {1, "bbb0"},
                    {2, "ccc0"},
                    {0, "aaa2"},
                    {3, "ddd0"},
                }};
            auto mml = ref;
            auto i = mml.erase (mml.sbegin());
            chk_assert (i == mml.sbegin());
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa1");
            i = mml.erase (mml.sbegin());
            chk_assert (i == mml.sbegin());
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa2");
            chk_assert (check_equal(mml, ref_1));
        }
        // erase pos
        {
            mml_t ref_1 {{
                    {1, "bbb0"},
                    // {0, "aaa0"}, - erased
                    {0, "aaa1"},
                    {2, "ccc0"},
                    {0, "aaa2"},
                    {3, "ddd0"},
                }};
            auto mml = ref;
            auto pos = mml.begin ();
            ++pos;
            auto i = mml.erase (pos);
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa1");
            chk_assert (check_equal(mml, ref_1));
        }
        // erase spos
        {
            mml_t ref_1 {{
                    {1, "bbb0"},
                    {0, "aaa0"},
                    // {0, "aaa1"},  - erased
                    {2, "ccc0"},
                    {0, "aaa2"},
                    {3, "ddd0"},
                }};
            auto mml = ref;
            auto spos = mml.sbegin ();
            ++spos;
            auto i = mml.erase (spos);
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa2");
            chk_assert (check_equal(mml, ref_1));
        }
        // erase pos_b pos_e
        {
            mml_t ref_1 {{
                    {1, "bbb0"},
                    // {0, "aaa0"}, - erased
                    // {0, "aaa1"}, - erased
                    // {2, "ccc0"}, - erased
                    {0, "aaa2"},
                    {3, "ddd0"},
                }};
            auto mml = ref;
            auto pos_b = mml.begin ();
            ++pos_b;
            auto pos_e = pos_b;
            ++pos_e; ++pos_e; ++pos_e;
            auto i = mml.erase (pos_b, pos_e);
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa2");
            chk_assert (check_equal(mml, ref_1));
        }
        // erase spos_b spos_e
        {
            mml_t ref_1 {{
                    // {1, "bbb0"}, - erased
                    {0, "aaa0"},
                    // {0, "aaa1"}, - erased
                    {2, "ccc0"},
                    // {0, "aaa2"}, - erased
                    {3, "ddd0"},
                }};
            auto mml = ref;
            auto pos_b = mml.sbegin ();
            ++pos_b;
            auto pos_e = pos_b;
            ++pos_e; ++pos_e; ++pos_e;
            auto i = mml.erase (pos_b, pos_e);
            chk_assert (i->first == 2);
            chk_assert (i->second == "ccc0");
            chk_assert (check_equal(mml, ref_1));
        }
        // erase key
        {
            mml_t ref_1 {{
                    {1, "bbb0"},
                    // {0, "aaa0"}, - erased
                    // {0, "aaa1"}, - erased
                    // {2, "ccc0"}, - erased
                    // {0, "aaa2"}, - erased
                    {3, "ddd0"},
                }};
            auto mml = ref;
            auto n = mml.erase (2);
            chk_assert (n == 1);
            n = mml.erase (0);
            chk_assert (n == 3);
            n = mml.erase (42);
            chk_assert (n == 0);
            chk_assert (check_equal(mml, ref_1));
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_search ()
    {
        // Reference objects
        mml_t ref {{
                {1, "bbb0"},
                {0, "aaa0"},
                {0, "aaa1"},
                {2, "ccc0"},
                {0, "aaa2"},
                {3, "ddd0"},
            }};
        const mml_t cref {{
                {1, "bbb0"},
                {0, "aaa0"},
                {0, "aaa1"},
                {2, "ccc0"},
                {0, "aaa2"},
                {3, "ddd0"},
            }};

        // find
        {
            mml_t::iterator i = ref.find (0);
            chk_assert (typeid(i) == typeid(mml_t::iterator));
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa0");
            i = ref.find (1);
            chk_assert (i->first == 1);
            chk_assert (i->second == "bbb0");
            i = ref.find (2);
            chk_assert (i->first == 2);
            chk_assert (i->second == "ccc0");
            i = ref.find (3);
            chk_assert (i->first == 3);
            chk_assert (i->second == "ddd0");
            i = ref.find (4);
            chk_assert (i == ref.end());
        }
        // find const iterator
        {
            auto i = cref.find (0);
            chk_assert (typeid(i) == typeid(mml_t::const_iterator));
            chk_assert (i->first == 0);
            chk_assert (i->second == "aaa0");
            i = cref.find (1);
            chk_assert (i->first == 1);
            chk_assert (i->second == "bbb0");
            i = cref.find (2);
            chk_assert (i->first == 2);
            chk_assert (i->second == "ccc0");
            i = cref.find (3);
            chk_assert (i->first == 3);
            chk_assert (i->second == "ddd0");
            i = cref.find (4);
            chk_assert (i == cref.cend());
        }
        // contains
        {
            chk_assert (ref.contains(-1) == false);
            chk_assert (ref.contains(0) == true);
            chk_assert (ref.contains(1) == true);
            chk_assert (ref.contains(2) == true);
            chk_assert (ref.contains(3) == true);
            chk_assert (ref.contains(4) == false);
        }
        // count
        {
            chk_assert (ref.count(-1) == 0);
            chk_assert (ref.count(0) == 3);
            chk_assert (ref.count(1) == 1);
            chk_assert (ref.count(2) == 1);
            chk_assert (ref.count(3) == 1);
            chk_assert (ref.count(4) == 0);
        }
        // equal range
        {
            auto r = ref.equal_range (0);
            chk_assert (typeid(r) == typeid(std::pair<mml_t::iterator, mml_t::iterator>));
            chk_assert (r.first != r.second);
            auto i = r.first;
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            ++i; chk_assert (*i == mml_t::value_type(0, "aaa1"));
            ++i; chk_assert (*i == mml_t::value_type(0, "aaa2"));
            ++i; chk_assert (i == r.second);

            r = ref.equal_range (2);
            i = r.first;
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            ++i; chk_assert (i == r.second);

            r = ref.equal_range (4);
            chk_assert (r.first == r.second);
            chk_assert (r.first == ref.send());
        }
        // const equal range
        {
            auto r = cref.equal_range (0);
            chk_assert (typeid(r) == typeid(std::pair<mml_t::const_iterator, mml_t::const_iterator>));
            chk_assert (r.first != r.second);
            auto i = r.first;
            chk_assert (*i == mml_t::value_type(0, "aaa0"));
            ++i; chk_assert (*i == mml_t::value_type(0, "aaa1"));
            ++i; chk_assert (*i == mml_t::value_type(0, "aaa2"));
            ++i; chk_assert (i == r.second);

            r = cref.equal_range (2);
            i = r.first;
            chk_assert (*i == mml_t::value_type(2, "ccc0"));
            ++i; chk_assert (i == r.second);

            r = cref.equal_range (4);
            chk_assert (r.first == r.second);
            chk_assert (r.first == cref.csend());
        }
        // lower_bound
        {
            auto i = ref.lower_bound (-1);
            chk_assert (typeid(i) == typeid(mml_t::iterator));
            chk_assert (i != ref.send());
            chk_assert (i == ref.sbegin());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));

            i = ref.lower_bound (0);
            chk_assert (i != ref.send());
            chk_assert (i == ref.sbegin());
            i = ref.lower_bound (1);
            chk_assert (i != ref.send());
            chk_assert (*i == mml_t::value_type(1, "bbb0"));

            i = ref.lower_bound (2);
            chk_assert (i != ref.send());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));

            i = ref.lower_bound (3);
            chk_assert (i != ref.send());
            chk_assert (*i == mml_t::value_type(3, "ddd0"));

            i = ref.lower_bound (4);
            chk_assert (i == ref.send());
        }
        // const lower_bound
        {
            auto i = cref.lower_bound (-1);
            chk_assert (typeid(i) == typeid(mml_t::const_iterator));
            chk_assert (i != cref.csend());
            chk_assert (i == cref.csbegin());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));

            i = cref.lower_bound (0);
            chk_assert (i != cref.csend());
            chk_assert (i == cref.csbegin());

            i = cref.lower_bound (1);
            chk_assert (i != cref.csend());
            chk_assert (*i == mml_t::value_type(1, "bbb0"));

            i = cref.lower_bound (2);
            chk_assert (i != cref.csend());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));

            i = cref.lower_bound (3);
            chk_assert (i != cref.csend());
            chk_assert (*i == mml_t::value_type(3, "ddd0"));

            i = cref.lower_bound (4);
            chk_assert (i == cref.csend());
        }
        // upper_bound
        {
            auto i = ref.upper_bound (-1);
            chk_assert (typeid(i) == typeid(mml_t::iterator));
            chk_assert (i != ref.send());
            chk_assert (i == ref.sbegin());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));

            i = ref.upper_bound (0);
            chk_assert (i != ref.send());
            chk_assert (*i == mml_t::value_type(1, "bbb0"));

            i = ref.upper_bound (1);
            chk_assert (i != ref.send());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));

            i = ref.upper_bound (2);
            chk_assert (i != ref.send());
            chk_assert (*i == mml_t::value_type(3, "ddd0"));

            i = ref.upper_bound (3);
            chk_assert (i == ref.send());

            i = ref.upper_bound (4);
            chk_assert (i == ref.send());
        }
        // const upper_bound
        {
            auto i = cref.upper_bound (-1);
            chk_assert (typeid(i) == typeid(mml_t::const_iterator));
            chk_assert (i != cref.send());
            chk_assert (i == cref.sbegin());
            chk_assert (*i == mml_t::value_type(0, "aaa0"));

            i = cref.upper_bound (0);
            chk_assert (i != cref.csend());
            chk_assert (*i == mml_t::value_type(1, "bbb0"));

            i = cref.upper_bound (1);
            chk_assert (i != cref.csend());
            chk_assert (*i == mml_t::value_type(2, "ccc0"));

            i = cref.upper_bound (2);
            chk_assert (i != cref.csend());
            chk_assert (*i == mml_t::value_type(3, "ddd0"));

            i = cref.upper_bound (3);
            chk_assert (i == cref.csend());

            i = cref.upper_bound (4);
            chk_assert (i == cref.csend());
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void test_misc ()
    {
        // Reference object
        mml_t ref {{
                {1, "bbb0"},
                {0, "aaa0"},
                {0, "aaa1"},
                {2, "ccc0"},
                {0, "aaa2"},
                {3, "ddd0"},
            }};

        // clear
        {
            mml_t mml = ref;
            chk_assert (mml == ref);
            chk_assert (mml.empty() == false);
            chk_assert (mml.size() == 6);
            chk_assert (mml.begin() != mml.end());
            chk_assert (mml.cbegin() != mml.cend());
            chk_assert (mml.rbegin() != mml.rend());
            chk_assert (mml.crbegin() != mml.crend());
            chk_assert (mml.sbegin() != mml.send());
            chk_assert (mml.csbegin() != mml.csend());
            chk_assert (mml.rsbegin() != mml.rsend());
            chk_assert (mml.crsbegin() != mml.crsend());
            mml.clear ();
            chk_assert (mml != ref);
            chk_assert (mml.empty() == true);
            chk_assert (mml.size() == 0);
            chk_assert (mml.begin() == mml.end());
            chk_assert (mml.cbegin() == mml.cend());
            chk_assert (mml.rbegin() == mml.rend());
            chk_assert (mml.crbegin() == mml.crend());
            chk_assert (mml.sbegin() == mml.send());
            chk_assert (mml.csbegin() == mml.csend());
            chk_assert (mml.rsbegin() == mml.rsend());
            chk_assert (mml.crsbegin() == mml.crsend());
        }

        // swap
        {
            mml_t ref_a {{
                    {1, "aaa0"},
                    {2, "bbb0"},
                    {3, "ccc0"},
                }};
            mml_t ref_b {{
                    {4, "ddd0"},
                    {5, "eee0"},
                    {6, "fff0"},
                }};
            mml_t ref_c {{
                    {4, "ddd0"},
                    {5, "xxx0"},
                    {6, "fff0"},
                }};
            mml_t a = ref_a;
            mml_t b = ref_b;
            chk_assert (check_equal(a, ref_a));
            chk_assert (check_equal(b, ref_b));

            a.swap (b);
            chk_assert (check_equal(a, ref_b));
            chk_assert (check_equal(b, ref_a));
            auto i = a.begin ();
            ++i; i->second = "xxx0";
            chk_assert (check_equal(a, ref_c));
        }

        std::cout << __FUNCTION__ << " - ok" << std::endl;
    }

} // Anonymous namespace


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    test_constructors ();
    test_assignment ();
    test_iterators ();
    test_front_back ();
    test_push_pop_front_back ();
    test_insert ();
    test_emplace ();
    test_erase ();
    test_search ();
    test_misc ();

    std::cout << "Ok" << std::endl;

    return 0;
}
