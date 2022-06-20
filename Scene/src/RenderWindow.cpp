#include "Effie/RenderWindow.h"

using namespace Effie;

RenderWindow::RenderWindow(const std::string &title, uint32_t width, uint32_t height) {
    Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	#if __APPLE__
	windowFlags |= SDL_WINDOW_METAL;
	#elif MSVC || _WIN32 || WIN32
	#else
	windowFlags |= SDL_WINDOW_VULKAN;
	#endif

    auto *w = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            (int) width,
            (int) height,
            windowFlags
    );

    if (w == nullptr) {
        LOG(ERROR) << "Could not create SDL2 Window: SDL_Error: " << SDL_GetError();
        exit(-1);
    }

    window = pSDL_Window(w, DESTROYER(SDL_Window *w, SDL_DestroyWindow(w)));
}

RenderWindow::~RenderWindow() = default;