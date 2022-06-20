INCLUDE(${PROJECT_SOURCE_DIR}/cmake/Common.cmake)

FIND_PACKAGE(SDL2 REQUIRED)
FIND_PACKAGE(glog CONFIG REQUIRED)

SET(CommonSources
        src/InitSystem.cpp
        src/Utilities.cpp
        )

ADD_LIBRARY(
        EffieCommon ${EFFIE_LIB_TYPE} ${CommonSources}
)

TARGET_INCLUDE_DEFAULT_DIRECTORIES(EffieCommon)
TARGET_LINK_LIBRARIES(
        EffieCommon
        PUBLIC
        glog::glog
        SDL2::SDL2
        SDL2::SDL2main
        SDL2::SDL2-static
)

#Re-enable once Dawn supports install targets
#INSTALL_TARGET(EffieCommon)
INSTALL_TARGET(EffieCommon)