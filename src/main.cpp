#include <cstdio>

#include <SDL3/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "FixedPoint3DNoise.hpp"

#include "NoiseFrame.h"


int main(int argc, char* argv[]) {
    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create a SDL3 window
    SDL_Window* window = SDL_CreateWindow("SDL3 + ImGui Example", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("Failed to create SDL3 window: %s\n", SDL_GetError());
        return -1;
    }

    // Create a SDL3 renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        printf("Failed to create SDL3 renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return -1;
    }

    NoiseFrame noiseFrame(renderer, 256, 256);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Apply ImGui's default dark theme
    ImGui::StyleColorsDark();

    // Initialize ImGui backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Main application loop
    bool running = true;
    SDL_Event event;

    FixedPoint3DNoise::Params params = { 8, 1, 0, FixedPoint3DNoise::Scale };

    while (running) {

        //
        // Poll events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            ImGui_ImplSDL3_ProcessEvent(&event);
        }

        //
        // ImGui Noise Frame
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Noise");

        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        noiseFrame.Update(params);
        SDL_Texture* noiseTexture = noiseFrame.GetTexture();

        ImGui::Image(reinterpret_cast<ImTextureID>(noiseTexture), ImVec2(512, 512));
        ImGui::SliderInt("Min", &params.min, 0, FixedPoint3DNoise::Scale);
        ImGui::SliderInt("Max", &params.max, 0, FixedPoint3DNoise::Scale);

        FixedPoint3DNoise::ComputeInfo info = noiseFrame.getComputeInfo();
        ImGui::Text("MMin: %d - MMax: %d", info.min, info.max);

        ImGui::End();
        ImGui::Render();

        //
        // SDL Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Clear the screen to black
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer); // Present the frame
    }

    // Cleanup resources
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
