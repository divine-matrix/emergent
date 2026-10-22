#include "base.h"
#include "SDL3/SDL_render.h"
#include "render.h"
#include <cstdint>
#include <sys/types.h>

uint32_t Color::getRawColor() {
  // Use left-shifts (<<) to place each channel into its correct byte position
  return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)g << 8) |
         (uint32_t)r;
}
Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    : r(r), g(g), b(b), a(a) {};

// Pass the necessary parameters through to the base class
RenderObject::RenderObject(GlobalContext &context) : Object(context) {
  object_uid = ctx.currentID;
  ctx.currentID++;
  ctx.drawCalls.push_back(this);
}

RenderObject::~RenderObject() {
  // Automatically remove it from drawCalls when destroyed to prevent crashes
  auto &calls = ctx.drawCalls;
  calls.erase(std::remove(calls.begin(), calls.end(), this), calls.end());
}

// Temporary fix to correct VTable errors
bool RenderObject::draw() { return false; }

ProceduralTextureRender::ProceduralTextureRender(GlobalContext &context,
                                                 uint16_t width,
                                                 uint16_t height)
    : RenderObject(context), width(width), height(height) {
  m_tex = new uint32_t[width * height];
  texture = nullptr;
}

ProceduralTextureRender::~ProceduralTextureRender() {
  delete[] m_tex;
  SDL_DestroyTexture(texture);
}

bool ProceduralTextureRender::draw() {
  // Lazy initialization: create the texture if it hasn't been created yet
  if (!texture && ctx.g_renderer) {
    texture = ctx.g_renderer->createTexture(width, height,
                                            SDL_TEXTUREACCESS_STREAMING);
    SDL_SetTextureScaleMode(texture,
                            SDL_SCALEMODE_NEAREST); // Optional: crisp pixels
  }

  // Safety check in case renderer still isn't available
  if (!texture || !m_tex || !ctx.g_renderer) {
    return false;
  }

  // updates texture with SDL
  ctx.g_renderer->updateTexture(texture, m_tex, width);

  // render texture to screen
  ctx.g_renderer->renderTexture(texture, region);

  return true;
}
