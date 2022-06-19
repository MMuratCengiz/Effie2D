#pragma once

#include "Effie/InitSystem.h"
#include "Effie/BasicRenderer.h"
#include "RenderWindow.h"

namespace Effie {

struct WorldOptions {
    std::string Title;
    uint32_t InitialWidth;
    uint32_t InitialHeight;
    bool Fullscreen;
};

class World {
public:
    NO_COPY(World)

    explicit World( const WorldOptions& config );

    void Run();
    void Update();
private:
    std::unique_ptr<RenderWindow> renderWindow;
    std::unique_ptr<BasicRenderer> renderer;
    bool running = true;
};

}
