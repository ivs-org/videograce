/*!
@file
Defines `boost::hana::Searchable`.

Copyright Louis Dionne 2013-2022
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_HANA_CONCEPT_SEARCHABLE_HPP
#define BOOST_HANA_CONCEPT_SEARCHABLE_HPP

#include <boost/hana/fwd/concept/searchable.hpp>

#include <boost/hana/any_of.hpp>
#include <boost/hana/config.hpp>
#include <boost/hana/core/default.hpp>
#include <boost/hana/core/tag_of.hpp>
#include <boost/hana/detail/integral_constant.hpp>
#include <boost/hana/find_if.hpp>


namespace boost { namespace hana {
    template <typename S>
    struct Searchable
        : hana::integral_constant<bool,
            !is_default<any_of_impl<typename tag_of<S>::type>>::value &&
            !is_default<find_if_impl<typename tag_of<S>::type>>::value
        >
    { };
}} // end namespace boost::hana

#endif // !BOOST_HANA_CONCEPT_SEARCHABLE_HPP
