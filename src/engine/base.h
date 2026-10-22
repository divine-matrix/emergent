#pragma once
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_render.h"
#include <cstdint>
#include <vector>

// TYPE DEFINITIONS

struct Vector2 {
  float x;
  float y;

  Vector2() : x(0.0f), y(0.0f) {};
  Vector2(float ax, float ay) : x(ax), y(ay) {};
};

struct Vector2I {
  int x;
  int y;

  Vector2I() : x(0), y(0) {};
  Vector2I(int ax, int ay) : x(ax), y(ay) {};
};

// 1. Forward-declare Renderer so GlobalContext knows it's a class
class Engine;

// 2. Define Object and RenderObject FIRST so GlobalContext knows about
// RenderObject
class GlobalContext; // Forward declaration if needed, or place GlobalContext
                     // below

class Transform {
public:
  Vector2I position;
  Vector2I scale;
  // deprecated
  float rotation;
};

struct TextureRegion {
  Transform pos;
  Vector2I base;
  Vector2I targetPosition;
  Vector2I target;
  bool targetUsed;
  bool destUsed;
};

class Color {
public:
  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  uint8_t r, g, b, a;
  // returns color in abgr32 format
  uint32_t getRawColor();
};

class Object {
public:
  Object(GlobalContext &context) : ctx(context) {};
  uint32_t get_object_uid();

protected:
  GlobalContext &ctx;
  uint32_t object_uid;

private:
};

class RenderObject : public Object {
public:
  RenderObject(GlobalContext &context);
  ~RenderObject();
  virtual bool draw();
};

class ProceduralTextureRender : public RenderObject {
public:
  Transform transform;
  uint16_t width, height;
  uint32_t *m_tex;
  SDL_Texture *texture;
  TextureRegion region;

  ProceduralTextureRender(GlobalContext &context, uint16_t width,
                          uint16_t height);
  ~ProceduralTextureRender();

  bool draw() override;
};
// 3. Now GlobalContext can safely use RenderObject and Renderer*
class GlobalContext {
public:
  Engine *g_renderer;
  std::vector<RenderObject *> drawCalls;
  uint32_t currentID = 0;
  SDL_PixelFormat pixelFormat = SDL_PIXELFORMAT_RGBA32;
  void set_renderer(Engine *ren) { g_renderer = ren; }
};
