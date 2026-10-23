#pragma once
#include "base.h"
#include <algorithm>

constexpr int WINDOW_W = 800;
constexpr int WINDOW_H = 600;

// Zoom/pan state, shared by every simulation
struct View {
  int w, h;          // canvas size in texture pixels
  float zoom = 1.0f; // 1 = whole canvas visible
  Vector2 pos;       // top-left of visible area, in canvas pixels

  View(int w, int h) : w(w), h(h) {}

  void reset() {
    zoom = 1.0f;
    pos = Vector2(0, 0);
  }

  // Mouse positions are in window pixels
  void zoomAt(float mx, float my, float wheelY) {
    float fx = mx / WINDOW_W, fy = my / WINDOW_H;
    float wx = pos.x + fx * (w / zoom);
    float wy = pos.y + fy * (h / zoom);
    zoom *= (wheelY > 0) ? 1.15f : 1.0f / 1.15f;
    zoom = std::clamp(zoom, 1.0f, 32.0f);
    pos.x = wx - fx * (w / zoom);
    pos.y = wy - fy * (h / zoom);
  }

  void drag(float dx, float dy) {
    pos.x -= dx / WINDOW_W * (w / zoom);
    pos.y -= dy / WINDOW_H * (h / zoom);
  }

  void apply(TextureRegion &r) {
    float vw = w / zoom, vh = h / zoom;
    pos.x = std::clamp(pos.x, 0.0f, w - vw);
    pos.y = std::clamp(pos.y, 0.0f, h - vh);
    r.targetUsed = true;
    r.destUsed = false;
    r.targetPosition = Vector2I((int)pos.x, (int)pos.y);
    r.target = Vector2I((int)vw, (int)vh);
  }
};

class Simulation {
public:
  ProceduralTextureRender tex; // registers itself for drawing on construction
  View view;

  Simulation(GlobalContext &ctx, int w, int h) : tex(ctx, w, h), view(w, h) {
    tex.region = TextureRegion(); // zeroed: draw the whole texture
  }
  virtual ~Simulation() = default;

  virtual void update() = 0; // one fixed-rate tick
  virtual void drawUI() = 0; // simulation-specific ImGui controls
  virtual void reset() = 0;
};
