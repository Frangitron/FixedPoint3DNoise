#ifndef NOISE_FRAME_H
#define NOISE_FRAME_H

#include <cstdio>
#include <SDL3/SDL.h>

#include "FixedPoint3DNoise.hpp"


class NoiseFrame {
public:
    NoiseFrame(SDL_Renderer* renderer, const int width, const int height)
        : renderer_(renderer), width_(width), height_(height), texture_(nullptr) {

        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width_, height_);
        if (!texture_) {
            printf("Failed to create gradient texture: %s\n", SDL_GetError());
            return;
        }
    }

    static void setPixelAt(const int32_t x, const int32_t y, Uint32* pPixel, Frangitron::FixedPoint3DNoise* pNoise, const float gamma) {
        const auto z = -static_cast<int32_t>(SDL_GetTicks() / 10);
        int32_t noise = pNoise->getValue(x, y, z);
        auto v = static_cast<Uint8>((noise * 255 / Frangitron::FixedPoint3DNoise::Scale));
        v = static_cast<uint8_t>(powf(static_cast<float>(v) / 255.0f, gamma) * 255);
        Uint8 a = 255;
        *pPixel = (v << 24) | (v << 16) | (v << 8) | a;
    }

    void Update(Frangitron::FixedPoint3DNoise* pNoise, const float gamma) {
        void* pixels = nullptr;
        int pitch = 0;
        if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
            for (int y = 0; y < height_; ++y) {
                for (int x = 0; x < width_; ++x) {
                    // Cast to Uint32 pointer
                    Uint32* pPixel = (Uint32*)((Uint8*)pixels + y * pitch) + x;
                    setPixelAt(
                        x * Frangitron::FixedPoint3DNoise::Scale / width_,
                        y * Frangitron::FixedPoint3DNoise::Scale / height_,
                        pPixel,
                        pNoise,
                        gamma
                    );
                }
            }
            SDL_UnlockTexture(texture_);
        } else {
            printf("Failed to lock gradient texture: %s\n", SDL_GetError());
        }
    }

    [[nodiscard]] SDL_Texture* GetTexture() const {
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
