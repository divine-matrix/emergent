#include "SDL3/SDL_events.h"
#include "SDL3/SDL_stdinc.h"
#include "base.h"
#include "engine/render.h"
#include <cmath> // Required for sin() and cos()

const int TARGET_FPS = 60;
const Uint64 FRAME_TARGET_MS = 1000 / TARGET_FPS;

class EmergentGarden : public App {
public:
  // Create an 800x600 canvas
  ProceduralTextureRender canvas{ctx, 800, 600};
  float timeCounter = 0.0f; // Used to animate the waves over time

  void init() override {
    canvas.region = TextureRegion();
    canvas.region.destUsed = false;
    canvas.region.targetUsed = false;
  }

  void update() override {
    // Increment time every frame to drive the animation
    timeCounter += 0.03f;

    int width = 800;
    int height = 600;

    // Loop through every pixel and generate a dynamic procedural plasma effect
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {

        // Mathematical wave calculations based on coordinates and time
        float v1 = std::sin(x * 0.01f + timeCounter);
        float v2 = std::cos(y * 0.01f + timeCounter);
        float v3 = std::sin((x + y) * 0.005f + timeCounter * 0.5f);
        float rawWave = v1 + v2 + v3; // Combined wave value

        // Map wave output to RGB color channels (0 to 255)
        uint8_t r = (uint8_t)(std::sin(rawWave + 0.0f) * 127.0f + 128.0f);
        uint8_t g = (uint8_t)(std::sin(rawWave + 2.0f) * 127.0f + 128.0f);
        uint8_t b = (uint8_t)(std::sin(rawWave + 4.0f) * 127.0f + 128.0f);

        // Pack into your Color class and write to the raw pixel buffer
        canvas.m_tex[y * width + x] = Color(r, g, b, 255).getRawColor();
      }
    }
  }

  void drawUI() override {
    // Begin a floating ImGui window titled "Diagnostics"
    ImGui::Begin("Diagnostics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Display built-in ImGui framerate metrics
    ImGui::Text("Application Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    ImGui::Separator();

    // Display custom diagnostic info
    ImGui::Text("Canvas Size: 800 x 600");
    // ImGui::Text("Active Objects: %zu", ctx.drawCalls.size());

    ImGui::End();
  }

  void on_event(SDL_Event event, bool *running) override {
    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      *running = false;
      break;
    }
  }
};

int main() {
  EmergentGarden app;
  app.run();
  return 0;
}
