#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <vector>

#define SDLMustBeTrue(expr)                                                    \
    if (!(expr)) {                                                             \
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;             \
        exit(EXIT_FAILURE);                                                    \
    }

#pragma - deprecate("Old renderer")
std::vector<const char *> getRequiredExtensions(SDL_Window *window,
                                                bool enableValidationLayers);
bool checkValidationLayerSupport();
