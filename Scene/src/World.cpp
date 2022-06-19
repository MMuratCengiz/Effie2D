#include "Effie/World.h"
#include "memory"

using namespace Effie;

World::World(const WorldOptions &config) {
    InitSystem::initEverything();

    renderWindow = std::make_unique<RenderWindow>(config.Title, config.InitialWidth, config.InitialHeight);
//    renderer = std::make_unique<BasicRenderer>(renderWindow->GetSDL_Window(), GraphicsOptions{});
}

void World::Run() {
    while (running) {
        Update();
    }
}

void World::Update() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        running = running && (event.type != SDL_QUIT);
    }

}
