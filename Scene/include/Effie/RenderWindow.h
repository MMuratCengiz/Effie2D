#pragma once

#include <string>
#include <functional>
#include "Effie/CommonIncludes.h"
#include "SDL2/SDL_syswm.h"

namespace Effie {

class
RenderWindow {
public:
    RenderWindow(const std::string &title, uint32_t width, uint32_t height);

    inline SDL_Window * GetSDL_Window() {
        return window.get( );
    }

    ~RenderWindow();
private:
    typedef std::unique_ptr<SDL_Window, std::function<void(SDL_Window *)>> pSDL_Window;
    pSDL_Window window;
};

}