#ifndef NOISE_FRAME_H
#define NOISE_FRAME_H

#include <cstdio>
#include <SDL3/SDL.h>

#include "Noise.hpp"


class NoiseFrame {
public:
    NoiseFrame(SDL_Renderer* renderer, const int width, const int height)
        : renderer_(renderer), width_(width), height_(height), texture_(nullptr), noiseFixed_(654) {
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width_, height_);
        if (!texture_) {
            printf("Failed to create gradient texture: %s\n", SDL_GetError());
            return;
        }
        Update();
    }

    void PixelAt(const int x, const int y, Uint32* pixel) const {
        const auto z = static_cast<int32_t>(SDL_GetTicks() / 10);
        //const int32_t z = 0;

        int32_t scale = 100;
        int32_t min = 0;
        int32_t max = Noise::FIXED_SCALE;

        int32_t noise = 0;
        for (int i = 0; i < 3; ++i) {
            auto octave = static_cast<int32_t>(pow(2, i));

            auto noiseSample = noiseFixed_.getValue(
                scale * x * octave,
                scale * y * octave,
                scale * z
            );
            noise += noiseSample / octave;
        }

        noise = (noise + Noise::FIXED_SCALE) / 2;

        auto r = static_cast<Uint8>((noise * 255 / Noise::FIXED_SCALE));
        auto g = static_cast<Uint8>((noise * 255 / Noise::FIXED_SCALE));
        auto b = static_cast<Uint8>((noise * 255 / Noise::FIXED_SCALE));
        Uint8 a = 255;
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
                    PixelAt(
                        // (x * Noise::FIXED_SCALE),
                        // (y * Noise::FIXED_SCALE),
                        (x * Noise::FIXED_SCALE) / width_,
                        (y * Noise::FIXED_SCALE) / height_,
                        pixel_ptr
                    );
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
    Noise noiseFixed_;
    SDL_Renderer* renderer_;
    int width_;
    int height_;
    SDL_Texture* texture_;
};

#endif //NOISE_FRAME_H
