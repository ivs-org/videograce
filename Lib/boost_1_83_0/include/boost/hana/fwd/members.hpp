/*!
@file
Forward declares `boost::hana::members`.

Copyright Louis Dionne 2013-2022
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_HANA_FWD_MEMBERS_HPP
#define BOOST_HANA_FWD_MEMBERS_HPP

#include <boost/hana/config.hpp>
#include <boost/hana/core/when.hpp>


namespace boost { namespace hana {
    //! Returns a `Sequence` containing the members of a `Struct`.
    //! @ingroup group-Struct
    //!
    //! Given a `Struct` object, `members` returns a `Sequence` containing
    //! all the members of the `Struct`, in the same order as their respective
    //! accessor appears in the `accessors` sequence.
    //!
    //!
    //! Example
    //! -------
    //! @include example/members.cpp
#ifdef BOOST_HANA_DOXYGEN_INVOKED
    constexpr auto members = [](auto&& object) {
        return tag-dispatched;
    };
#else
    template <typename S, typename = void>
    struct members_impl : members_impl<S, when<true>> { };

    struct members_t {
        template <typename Object>
        constexpr auto operator()(Object&& object) const;
    };

    BOOST_HANA_INLINE_VARIABLE constexpr members_t members{};
#endif
}} // end namespace boost::hana

#endif // !BOOST_HANA_FWD_MEMBERS_HPP
