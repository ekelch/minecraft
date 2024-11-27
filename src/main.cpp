#include "SDL_vulkan.h"
#include <__config>
#include <iostream>
#include <istream>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <stdio.h>
#include "SDL_events.h"
#include "SDL_render.h"
#include "fwd.hpp"
#include "vulkan/vulkan_core.h"
#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <glm.hpp>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SDL_Event e;
SDL_Window* gWindow;
SDL_Renderer* gRenderer;

bool init();
bool loadMedia();

int main(int argc, char *argv[]) {
    bool quit = false;

    if (!init()) {
        printf("Failed to init!\n");
        return 0;
    }
    if (!loadMedia()) {
        printf("Failed to load media!\n");
        return 0;
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported.\n";

    glm::mat4 matrix;
    glm::vec4 vec;
    glm::vec3 test = matrix * vec;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0, 255, 255, 255);
        SDL_RenderClear(gRenderer);

        SDL_RenderPresent(gRenderer);
    }

    return 0;
}

bool init() {
    gWindow = SDL_CreateWindow("minecraft", 69, 69, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        printf("Failed to create window!\n");
        return false;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == NULL) {
        printf("Failed to create renderer!\n");
        return false;
    }
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) && imgFlags)) {
        printf("SDL Image could not be initialized!\nSDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    return true;
}
bool loadMedia() {
    return true;
}

void close() {
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    IMG_Quit();
    SDL_Quit();
}
