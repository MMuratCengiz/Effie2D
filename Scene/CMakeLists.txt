INCLUDE(../cmake/Common.cmake)

SET(SceneSources
    src/RenderWindow.cpp
    src/World.cpp
        )

ADD_LIBRARY(
        EffieScene ${EFFIE_LIB_TYPE} ${SceneSources}
)



TARGET_INCLUDE_DEFAULT_DIRECTORIES(EffieScene)

TARGET_LINK_LIBRARIES(
        EffieScene
        PUBLIC
        EffieGraphics
)

#Re-enable once Dawn supports install targets
#ADD_DEPENDENCIES(EffieScene EffieGraphics)
#INSTALL_TARGET(EffieScene)