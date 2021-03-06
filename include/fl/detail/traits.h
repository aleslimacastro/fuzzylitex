/**
 * \file fl/detail/traits.h
 *
 * \brief Type traits
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2014 Marco Guazzone (marco.guazzone@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FL_DETAIL_TRAITS_H
#define FL_DETAIL_TRAITS_H


#include <algorithm>
#include <cmath>
#include <limits>

#ifdef FL_CPP11
# include <type_traits>
//# define FL_DETAIL_TRAITS_NS std
#else // FL_CPP11
# include <boost/type_traits/is_floating_point.hpp>
# include <boost/utility/enable_if.hpp>
//# define FL_DETAIL_TRAITS_NS boost
#endif // FL_CPP11


namespace fl { namespace detail {

/**
 * Traits class for floating point types
 *
 * \tparam T The floating point type
 * \tparam Enable_ For SFINAE-based implementation
 *
 * See also:
 * - http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
 * - http://www.petebecker.com/js/js200012.html
 * - http://code.google.com/p/googletest/source/browse/trunk/include/gtest/internal/gtest-internal.h
 * - http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.16
 * - http://adtmag.com/articles/2000/03/16/comparing-floats-how-to-determine-if-floating-quantities-are-close-enough-once-a-tolerance-has-been.aspx
 * - http://www.boost.org/doc/libs/1_47_0/libs/test/doc/html/utf/testing-tools/floating_point_comparison.html
 * - http://learningcppisfun.blogspot.com/2010/04/comparing-floating-point-numbers.html
 * .
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename T, typename Enable_ = void>
struct FloatTraits;

/// Specialization for floating point types
template <typename T>
#ifdef FL_CPP11
struct FloatTraits<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
#else
struct FloatTraits<T, typename boost::enable_if< boost::is_floating_point<T> >::type>
#endif // FL_CPP11
{
    /// Default tolerance for floating-point comparison.
    static const T tolerance;


    /**
     * \brief x is approximately equal to y.
     *
     * Inspired by [1]:
     * \f[
     *  $x \approx y\,\text{ if and only if } |y-x|\le\epsilon\max(|x|,|y|)
     * \f]
     *
     * References:
     * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
     * .
     */
    static bool ApproximatelyEqual(T x, T y, T tol = tolerance)
    {
        if (std::isnan(x) || std::isnan(y))
        {
            // According to IEEE, NaN are different event by itself
            return false;
        }
        return std::abs(x-y) <= (std::max(std::abs(x), std::abs(y))*tol);
    }

    /**
     * \brief x is essentially equal to y.
     *
     * Inspired by [1]:
     * \f[
     *  $x \sim y\,\text{ if and only if } |y-x|\le\epsilon\min(|x|,|y|)
     * \f]
     *
     * References:
     * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
     * .
     */
    static bool EssentiallyEqual(T x, T y, T tol = tolerance)
    {
        if (std::isnan(x) || std::isnan(y))
        {
            // According to IEEE, NaN are different event by itself
            return false;
        }
        return std::abs(x-y) <= (std::min(std::abs(x), std::abs(y))*tol);
    }

    /**
     * \brief x is definitely less than y.
     *
     * Inspired by [1]:
     * \f[
     *  $x \prec y\,\text{ if and only if } y-x > \epsilon\max(|x|,|y|)
     * \f]
     *
     * References:
     * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
     * .
     */
    static bool DefinitelyLess(T x, T y, T tol = tolerance)
    {
        return (y-x) > (std::max(std::abs(x), std::abs(y))*tol);
    }

    /**
     * \brief x is definitely greater than y.
     *
     * Inspired by [1]:
     * \f[
     *  $x \succ y\,\text{ if and only if } x-y > \epsilon\max(|x|,|y|)
     * \f]
     *
     * References:
     * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
     * .
     */
    static bool DefinitelyGreater(T x, T y, T tol = tolerance)
    {
        return (x-y) > (std::max(std::abs(x), std::abs(y))*tol);
    }

    /// Returns true if \a x is approximately equal or definitely less than \a y with respect to the given tolerance \a tol
    static bool ApproximatelyLessEqual(T x, T y, T tol = tolerance)
    {
        return DefinitelyLess(x, y, tol) || ApproximatelyEqual(x, y, tol);
    }

    /// Returns true if \a x is essentially equal or definitely less than \a y with respect to the given tolerance \a tol
    static bool EssentiallyLessEqual(T x, T y, T tol = tolerance)
    {
        return DefinitelyLess(x, y, tol) || EssentiallyEqual(x, y, tol);
    }

    /// Returns true if \a x is approximately equal or definitely greater than \a y with respect to the given tolerance \a tol
    static bool ApproximatelyGreaterEqual(T x, T y, T tol = tolerance)
    {
        return DefinitelyGreater(x, y, tol) || ApproximatelyEqual(x, y, tol);
    }

    /// Returns true if \a x is essentially equal or definitely greater than \a y with respect to the given tolerance \a tol
    static bool EssentiallyGreaterEqual(T x, T y, T tol = tolerance)
    {
        return DefinitelyGreater(x, y, tol) || EssentiallyEqual(x, y, tol);
    }

    /// Returns true if \a x is approximately equal to zero with respect to the given tolerance \a tol
    static bool ApproximatelyZero(T x, T tol = tolerance)
    {
        return ApproximatelyEqual(x, T(0), tol);
    }

    /// Returns true if \a x is essentially equal to zero with respect to the given tolerance \a tol
    static bool EssentiallyZero(T x, T tol = tolerance)
    {
        return EssentiallyEqual(x, T(0), tol);
    }

    /// Returns \a x if \a x is definitely less than \a y with respect to the given tolerance \a tol; otherwise, returns \a y
    static T DefinitelyMin(T x, T y, T tol = tolerance)
    {
        if (DefinitelyLess(x, y, tol))
        {
            return x;
        }
        return y;
    }

    /// Returns \a x if \a x is less than \a y; otherwise, returns \a y
    static T Min(T x, T y)
    {
        return std::min(x, y);
    }

    /// Returns \a x if \a x is definitely greater than \a y with respect to the given tolerance \a tol; otherwise, returns \a y
    static T DefinitelyMax(T x, T y, T tol = tolerance)
    {
        if (DefinitelyGreater(x, y, tol))
        {
            return x;
        }
        return y;
    }

    /// Returns \a x if \a x is greater than \a y; otherwise, returns \a y
    static T Max(T x, T y)
    {
        return std::max(x, y);
    }

    /**
     * Returns either \a l if \a x is definitely less than \a l with respect to
     * the given tolerance \a tol, or \a h if \a x is definitely greater than
     * \a h with respect to the given tolerance \a tol, or \a x otherwise
     */
    static T DefinitelyClamp(T x, T l, T h, T tol = tolerance)
    {
        return DefinitelyMin(h, DefinitelyMax(l, x));
    }

    /**
     * Returns either \a l if \a x is less than \a l, or \a h if \a x is
     * greater than \a h, or \a x otherwise
     */
    static T Clamp(T x, T l, T h)
    {
        return Min(h, Max(l, x));
    }
}; // FloatTraits

template <typename T>
#ifdef FL_CPP11
const T FloatTraits<T, typename std::enable_if<std::is_floating_point<T>::value>::type>::tolerance = static_cast<T>(100)*std::numeric_limits<T>::epsilon();
#else
const T FloatTraits<T, typename boost::enable_if< boost::is_floating_point<T> >::type>::tolerance = static_cast<T>(100)*std::numeric_limits<T>::epsilon();
#endif // FL_CPP11

}} // Namespace fl::detail

#endif // FL_DETAIL_TRAITS_H
