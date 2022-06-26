INCLUDE(${PROJECT_SOURCE_DIR}/cmake/Common.cmake)

FIND_PACKAGE(spirv_cross_core CONFIG REQUIRED)
FIND_PACKAGE(spirv_cross_glsl CONFIG REQUIRED)
FIND_PACKAGE(spirv_cross_reflect CONFIG REQUIRED)
FIND_PACKAGE(spirv_cross_util CONFIG REQUIRED)
FIND_PACKAGE(spirv_cross_cpp CONFIG REQUIRED)
FIND_PACKAGE(spirv_cross_hlsl CONFIG REQUIRED)
FIND_PACKAGE(spirv_cross_msl CONFIG REQUIRED)
FIND_PACKAGE(spirv_cross_c CONFIG REQUIRED)
FIND_PACKAGE(glslang CONFIG REQUIRED)

SET(GraphicsSources
    src/BasicRenderer.cpp
    src/RenderDevice.cpp
    src/ShaderReflection.cpp
    src/SpirvHelper.cpp
    src/RenderPipeline.cpp
)

ADD_LIBRARY(
        EffieGraphics ${EFFIE_LIB_TYPE} ${GraphicsSources}
)

TARGET_INCLUDE_DEFAULT_DIRECTORIES(EffieGraphics)
TARGET_LINK_LIBRARIES(
    EffieGraphics
    PUBLIC
    EffieCommon
    dawncpp
    dawn_proc
    dawn_common
    dawn_platform
    dawn_native
    dawn_utils
    dawn_sample_utils
    HLSL
    SPIRV
    glslang
    OGLCompiler
    spirv-cross-glsl
    spirv-cross-hlsl
    spirv-cross-cpp
    spirv-cross-reflect
    spirv-cross-msl
    spirv-cross-util
    spirv-cross-core
)

#Re-enable once Dawn supports install targets
#ADD_DEPENDENCIES(EffieGraphics EffieCommon)
#INSTALL_TARGET(EffieGraphics)