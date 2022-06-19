INCLUDE(${PROJECT_SOURCE_DIR}/cmake/Common.cmake)

SET(GraphicsSources
    src/BasicRenderer.cpp
    src/RenderDevice.cpp
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
)