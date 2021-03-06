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

#include "DVR.h"

namespace ospray {
  namespace cpp_renderer {

    static thread_local std::mt19937 rng;

    // Material definition ////////////////////////////////////////////////////

    struct DVMaterial : public ospray::Material
    {
      void commit() override;

      float d;
      vec3f Kd;
      vec3f Ks;
      float Ns;

      Ref<Texture2D> map_d;
      Ref<Texture2D> map_Kd;
      Ref<Texture2D> map_Ks;
      Ref<Texture2D> map_Ns;
    };

    void DVMaterial::commit()
    {
      map_d  = (Texture2D*)getParamObject("map_d", nullptr);
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
      map_Ks = (Texture2D*)getParamObject("map_Ks",
                                          getParamObject("map_ks", nullptr));
      map_Ns = (Texture2D*)getParamObject("map_Ns",
                                          getParamObject("map_ns", nullptr));

      d  = getParam1f("d", 1.f);
      Kd = getParam3f("kd", getParam3f("Kd", vec3f(.8f)));
      Ks = getParam3f("ks", getParam3f("Ks", vec3f(0.f)));
      Ns = getParam1f("ns", getParam1f("Ns", 10.f));
    }

    // DVR definitions ////////////////////////////////////////////////////////

    std::string DVRenderer::toString() const
    {
      return "ospray::cpp_renderer::DVRenderer";
    }

    void DVRenderer::commit()
    {
      cpp_renderer::Renderer::commit();
    }

    void *DVRenderer::beginFrame(FrameBuffer *fb)
    {
      auto &volumes = model->volume;

      if (!volumes.empty()) {
        currentVolume = dynamic_cast<cpp_renderer::Volume*>(volumes[0].ptr);
      }

      return cpp_renderer::Renderer::beginFrame(fb);
    }

    void DVRenderer::renderSample(void *perFrameData,
                                  ScreenSample &sample) const
    {
      UNUSED(perFrameData);

      sample.rgb = bgColor;

      if (currentVolume == nullptr)
        return;

      auto &ray      = sample.ray;
      auto hitVolume = currentVolume->intersect(ray);

      if (hitVolume) {
        vec3f color {0.f};
        float opacity {0.f};
        const auto &volume = *currentVolume;
        const auto &tFcn   = *volume.transferFunction;

        static std::uniform_real_distribution<float> distribution {0.f, 1.f};
        const auto offsetStepSize = (volume.samplingStep / volume.samplingRate);
        ray.t0 += distribution(rng) * offsetStepSize;

        ///////////////////////////////////////////////////////////////////////
        // NOTE(jda) - this section needs to be a function/object!
        while (ray.t0 < ray.t) {
          auto samplePoint  = ray.org + ray.t0 * ray.dir;
          auto volumeSample = volume.computeSample(samplePoint);

          auto sampleColor   = tFcn.color(volumeSample);
          auto sampleOpacity = tFcn.opacity(volumeSample);

          auto clampedOpacity = clamp(sampleOpacity / volume.samplingRate);
          sampleColor *= clampedOpacity;

          color   += (1.f - opacity) * sampleColor;
          opacity += (1.f - opacity) * clampedOpacity;

          if (opacity >= 0.99f)
            break;

          currentVolume->advance(ray);
        }
        ///////////////////////////////////////////////////////////////////////

        sample.rgb *= (1.f - opacity);
        sample.rgb += opacity * color;
      }
    }

    Material *DVRenderer::createMaterial(const char *type)
    {
      UNUSED(type);
      return new DVMaterial;
    }

    OSP_REGISTER_RENDERER(DVRenderer, cpp_dvr);

  }// namespace cpp_renderer
}// namespace ospray
