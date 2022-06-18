FUNCTION(COPY_TO_BINARY Dir FileSelect)
    FILE(GLOB_RECURSE FilesInDir "${PROJECT_SOURCE_DIR}/${Dir}/${FileSelect}")
    STRING(LENGTH "${PROJECT_SOURCE_DIR}/${Dir}/" PathLen)

    FOREACH(File IN LISTS FilesInDir)
        GET_FILENAME_COMPONENT(FileParentDir ${File} DIRECTORY)
        SET(FileParentDir ${FileParentDir}/)

        STRING(LENGTH "${File}" FilePathLen)
        STRING(LENGTH "${FileParentDir}" DirPathLen)

        MATH(EXPR FileTrimmedLen "${FilePathLen}-${PathLen}")
        MATH(EXPR DirTrimmedLen "${FilePathLen}-${DirPathLen}")

        STRING(SUBSTRING ${File} ${PathLen} ${FileTrimmedLen} FileStripped)
        STRING(SUBSTRING ${FileParentDir} ${PathLen} ${DirTrimmedLen} DirStripped)

        FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${Dir}")
        FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${Dir}/${DirStripped}")
        CONFIGURE_FILE(${File} ${PROJECT_BINARY_DIR}/${Dir}/${DirStripped} COPYONLY)

        #Todo not sure if this is the best way to do this.
        IF(APPLE)
            FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${Dir}")
            FILE(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${Dir}/${DirStripped}")
            CONFIGURE_FILE(${File} ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${Dir}/${DirStripped} COPYONLY)
        ENDIF()
    ENDFOREACH()
ENDFUNCTION()

FUNCTION(INSTALL_TARGET target)
    IF (BLAZAR_INSTALL_LIBS)
        INSTALL(TARGETS ${target}
                EXPORT ${target}-export
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                )

        INSTALL(EXPORT ${target}-export
                FILE ${target}Targets.cmake
                NAMESPACE BlazarEngine::
                DESTINATION cmake/${target}
                )

        INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
    ENDIF()
ENDFUNCTION()

FUNCTION(TARGET_INCLUDE_DEFAULT_DIRECTORIES target)
    TARGET_INCLUDE_DIRECTORIES(${target}
            PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            )
ENDFUNCTION()

IF(MSVC)
    ADD_COMPILE_OPTIONS(
            $<$<CONFIG:>:/MT>
            $<$<CONFIG:Debug>:/MTd>
            $<$<CONFIG:Release>:/MT>
    )

    SET(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )

    IF (CMAKE_BUILD_TYPE MATCHES "Debug")
        FOREACH(CompilerFlag ${CompilerFlags})
            STRING(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        ENDFOREACH()
    ENDIF()
ENDIF()

INCLUDE(FetchContent)