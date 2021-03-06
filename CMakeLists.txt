## ======================================================================== ##
## Copyright 2009-2016 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

option(OSPRAY_MODULE_CPP "Build 'C++' module (renderers in all C++ code)" ON)

if (OSPRAY_MODULE_CPP)

  set(OSPRAY_MODULE_CPP_STREAM_SIZE ${OSPRAY_TILE_SIZE} CACHE INT "Stream size")
  add_definitions(-DSTREAM_SIZE=${OSPRAY_MODULE_CPP_STREAM_SIZE})

  include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/..
  )

  set(CMAKE_CXX_FLAGS "-march=native ${CMAKE_CXX_FLAGS}")

  add_subdirectory(embc)

  set(LIBRARY_SRCS

    common/simd.h

    camera/Camera.h
    camera/CameraN.h
    camera/Perspective.cpp
    camera/PerspectiveN.cpp

    common/DifferentialGeometry.h
    common/DifferentialGeometryN.h
    common/Ray.h
    common/RayN.h
    common/ScreenSample.h
    common/ScreenSampleN.h

    geometry/Geometry.h
    geometry/TriangleMesh.cpp

    lights/Light.cpp
    lights/AmbientLight.cpp
    lights/DirectionalLight.cpp

    renderer/Renderer.cpp
    renderer/SimdRenderer.cpp

    # Scalar
    renderer/raycast/Raycast.cpp
    renderer/scivis/SciVis.cpp
    renderer/scivis/SciVisShadingInfo.h
    renderer/simple_ao/ao_util.cpp
    renderer/simple_ao/SimpleAO.cpp
    renderer/volume/DVR.cpp

    # Stream
    common/Stream.h
    renderer/StreamRenderer.cpp
    renderer/raycast/StreamRaycast.cpp
    renderer/scivis/StreamSciVis.cpp
    renderer/simple_ao/StreamSimpleAO.cpp

    # Simd
    renderer/raycast/SimdRaycast.cpp
    renderer/simple_ao/ao_util_simd.h
    renderer/simple_ao/SimdSimpleAO.cpp

    transferFunction/TransferFunction.cpp
    transferFunction/LinearTransferFunction.cpp

    volume/Volume.cpp
    volume/StructuredVolume.cpp
    volume/BlockBrickedVolume.cpp
    volume/GhostBlockBrickedVolume.cpp

    util.cpp
  )

  if (OSPRAY_USE_EMBREE_STREAMS)
    add_definitions(-DUSE_EMBREE_STREAMS)
  endif()

  ospray_create_library(ospray_module_cpp
    ${LIBRARY_SRCS}
    LINK
    embc_simd
    ospray
  )

  ospray_create_application(ospCppViewer
    app/cpp_viewer.cpp
    app/cpp_nodes.cpp
    app/importOBJ_cpp.cpp
  LINK
    ospray
    ospray_common
    ospray_imgui3d_sg
    ospray_sg
  )

endif()
