#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vector>

std::vector<const char *> getRequiredExtensions(SDL_Window *window,
												bool enableValidationLayers);
bool checkValidationLayerSupport();
