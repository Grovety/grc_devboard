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
/// Clusters exceedance pixels from an image
///

#pragma once

#include <vector>

#include "NumCpp/Core/Internal/StaticAsserts.hpp"
#include "NumCpp/Core/Types.hpp"
#include "NumCpp/ImageProcessing/Cluster.hpp"
#include "NumCpp/ImageProcessing/ClusterMaker.hpp"
#include "NumCpp/NdArray.hpp"

namespace nc
{
    namespace imageProcessing
    {
        //============================================================================
        // Method Description:
        /// Clusters exceedance pixels from an image
        ///
        /// @param inImageArray
        /// @param inExceedances
        /// @param inBorderWidth: border to apply around exceedance pixels post clustering (default 0)
        /// @return std::vector<Cluster>
        ///
        template<typename dtype>
        std::vector<Cluster<dtype>> clusterPixels(const NdArray<dtype>& inImageArray,
                                                  const NdArray<bool>&  inExceedances,
                                                  uint8                 inBorderWidth = 0)
        {
            STATIC_ASSERT_ARITHMETIC(dtype);

            ClusterMaker<dtype> clusterMaker(&inExceedances, &inImageArray, inBorderWidth);
            return std::vector<Cluster<dtype>>(clusterMaker.begin(), clusterMaker.end());
        }
    } // namespace imageProcessing
} // namespace nc
