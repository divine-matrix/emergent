#include "SDL3/SDL_events.h"
#include "base.h"
#include "engine/render.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

class EmergentGarden : public App {
public:
  static constexpr int W = 800;
  static constexpr int H = 600;
  static constexpr float TAU = 6.28318530718f;

  ProceduralTextureRender canvas{ctx, W, H};
  float timeCounter = 0.0f;

  // Precomputed lookup tables (keeps per-pixel work cheap)
  std::vector<float> distTable, angTable, vigTable;
  float sinLUT[1024];
  uint8_t palR[256], palG[256], palB[256];

  // Fast sine using lookup table
  inline float sinT(float x) const {
    return sinLUT[(int)(x * (1024.0f / TAU)) & 1023];
  }

  void init() override {
    canvas.region = TextureRegion();
    canvas.region.destUsed = false;   // fill whole window
    canvas.region.targetUsed = false; // use whole texture

    for (int i = 0; i < 1024; i++)
      sinLUT[i] = std::sin(i * TAU / 1024.0f);

    // Cosine-based rainbow palette
    for (int i = 0; i < 256; i++) {
      float p = i / 255.0f;
      palR[i] = (uint8_t)(255 * (0.5f + 0.5f * std::cos(TAU * (p + 0.00f))));
      palG[i] = (uint8_t)(255 * (0.5f + 0.5f * std::cos(TAU * (p + 0.33f))));
      palB[i] = (uint8_t)(255 * (0.5f + 0.5f * std::cos(TAU * (p + 0.67f))));
    }

    // Distance / angle from center + vignette per pixel
    distTable.resize(W * H);
    angTable.resize(W * H);
    vigTable.resize(W * H);
    const float cx = W / 2.0f, cy = H / 2.0f;
    const float maxD = std::sqrt(cx * cx + cy * cy);
    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        float dx = x - cx, dy = y - cy;
        float d = std::sqrt(dx * dx + dy * dy);
        int i = y * W + x;
        distTable[i] = d;
        angTable[i] = std::atan2(dy, dx);
        vigTable[i] = 1.0f - 0.65f * (d / maxD) * (d / maxD);
      }
    }
  }

  void update() override {
    timeCounter += 0.03f;
    const float t = timeCounter;
    uint32_t *px = canvas.m_tex;

    // --- Plasma + tunnel swirl ---
    for (int y = 0; y < H; y++) {
      const float sy = sinT(y * 0.03f - t * 1.3f);
      for (int x = 0; x < W; x++) {
        const int i = y * W + x;
        float v = sinT(x * 0.025f + t) + sy +
                  sinT((x + y) * 0.015f + t * 0.7f) +
                  sinT(distTable[i] * 0.035f - t * 2.0f + angTable[i] * 3.0f);

        int pi = ((int)(v * 32.0f + t * 40.0f)) & 255;
        float vig = vigTable[i];
        px[i] = Color((uint8_t)(palR[pi] * vig), (uint8_t)(palG[pi] * vig),
                      (uint8_t)(palB[pi] * vig), 255)
                    .getRawColor();
      }
    }

    // --- Orbiting glow orbs (additive) ---
    const int numOrbs = 6;
    const int radius = 70;
    for (int o = 0; o < numOrbs; o++) {
      float phase = o * TAU / numOrbs;
      float ox = W / 2.0f + std::cos(t * 0.8f + phase) *
                                (180 + 60 * std::sin(t * 0.5f + o));
      float oy = H / 2.0f + std::sin(t * 1.1f + phase * 2.0f) *
                                (130 + 40 * std::cos(t * 0.7f + o));

      int x0 = std::max(0, (int)ox - radius),
          x1 = std::min(W - 1, (int)ox + radius);
      int y0 = std::max(0, (int)oy - radius),
          y1 = std::min(H - 1, (int)oy + radius);

      for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
          float dx = x - ox, dy = y - oy;
          float d2 = (dx * dx + dy * dy) / (float)(radius * radius);
          if (d2 >= 1.0f)
            continue;
          float glow = (1.0f - d2);
          glow *= glow;
          int add = (int)(glow * 200.0f);

          uint32_t c = px[y * W + x];
          int r = std::min(255, (int)(c & 0xFF) + add);
          int g = std::min(255, (int)((c >> 8) & 0xFF) + add);
          int b = std::min(255, (int)((c >> 16) & 0xFF) + add);
          px[y * W + x] =
              Color((uint8_t)r, (uint8_t)g, (uint8_t)b, 255).getRawColor();
        }
      }
    }
  }

  void drawUI() override {
    ImGui::Begin("Diagnostics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Application Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Canvas Size: %d x %d", W, H);
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
