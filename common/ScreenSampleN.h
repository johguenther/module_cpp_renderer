// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "RayN.h"

namespace ospray {
  namespace cpp_renderer {

    struct ScreenSampleRef;

    struct ScreenSampleN
    {
      // input values to 'renderSample'
      simd::vec3i sampleID; /*!< x/y=pixelID,z=accumID/sampleID */
      RayN        ray;      /*!< the primary ray generated by the camera */

      // return values from 'renderSample'
      simd::vec3f  rgb {simd::vfloat{0.f}};
      simd::vfloat alpha{0.f};
      simd::vfloat z{inf};
      simd::vint   tileOffset{-1};// linear value --> comes from tileX,tileY
    };

    struct ScreenSampleNRef
    {
      simd::vec3i  &sampleID;
      RayN         &ray;
      simd::vec3f  &rgb;
      simd::vfloat &alpha;
      simd::vfloat &z;
      simd::vint   &tileOffset;
    };

    template <int SIZE>
    struct ScreenSampleNStreamN
    {
      static constexpr int size = SIZE;

      std::array<simd::vec3i, SIZE> sampleID;

      StreamN<RayN, SIZE> rays;

      std::array<simd::vec3f,  SIZE> rgb;
      std::array<simd::vfloat, SIZE> alpha;
      std::array<simd::vfloat, SIZE> z;
      std::array<simd::vint,   SIZE> tileOffset;

      // Member functions //

      ScreenSampleNRef get(int i);
    };

    using ScreenSampleNStream = ScreenSampleNStreamN<STREAM_SIZE>;

    // Inlined function definitions ///////////////////////////////////////////

    template <int SIZE>
    inline ScreenSampleNRef ScreenSampleNStreamN<SIZE>::get(int i)
    {
      return {sampleID[i], rays[i], rgb[i], alpha[i], z[i], tileOffset[i]};
    }

    // Inlined helper functions ///////////////////////////////////////////////

    template <int SIZE, typename FCN_T>
    inline void
    for_each_sample(ScreenSampleNStreamN<SIZE> &stream, const FCN_T &fcn)
    {
      // TODO: Add static_assert() check for signature of FCN_T, similar to
      //       the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i)
        fcn(stream.get(i));
    }

    template <int SIZE, typename FCN_T, typename PRED_T>
    inline void
    for_each_sample(ScreenSampleNStreamN<SIZE> &stream,
                    const FCN_T &fcn,
                    const PRED_T &pred)
    {
      // TODO: Add static_assert() check for signature of FCN_T and PRED_T,
      //       similar to the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i) {
        auto sample = stream.get(i);
        fcn(pred(sample), sample);
      }
    }

    template <int SIZE, typename FCN_T>
    inline void
    for_each_sample_i(ScreenSampleNStreamN<SIZE> &stream, const FCN_T &fcn)
    {
      // TODO: Add static_assert() check for signature of FCN_T, similar to
      //       the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i)
        fcn(stream.get(i), i);
    }

    template <int SIZE, typename FCN_T, typename PRED_T>
    inline void
    for_each_sample_i(ScreenSampleNStreamN<SIZE> &stream,
                      const FCN_T &fcn,
                      const PRED_T &pred)
    {
      // TODO: Add static_assert() check for signature of FCN_T and PRED_T,
      //       similar to the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i) {
        auto sample = stream.get(i);
        fcn(pred(sample), sample, i);
      }
    }

    // Predefined predicates //////////////////////////////////////////////////

    inline simd::vmaski sampleEnabled(const ScreenSampleNRef &sample)
    {
      return sample.tileOffset >= 0;
    }

    inline simd::vmaski rayHit(const ScreenSampleNRef &sample)
    {
      return sample.ray.hitSomething();
    }

    inline simd::vmaski rayMiss(const ScreenSampleNRef &sample)
    {
      return !sample.ray.hitSomething();
    }

  }// namespace cpp_renderer
}// namespace ospray
