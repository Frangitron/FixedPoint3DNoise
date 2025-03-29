#ifndef GRADIENT_H
#define GRADIENT_H

#include <cstdio>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>


class Gradient {
public:
    Gradient(SDL_Renderer* renderer, int width, int height)
        : renderer_(renderer), width_(width), height_(height), texture_(nullptr) {
        // Create a texture with the given dimensions
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width_, height_);
        if (!texture_) {
            printf("Failed to create gradient texture: %s\n", SDL_GetError());
            return;
        }

        // Generate the gradient
        void* pixels = nullptr;
        int pitch = 0;
        if (SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
            for (int y = 0; y < height_; ++y) {
                for (int x = 0; x < width_; ++x) {
                    // Cast to Uint32 pointer
                    Uint32* pixel_ptr = (Uint32*)((Uint8*)pixels + y * pitch) + x;

                    // Calculate gradient color
                    Uint8 r = (Uint8)((float)x / width_ * 255.0f); // Horizontal red gradient
                    Uint8 g = (Uint8)((float)y / height_ * 255.0f); // Vertical green gradient
                    Uint8 b = 255 - r;                              // Blue complementary to red
                    Uint8 a = 255;                                  // Fully opaque

                    *pixel_ptr = (r << 24) | (g << 16) | (b << 8) | a;
                }
            }
            SDL_UnlockTexture(texture_);
        } else {
            printf("Failed to lock gradient texture: %s\n", SDL_GetError());
        }
    }

    ~Gradient() {
        if (texture_) {
            SDL_DestroyTexture(texture_);
        }
    }

    void Render(int window_width, int window_height) const {
        if (!texture_) return;

        // Center the gradient in the window
        SDL_FRect dst_rect = {
            static_cast<float>((window_width - width_) / 2),
            static_cast<float>((window_height - height_) / 2),
            static_cast<float>(width_),
            static_cast<float>(height_)
        };
        SDL_RenderTexture(renderer_, texture_, nullptr, &dst_rect);
    }

private:
    SDL_Renderer* renderer_;
    int width_;
    int height_;
    SDL_Texture* texture_;
};

#endif //GRADIENT_H
