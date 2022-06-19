#include "Effie/InitSystem.h"
#include <string>

using namespace Effie;

void InitSystem::initEverything() {
    google::InitGoogleLogging("BlazarEngine");
    google::SetStderrLogging(google::GLOG_INFO);

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        LOG(ERROR) << "Could not initialize one or more SDL subsystems: SDL_Error: " << SDL_GetError();
        exit(-1);
    }

    LOG(INFO) << "Initialized SDL2.";
}
