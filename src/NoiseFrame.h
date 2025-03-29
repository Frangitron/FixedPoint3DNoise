#ifndef NOISE_FRAME_H
#define NOISE_FRAME_H

#include <cmath>
#include <cstdio>
#include <SDL3/SDL.h>


class NoiseFrame {
public:
    NoiseFrame(SDL_Renderer* renderer, int width, int height)
        : renderer_(renderer), width_(width), height_(height), texture_(nullptr) {
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width_, height_);
        if (!texture_) {
            printf("Failed to create gradient texture: %s\n", SDL_GetError());
            return;
        }
    }

    void PixelAt(const int x, const int y, Uint32* pixel) const {

        // Calculate gradient color
        Uint8 r = (Uint8)((float)x / width_ * 255.0f); // Horizontal red gradient
        Uint8 g = (Uint8)((float)y / height_ * 255.0f); // Vertical green gradient
        Uint8 b = 255 - static_cast<Uint8>(cos(static_cast<float>(SDL_GetTicks()) / 1000.0) * 128 + 128);
        Uint8 a = 255;                                  // Fully opaque

        *pixel = (r << 24) | (g << 16) | (b << 8) | a;
    }

    void Update() const {
        void* pixels = nullptr;
        int pitch = 0;
        if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
            for (int y = 0; y < height_; ++y) {
                for (int x = 0; x < width_; ++x) {
                    // Cast to Uint32 pointer
                    Uint32* pixel_ptr = (Uint32*)((Uint8*)pixels + y * pitch) + x;
                    PixelAt(x, y, pixel_ptr);
                }
            }
            SDL_UnlockTexture(texture_);
        } else {
            printf("Failed to lock gradient texture: %s\n", SDL_GetError());
        }
    }

    [[nodiscard]] SDL_Texture* GetTexture() const {
        Update();
        return texture_;
    }

    ~NoiseFrame() {
        if (texture_) {
            SDL_DestroyTexture(texture_);
        }
    }

private:
    SDL_Renderer* renderer_;
    int width_;
    int height_;
    SDL_Texture* texture_;
};

#endif //NOISE_FRAME_H
