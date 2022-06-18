CMAKE_MINIMUM_REQUIRED(VERSION 3.22)
SET(CMAKE_CXX_STANDARD 23)

SET(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
IF(WIN32)
    SET(VCPKG_TARGET_TRIPLET "x64-windows-static")
ENDIF()

INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake")
PROJECT(Effie2D VERSION 0.2)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/Common.cmake)

FIND_PACKAGE(SDL2 CONFIG REQUIRED)
FIND_PACKAGE(Vulkan REQUIRED)

ADD_SUBDIRECTORY(external/DiligentEngine/DiligentCore)

ADD_EXECUTABLE(Effie2D main.cpp)
TARGET_COMPILE_OPTIONS(Effie2D PRIVATE -DUNICODE -DENGINE_DLL)
TARGET_INCLUDE_DIRECTORIES(Effie2D PRIVATE "external/DiligentEngine/DiligentCore")
TARGET_LINK_LIBRARIES(Effie2D
        PRIVATE
        Diligent-GraphicsEngineD3D12-shared
        Diligent-GraphicsEngineVk-shared
        SDL2::SDL2
        SDL2::SDL2main
        SDL2::SDL2-static
        Vulkan::Vulkan
        )

COPY_TO_BINARY("shaders" "*.glsl")
COPY_TO_BINARY("assets" "*.*")

COPY_REQUIRED_DLLS(Effie2D)
