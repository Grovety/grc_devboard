/// @file
/// @author David Pilger <dpilger26@gmail.com>
/// [GitHub Repository](https://github.com/dpilger26/NumCpp)
///
/// License
/// Copyright 2018-2023 David Pilger
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this
/// software and associated documentation files(the "Software"), to deal in the Software
/// without restriction, including without limitation the rights to use, copy, modify,
/// merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
/// permit persons to whom the Software is furnished to do so, subject to the following
/// conditions :
///
/// The above copyright notice and this permission notice shall be included in all copies
/// or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
/// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
/// PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
/// FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
/// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
/// DEALINGS IN THE SOFTWARE.
///
/// Description
/// Functions for working with NdArrays
///
#pragma once

#include <cmath>
#include <string>

#include "NumCpp/Core/Internal/Error.hpp"
#include "NumCpp/Core/Internal/StaticAsserts.hpp"
#include "NumCpp/Core/Internal/StlAlgorithms.hpp"
#include "NumCpp/NdArray.hpp"
#include "NumCpp/Utils/sqr.hpp"

namespace nc
{
    //============================================================================
    // Method Description:
    /// Given the "legs" of a right triangle, return its hypotenuse.
    ///
    /// Equivalent to sqrt(x1**2 + x2**2), element - wise.
    ///
    /// NumPy Reference: https://www.numpy.org/devdocs/reference/generated/numpy.hypot.html
    ///
    ///
    /// @param inValue1
    /// @param inValue2
    ///
    /// @return value
    ///
    template<typename dtype>
    double hypot(dtype inValue1, dtype inValue2) noexcept
    {
        STATIC_ASSERT_ARITHMETIC(dtype);

        return std::hypot(static_cast<double>(inValue1), static_cast<double>(inValue2));
    }

    //============================================================================
    // Method Description:
    /// Given the "legs" of a right triangle, return its hypotenuse.
    ///
    /// Equivalent to sqrt(x1**2 + x2**2 + x3**2), element - wise.
    ///
    /// NumPy Reference: https://www.numpy.org/devdocs/reference/generated/numpy.hypot.html
    ///
    ///
    /// @param inValue1
    /// @param inValue2
    /// @param inValue3
    ///
    /// @return value
    ///
    template<typename dtype>
    double hypot(dtype inValue1, dtype inValue2, dtype inValue3) noexcept
    {
        STATIC_ASSERT_ARITHMETIC(dtype);

#ifdef __cpp_lib_hypot
        return std::hypot(static_cast<double>(inValue1), static_cast<double>(inValue2), static_cast<double>(inValue3));
#else
        return std::sqrt(utils::sqr(static_cast<double>(inValue1)) + utils::sqr(static_cast<double>(inValue2)) +
                         utils::sqr(static_cast<double>(inValue3)));
#endif
    }

    //============================================================================
    // Method Description:
    /// Given the "legs" of a right triangle, return its hypotenuse.
    ///
    /// Equivalent to sqrt(x1**2 + x2**2), element - wise.
    ///
    /// NumPy Reference: https://www.numpy.org/devdocs/reference/generated/numpy.hypot.html
    ///
    ///
    /// @param inArray1
    /// @param inArray2
    ///
    /// @return NdArray
    ///
    template<typename dtype>
    NdArray<double> hypot(const NdArray<dtype>& inArray1, const NdArray<dtype>& inArray2)
    {
        if (inArray1.shape() != inArray2.shape())
        {
            THROW_INVALID_ARGUMENT_ERROR("input array shapes are not consistant.");
        }

        NdArray<double> returnArray(inArray1.shape());

        stl_algorithms::transform(inArray1.cbegin(),
                                  inArray1.cend(),
                                  inArray2.cbegin(),
                                  returnArray.begin(),
                                  [](dtype inValue1, dtype inValue2) noexcept -> double
                                  { return hypot(inValue1, inValue2); });

        return returnArray;
    }
} // namespace nc
