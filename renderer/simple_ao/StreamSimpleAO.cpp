// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
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

// ospray
#include "StreamSimpleAO.h"
#include "ao_util.h"
#include "../../util.h"

#include <random>

static thread_local std::minstd_rand generator;

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the StreamSimpleAO renderer
    /*! \detailed Since the StreamSimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct StreamSimpleAOMaterial : public ospray::Material {
      /*! \brief commit the object's outstanding changes
       *         (such as changed parameters etc) */
      void commit() override;

      // -------------------------------------------------------
      // member variables
      // -------------------------------------------------------

      //! \brief diffuse material component, that's all we care for
      vec3f Kd;

      //! \brief diffuse texture, if available
      Ref<Texture2D> map_Kd;
    };

    void StreamSimpleAOMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // StreamSimpleAO definitions /////////////////////////////////////////////

    std::string StreamSimpleAORenderer::toString() const
    {
      return "ospray::cpp_renderer::StreamSimpleAORenderer";
    }

    void StreamSimpleAORenderer::commit()
    {
      ospray::cpp_renderer::Renderer::commit();
      samplesPerFrame = getParam1i("aoSamples", 1);
      aoRayLength     = getParam1f("aoDistance", 1e20f);
    }

    void StreamSimpleAORenderer::renderStream(void */*perFrameData*/,
                                              ScreenSampleStream &stream) const
    {
      traceRays(stream.rays, RTC_INTERSECT_COHERENT);

      DGStream dgs = postIntersect(stream.rays,
                                   DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD|
                                   DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      int nActiveRays = ScreenSampleStream::size;

      for_each_sample(stream,[](ScreenSampleRef sample){ sample.alpha = 1.f; });

      // Disable rays which didn't hit anything
      for_each_sample(
        stream,
        [&](ScreenSampleRef sample){
          sample.rgb = bgColor;
          disableRay(sample.ray);
          nActiveRays--;
        },
        rayMiss
      );

      if (nActiveRays <= 0)
        return;

      // Get material color for rays which did hit something
      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          auto &dg = dgs[i];

          StreamSimpleAOMaterial *mat =
              dynamic_cast<StreamSimpleAOMaterial*>(dg.material);

          if (mat) {
            sample.rgb = mat->Kd;
#if 0// NOTE(jda) - texture fetches not yet implemented
            if (mat->map_Kd) {
              vec4f Kd_from_map = get4f(mat->map_Kd, dg.st);
              sample.rgbcolor *=
                  vec3f(Kd_from_map.x, Kd_from_map.y, Kd_from_map.z);
            }
#endif
          } else {
            sample.rgb = vec3f{1.f};
          }

          // should be done in material:
          sample.rgb *= vec3f{dg.color.x, dg.color.y, dg.color.z};
        },
        rayHit
      );

      Stream<int> hits;
      std::fill(begin(hits), end(hits), 0);

      Stream<ao_context> ao_ctxs;

      RayStream ao_rays;

      for (int j = 0; j < samplesPerFrame; j++) {
        // Setup AO rays for active "lanes"
        for_each_sample_i(
          stream,
          [&](ScreenSampleRef /*sample*/, int i) {
            auto &dg  = dgs[i];
            auto &ctx = ao_ctxs[i];
            ctx = getAOContext(dg, aoRayLength, epsilon);
            ao_rays[i] = calculateAORay(dg, ctx);
          },
          rayHit
        );

        // Trace AO rays
        occludeRays(ao_rays, RTC_INTERSECT_INCOHERENT);

        // Record occlusion test
        for_each_sample_i(
          stream,
          [&](ScreenSampleRef sample, int i) {
            UNUSED(sample);
            auto &ao_ray = ao_rays[i];
            ao_ray.t = aoRayLength;
            if (dot(ao_ray.dir, dgs[i].Ng) < 0.05f || ao_ray.hitSomething())
              hits[i]++;
          },
          rayHit
        );
      }

      // Write pixel colors
      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          float diffuse = ospcommon::abs(dot(dgs[i].Ng, sample.ray.dir));
          sample.rgb *= diffuse * (1.0f - float(hits[i])/samplesPerFrame);
        },
        rayHit
      );
    }

    Material *StreamSimpleAORenderer::createMaterial(const char *type)
    {
      UNUSED(type);
      return new StreamSimpleAOMaterial;
    }

    OSP_REGISTER_RENDERER(StreamSimpleAORenderer, cpp_ao_stream);

  }// namespace cpp_renderer
}// namespace ospray
