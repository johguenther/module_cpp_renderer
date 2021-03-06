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

#include "../embc/simd/simd.h"

namespace ospcommon {

  // Pack types //

  using vfloat = embree::vfloatx;
  using vint   = embree::vintx;
  using vuint  = embree::vuint<embree::VSIZEX>;

  inline vfloat rcp(const vfloat &val)
  {
    return 1.f / val;
  }

} // ::ospcommon

#ifdef _WIN32
#  define OSPRAY_ALIGN(...) __declspec(align(__VA_ARGS__))
#else
#  define OSPRAY_ALIGN(...) __attribute__((aligned(__VA_ARGS__)))
#endif

#include "ospcommon/vec.h"
// std
#include <random>

namespace ospray {
  namespace simd {

    using namespace embree;

    // Constants //////////////////////////////////////////////////////////////

    constexpr auto width = embree::VSIZEX;
    //const auto programIndex =

    // Aliases for vector types based on boost::simd types ////////////////////

    // Pack types //
    using vfloat = embree::vfloatx;
    using vint   = embree::vintx;
    using vuint  = embree::vuint<embree::VSIZEX>;

    template <typename T>
    using vptr = std::array<T*, width>;

    // Mask types //

    using vmask  = embree::vboolx;
    using vmaskf = embree::vboolfx;
    using vmaski = embree::vboolx;
    using vmasku = embree::vboolx;

    // Vector math types //

    template <typename T, int N>
    using vec_t = ospcommon::vec_t<T, N>;

    using vec2f = vec_t<vfloat, 2>;
    using vec2i = vec_t<vint,   2>;
    using vec2u = vec_t<vuint,  2>;

    using vec3f = vec_t<vfloat, 3>;
    using vec3i = vec_t<vint,   3>;
    using vec3u = vec_t<vuint,  3>;

    using vec4f = vec_t<vfloat, 4>;
    using vec4i = vec_t<vint,   4>;
    using vec4u = vec_t<vuint,  4>;

    // Cast operatons /////////////////////////////////////////////////////////

    template <typename NewType, typename OriginalType>
    inline NewType cast(const OriginalType &t)
    {
#if 1
      return NewType(t);
#else
      NewType nt;

      for (int i = 0; i < OriginalType::size; ++i)
        nt[i] = t[i];

      return nt;
#endif
    }

    // Algorithms /////////////////////////////////////////////////////////////

    // foreach //

    template <typename SIMD_T, typename FCN_T>
    inline void foreach_v(SIMD_T &v, FCN_T &&fcn)
    {
      // NOTE(jda) - need to static_assert() FCN_T's signature

      for (int i = 0; i < SIMD_T::size; ++i) {
        typename SIMD_T::value_type tmp = v[i];
        fcn(tmp, i);
        v[i] = tmp;
      }
    }

    // foreach_active //

    template <typename MASK_T, typename FCN_T>
    inline void foreach_active(const MASK_T &l, FCN_T &&fcn)
    {
      // NOTE(jda) - need to static_assert() FCN_T's signature
      for (int i = 0; i < MASK_T::size; ++i)
        if (l[i]) fcn(i);
    }

    template <typename MASK_T, typename SIMD_T, typename FCN_T>
    inline void foreach_active(const MASK_T &l, SIMD_T &v, FCN_T &&fcn)
    {
      // NOTE(jda) - need to static_assert() FCN_T's signature
      static_assert(std::is_same<typename MASK_T::value_type,
                                 typename SIMD_T::value_type>::value,
                    "The LOGICAL_T and SIMD_T types provided must be of the "
                    "same value type. In other words, you can't mismatch the "
                    "mask type and simd type. (ex: can't do vmaskf with vint)");

      for (int i = 0; i < SIMD_T::size; ++i) {
        if (l[i]) {
          typename SIMD_T::value_type tmp = v[i];
          fcn(tmp, i);
          v[i] = tmp;
        }
      }
    }

    // select //

    template <typename SIMD_T, typename MASK_T>
    inline SIMD_T select(const MASK_T &mask, const SIMD_T &t, const SIMD_T &f)
    {
      return embree::select(mask, t, f);
    }

    template <typename SIMD_T, typename MASK_T>
    inline SIMD_T select(const MASK_T &mask, const float &t, const SIMD_T &f)
    {
      return embree::select(mask, SIMD_T(t), f);
    }

    template <typename SIMD_T, typename MASK_T>
    inline SIMD_T select(const MASK_T &mask, const SIMD_T &t, const float &f)
    {
      return embree::select(mask, t, SIMD_T(f));
    }

    // NOTE(jda) - Add variants which allow scalar values for either t or f
    //             types.

    template <typename MASK_T, typename T>
    inline vec_t<T, 2> select(const MASK_T &m,
                              const vec_t<T, 2> &t,
                              const vec_t<T, 2> &f)
    {
      return {select(m, t.x, f.x), select(m, t.y, f.y)};
    }

    template <typename MASK_T, typename T>
    inline vec_t<T, 3> select(const MASK_T &m,
                              const vec_t<T, 3> &t,
                              const vec_t<T, 3> &f)
    {
      return {select(m, t.x, f.x), select(m, t.y, f.y), select(m, t.z, f.z)};
    }

    template <typename MASK_T, typename T>
    inline vec_t<T, 4> select(const MASK_T &m,
                              const vec_t<T, 4> &t,
                              const vec_t<T, 4> &f)
    {
      return {select(m, t.x, f.x), select(m, t.y, f.y),
              select(m, t.z, f.z), select(m, t.w, f.w)};
    }

    // Helper functions ///////////////////////////////////////////////////////

    inline simd::vec3f make_vec3f(float x, float y, float z)
    {
      return {vfloat{x}, vfloat{y}, vfloat{z}};
    }

    // NOTE(jda) - This adds a random number generator per C++ translation unit,
    //             which means random number sequences between different .cpp
    //             files may overlap or be identical...beware!
    static thread_local std::minstd_rand generator;
    static std::uniform_real_distribution<float> distribution {0.f, 1.f};
    static inline simd::vfloat randUniformDist()
    {
      simd::vfloat retval;
      simd::foreach_v(retval, [&](float &v, int i) {
        v = distribution(generator);
      });
      return retval;
    }

    ///////////////////////////////////////////////////////////////////////////
    // TEA - Random numbers based on Tiny Encryption Algorithm

    template <typename T, int NUM_ROUNDS = 8>
    inline void tea8(T& v0, T& v1)
    {
      T sum{0};

      for(int i = 0; i < NUM_ROUNDS; i++) {
        sum += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + sum) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + sum) ^ ((v0 >> 5) + 0x7e95761e);
      }
    }

    template <typename T, int NUM_TEA_ROUNDS = 8>
    struct OSPRAY_ALIGN(64) RandomTEA
    {
      RandomTEA(const T &idx, const T &seed) : v0(idx), v1(seed) {}
      simd::vec2f getFloats()
      {
        tea8<T, NUM_TEA_ROUNDS>(v0, v1);
        const float tofloat = 2.3283064365386962890625e-10f; // 1/2^32
        return {simd::cast<vfloat>(v0) * tofloat,
                simd::cast<vfloat>(v1) * tofloat};
      }

      T v0, v1;
    };

    // Missing stuff from EMBC ////////////////////////////////////////////////

    template <typename SIMD_T>
    inline SIMD_T load(const void *from)
    {
      return SIMD_T::load(from);
    }

    template <typename SIMD_T, typename OFFS_T>
    inline void store(const SIMD_T &from, void *to, const OFFS_T &ofs)
    {
      SIMD_T::scatter(to, ofs, from);
    }

    template <typename SIMD_T, typename OFFS_T, typename MASK_T>
    inline void store(const SIMD_T &from, void *to,
                      const OFFS_T &ofs, const MASK_T &mask)
    {
      SIMD_T::scatter(mask, to, ofs, from);
    }

    inline vfloat sin(const vfloat &in)
    {
      vfloat result = in;
      foreach_v(result, [](float &v, int i) { v = std::sin(v); });
      return result;
    }

    inline vfloat cos(const vfloat &in)
    {
      vfloat result = in;
      foreach_v(result, [](float &v, int i) { v = std::cos(v); });
      return result;
    }

  }// namespace simd
}// namespace ospray
