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

#pragma once

// embree
#include "embree2/rtcore.h"

#include "../camera/Camera.h"
#include "../common/DifferentialGeometry.h"
#include "../common/ScreenSample.h"
#include "../geometry/Geometry.h"
#include "Renderer.h"

namespace ospray {
  namespace cpp_renderer {

    struct StreamRenderer : public ospray::cpp_renderer::Renderer
    {
      virtual std::string toString() const override;

      virtual void renderTile(void *perFrameData,
                              Tile &tile,
                              size_t jobID) const override;

      virtual void renderStream(void *perFrameData,
                                ScreenSampleStream &stream) const = 0;

    protected:

      void traceRays(RayStream &rays, RTCIntersectFlags flags) const;

    };

    // Inlined member functions ///////////////////////////////////////////////

    inline void StreamRenderer::traceRays(RayStream &rays,
                                          RTCIntersectFlags flags) const
    {
      RTCIntersectContext ctx{flags, nullptr};
      rtcIntersect1M(model->embreeSceneHandle, &ctx,
                     (RTCRay*)&rays, rays.size(), sizeof(Ray));
    }

  }// namespace cpp_renderer
}// namespace ospray