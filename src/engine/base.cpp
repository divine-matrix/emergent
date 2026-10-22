#include "base.h"
#include "SDL3/SDL_render.h"
#include "render.h"
#include <cstdint>

// Pass the necessary parameters through to the base class
RenderObject::RenderObject(GlobalContext &context) : Object(context) {
  object_uid = ctx.currentID;
  ctx.currentID++;
}

// Temporary fix to correct VTable errors
bool RenderObject::draw() { return false; }

ProceduralTextureRender::ProceduralTextureRender(GlobalContext &context,
                                                 uint16_t width,
                                                 uint16_t height)
    : RenderObject(context), width(width), height(height) {
  texture =
      ctx.g_renderer->createTexture(width, height, SDL_TEXTUREACCESS_STREAMING);
}

ProceduralTextureRender::~ProceduralTextureRender() {
  delete[] m_tex;
  SDL_DestroyTexture(texture);
}

bool ProceduralTextureRender::draw() {
  if (!texture || !m_tex || !ctx.g_renderer) {
    return false;
  }

  return true;
}
