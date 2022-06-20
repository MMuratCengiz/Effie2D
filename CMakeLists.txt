# Note download depot_tools somewhere and add to path
# http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up
# cd external/dawn
# cp scripts/standalone.gclient .gclient
# cd ..
# gclient sync

CMAKE_MINIMUM_REQUIRED(VERSION 3.22)
SET(CMAKE_CXX_STANDARD 23)

SET(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
IF(WIN32)
    SET(VCPKG_TARGET_TRIPLET "x64-windows-static")
ENDIF()

OPTION(EFFIE_INSTALL_LIBS "Install Effie Libraries" ON)
OPTION(EFFIE_BUILD_SHARED "Enable to build shared libraries instead of static, default is off." OFF)

SET(EFFIE_LIB_TYPE STATIC)

IF(EFFIE_BUILD_SHARED)
    SET(EFFIE_LIB_TYPE SHARED)
ENDIF()

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake")
PROJECT(Effie2D VERSION 0.2)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/Common.cmake)

FIND_PACKAGE(SDL2 CONFIG REQUIRED)

SET(DAWN_ENABLE_PIC        ON CACHE BOOL "Position-Independent-Code")
SET(DAWN_ENABLE_DESKTOP_GL OFF CACHE BOOL "OpenGL backend")
SET(DAWN_ENABLE_OPENGLES   OFF CACHE BOOL "OpenGL ES backend")
SET(DAWN_BUILD_EXAMPLES    ON CACHE BOOL "Dawn examples")
SET(TINT_BUILD_SAMPLES     OFF CACHE BOOL "Tint examples")
SET(TINT_BUILD_GLSL_WRITER OFF CACHE BOOL "OpenGL SL writer")

IF (WIN32)
    SET(TINT_BUILD_MSL_WRITER  OFF CACHE BOOL "Metal SL writer")
    SET(TINT_BUILD_SPV_WRITER  OFF CACHE BOOL "SPIR-V writer")
ELSEIF(APPLE)
    SET(TINT_BUILD_MSL_WRITER  ON CACHE BOOL "Metal SL writer")
    SET(TINT_BUILD_HLSL_WRITER OFF CACHE BOOL "DirectX SL writer")
ELSEIF(UNIX)
    SET(TINT_BUILD_MSL_WRITER  OFF CACHE BOOL "Metal SL writer")
    SET(TINT_BUILD_HLSL_WRITER OFF CACHE BOOL "DirectX SL writer")
ENDIF()

ADD_SUBDIRECTORY(external/dawn dawn EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Graphics)
ADD_SUBDIRECTORY(Scene)

ADD_EXECUTABLE(Effie2D main.cpp)
TARGET_COMPILE_OPTIONS(Effie2D PRIVATE -DUNICODE -DENGINE_DLL)
TARGET_LINK_LIBRARIES(Effie2D
        PUBLIC
        EffieScene
        )

COPY_TO_BINARY("Shaders" "*.wgsl")
COPY_TO_BINARY("Assets" "*.*")
