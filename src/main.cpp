#include "SDL3/SDL_events.h"
#include "base.h"
#include "engine/render.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL_mouse.h>
#include <algorithm>
#include <cstdint>
#include <cstring>

class LangtonBoard {
public:
  // Raw format is 0xAABBGGRR: opaque black, opaque white
  static constexpr uint32_t stateColors[2] = {0xFF000000, 0xFFFFFFFF};

  Vector2I dimensions;
  uint8_t *board;
  ProceduralTextureRender tex;

  LangtonBoard(App *app, int width, int height)
      : dimensions(width, height), board(new uint8_t[width * height]()),
        tex(app->ctx, (uint16_t)width, (uint16_t)height) {
    tex.region = TextureRegion(); // zeroes destUsed/targetUsed: fill the window
    for (int i = 0; i < width * height; i++)
      tex.m_tex[i] = stateColors[0];
  }
  ~LangtonBoard() { delete[] board; }

  // Not copyable (owns raw memory and registers itself for drawing)
  LangtonBoard(const LangtonBoard &) = delete;
  LangtonBoard &operator=(const LangtonBoard &) = delete;

  // Requires: int Vector2I::getAsInt(int width) const { return y * width + x; }
  inline int idx(const Vector2I &pos) const {
    return pos.getAsInt(dimensions.x);
  }

  uint8_t getSquare(const Vector2I &pos) const { return board[idx(pos)]; }

  void setSquare(const Vector2I &pos, uint8_t state) {
    int i = idx(pos);
    board[i] = state;
    tex.m_tex[i] = stateColors[state];
  }

  void clear() {
    const int n = dimensions.x * dimensions.y;
    std::memset(board, 0, n);
    std::fill(tex.m_tex, tex.m_tex + n, stateColors[0]);
  }
};

class Ant {
public:
  Vector2I position;
  int rotation; // 0 = north, 1 = east, 2 = south, 3 = west
  LangtonBoard *board;

  Ant(LangtonBoard *b)
      : position(b->dimensions.x / 2, b->dimensions.y / 2), rotation(0),
        board(b) {}

  static constexpr int DX[4] = {0, 1, 0, -1};
  static constexpr int DY[4] = {-1, 0, 1, 0};

  inline void stepForward(Vector2I &pos, int rot) {
    pos.x += DX[rot & 3];
    pos.y += DY[rot & 3];
  }

  void evaluate() {
    uint8_t cell = board->getSquare(position);

    if (cell == 0)
      rotation = (rotation + 1) % 4; // black: turn right
    else
      rotation = (rotation + 3) % 4; // white: turn left

    board->setSquare(position, cell ^ 1); // flip the square we're leaving
    stepForward(position, rotation);

    // Wrap around the edges
    position.x = (position.x + board->dimensions.x) % board->dimensions.x;
    position.y = (position.y + board->dimensions.y) % board->dimensions.y;
  }
};

class EmergentGarden : public App {
public:
  static constexpr int W = 800;
  static constexpr int H = 600;

  LangtonBoard board{this, W, H};
  Ant ant{&board};

  float stepsPerTick = 30.0f;
  float stepAccumulator = 0.0f;

  float zoom = 1.0f; // whole screen is 1
  Vector2 zoomPosition;

  int iterationCounter = 0;

  void applyView() {
    float viewWidth = W / zoom, viewHeight = H / zoom;
    zoomPosition.x = std::clamp(zoomPosition.x, 0.0f, W - viewWidth);
    zoomPosition.y = std::clamp(zoomPosition.y, 0.0f, H - viewHeight);
    TextureRegion &r = board.tex.region;
    r.targetUsed = true;
    r.targetPosition = Vector2I((int)zoomPosition.x, (int)zoomPosition.y);
    r.target = Vector2I((int)viewWidth, (int)viewHeight);
    r.destUsed = false;
  }

  void init() override {}

  void update() override {
    stepAccumulator += stepsPerTick;
    int steps = (int)stepAccumulator; // whole steps to run this tick
    stepAccumulator -= steps;         // keep the fractional remainder

    for (int i = 0; i < steps; i++) {
      ant.evaluate();
      iterationCounter++;
    }

    // Mark the ant's current position in red
    board.tex.m_tex[board.idx(ant.position)] =
        Color(255, 0, 0, 255).getRawColor();
  }

  void drawUI() override {
    ImGui::Begin("Diagnostics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Application Average");
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Text("Canvas Size: %d x %d", W, H);
    ImGui::SliderFloat("Steps per tick", &stepsPerTick, 0.03f, 2000.0f, "%.2f",
                       ImGuiSliderFlags_Logarithmic |
                           ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::Button("Restart")) {
      board.clear();
      ant.position = Vector2I(W / 2, H / 2);
      ant.rotation = 0;
      iterationCounter = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button("Toggle Simulation")) {
      paused = !paused;
    }

    if (paused) {
      ImGui::TextColored(ImVec4(255, 0, 0, 255), "Simulation paused");
    } else {
      ImGui::Text("Simulation active");
    }
    ImGui::Separator();
    ImGui::Text("Debug info for simulation:");
    ImGui::TextColored(ImVec4(200, 200, 200, 255), "Iteration counter: %d",
                       iterationCounter);
    ImGui::End();
  }

  void on_event(SDL_Event event, bool *running) override {
    bool uiWantsMouse = ImGui::GetIO().WantCaptureMouse;
    switch (event.type) {
    case SDL_EVENT_MOUSE_WHEEL: {
      if (uiWantsMouse)
        break;
      float mx = event.wheel.mouse_x, my = event.wheel.mouse_y;

      // Board point currently under the cursor
      float wx = zoomPosition.x + mx / zoom;
      float wy = zoomPosition.y + my / zoom;

      zoom *= (event.wheel.y > 0) ? 1.15f : 1.0f / 1.15f;
      zoom = std::clamp(zoom, 1.0f, 32.0f);

      // Re-anchor so that same board point stays under the cursor
      zoomPosition.x = wx - mx / zoom;
      zoomPosition.y = wy - my / zoom;
      applyView();
      break;
    }
    case SDL_EVENT_MOUSE_MOTION:
      if (uiWantsMouse)
        break;
      if (event.motion.state & SDL_BUTTON_LMASK) {
        zoomPosition.x -= event.motion.xrel / zoom;
        zoomPosition.y -= event.motion.yrel / zoom;
        applyView();
      }
      break;
    }
  }
};

int main() {
  EmergentGarden app;
  app.run();
  return 0;
}
