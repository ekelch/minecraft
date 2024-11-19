#include <stdio.h>
#include "SDL_events.h"
#include "SDL_render.h"
#include "SDL_video.h"
#include "stdbool.h"
#include <SDL.h>
#include <SDL_image.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int MAX = 9;
const int SPRITE_RES = 16;

typedef struct LTexture{
    SDL_Texture* texture;
    int w;
    int h;

    bool (*loadTexture)(char* path, struct LTexture *this);
    void (*setColor)(Uint8 r, Uint8 g, Uint8 b, struct LTexture *this);
    void (*setBlendMode)(SDL_BlendMode mode, struct LTexture *this);
    void (*setAlpha)(Uint8 a, struct LTexture* this);
    void (*render)(int x, int y, SDL_Rect* clip, struct LTexture *this);
    void (*free)(struct LTexture *this);
} LTexture;

typedef struct LBlock {
    int x;
    int y;
    int z;

    SDL_Texture* texture;
    void (*render)(int x, int y, SDL_Rect* clip, struct LBlock *this);
} LBlock;

typedef struct LViewmodel {
    int x;
    int y;
    int z;
    float fov;
    float theta;
    float phi;
} LViewmodel;

SDL_Event e;
SDL_Window* gWindow;
SDL_Renderer* gRenderer;

LBlock blocks[MAX][MAX][MAX];
LTexture lSprites;
LViewmodel vm;

LTexture newTexture();
LBlock newBlock(int x, int y, int z);
LViewmodel newViewmodel(int x, int y, int z);

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
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        for (int x = 0; x < MAX; x++) {
            for (int z = 0; z < MAX; z++) {
                LBlock nb = newBlock(x, 0, z);
                blocks[x][0][z] = nb;
                nb.render(x * SPRITE_RES, )
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0);
        SDL_RenderClear(gRenderer);

        SDL_Rect bClip = {0, 0, SPRITE_RES, SPRITE_RES};
        lSprites.render(20, 20, &bClip, &lSprites);

        SDL_Rect gClip = {SPRITE_RES, 0, SPRITE_RES, SPRITE_RES};
        lSprites.render(20, 40, &gClip, &lSprites);

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
    vm = newViewmodel(0, 0, 0);
    return true;
}
bool loadMedia() {
    lSprites = newTexture();
    if (!lSprites.loadTexture("./resources/mainSprite.png", &lSprites)) {
        return false;
    };
    return true;
}

void close() {
    lSprites.free(&lSprites);

    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    IMG_Quit();
    SDL_Quit();
}

//LTexture
bool loadTexture(char *path, LTexture *this) {
    // Load image at path
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Failed to load surface from %s!\nSDL_image Error: %s\n", path, IMG_GetError());
        return false;
    }

    // surface to key, enable keying, pixel to key with (cyan//mapRGB is cross platform compatible), loaded surface has format var
    SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface -> format, 0, 255, 255));

    // Creating texture from surface pixels
    this->texture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
    if (this->texture == NULL) {
        printf("Unable to create texture from %s!\nSDL_image Error: %s\n", path, IMG_GetError());
        return false;
    }
    this->w = loadedSurface->w;
    this->h = loadedSurface->h;
    SDL_FreeSurface(loadedSurface);
    return true;
}

void renderTexture(int x, int y, SDL_Rect* clip, struct LTexture *this) {
    SDL_Rect renderQuad = {x, y, this->w, this->h};
    if (clip != NULL) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopy(gRenderer, this->texture, clip, &renderQuad);
}

void freeTexture(struct LTexture *this) {
    if (this->texture != NULL) {
        SDL_DestroyTexture(this->texture);
        this->texture = NULL;
        this->w = 0;
        this->h = 0;
    }
    free(this);
}

void setColor(Uint8 r, Uint8 g, Uint8 b, struct LTexture *this) {
    SDL_SetTextureColorMod(this->texture, r, g, b);
}

void setBlendMode(SDL_BlendMode blendMode, LTexture *this) {
    SDL_SetTextureBlendMode(this->texture, blendMode);
}

void setAlpha(Uint8 a, LTexture *this) {
    SDL_SetTextureAlphaMod(this->texture, a);
}

LTexture newTexture() {
    LTexture *lt = malloc(sizeof(LTexture));
    lt->texture = NULL;
    lt->w = 0;
    lt->h = 0;
    lt->loadTexture = loadTexture;
    lt->setColor = setColor;
    lt->setBlendMode = setBlendMode;
    lt->setAlpha = setAlpha;
    lt->render = renderTexture;
    lt->free = freeTexture;
    return *lt;
}

LBlock newBlock(int x, int y, int z) {
    LBlock* lb = malloc(sizeof(LBlock));
    lb->x = x;
    lb->y = y;
    lb->z = z;
    return *lb;
}

LViewmodel newViewmodel(int x, int y, int z) {
    LViewmodel* vm = malloc(sizeof(LViewmodel));
    vm->x = x;
    vm->y = y;
    vm->z = z;
    vm->fov = 70.0;
    vm->theta = 0.0;
    vm->phi = 0.0;
    return *vm;
}
//LTexture
