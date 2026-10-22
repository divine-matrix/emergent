#include "render.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "base.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <iostream>

void StyleColorsGrayscale() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
  colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

  colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

  colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);

  colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
  colors[ImGuiCol_ButtonHovered] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
  colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);

  colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

  colors[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);

  style.WindowRounding = 3.0f;
  style.FrameRounding = 2.0f;
}

bool Engine::renderInit(const char *title, int width, int height) {
  this->width = width;
  this->height = height;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return false;
  }

  if (!SDL_CreateWindowAndRenderer(title, width, height, 0, &m_window,
                                   &m_renderer)) {
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  StyleColorsGrayscale();

  // Initialize SDL3 and SDLRenderer3 backends
  ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer);
  ImGui_ImplSDLRenderer3_Init(m_renderer);

  return true;
}

void Engine::cleanup() {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  if (m_renderer)
    SDL_DestroyRenderer(m_renderer);
  if (m_window)
    SDL_DestroyWindow(m_window);
}

bool Engine::_handle_event(App *app) {
  bool running = true;
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    app->on_event(event, &running);
  }
  return running;
}

void Engine::draw() {
  SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
  SDL_RenderClear(m_renderer);
  for (RenderObject *obj : ctx.drawCalls) {
    obj->draw();
  }
}

bool Engine::renderFrame(App *app) {
  if (!_handle_event(app)) {
    return false;
  }
  draw();

  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  app->drawUI();

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
  SDL_RenderPresent(m_renderer);
  return true;
}

bool App::run() {
  if (!engine.renderInit("Emergent Garden", 800, 600)) {
    return false;
  }
  ctx.g_renderer = &engine;
  init();

  Uint64 last_time = SDL_GetTicks();
  double accumulator = 0.0;
  const double target_step = 1.0 / 60.0;

  bool is_running = true;
  while (is_running) {
    Uint64 current_time = SDL_GetTicks();
    double frame_time = (current_time - last_time) / 1000.0;
    last_time = current_time;

    // Prevent spiral of death if a frame lags
    if (frame_time > 0.25)
      frame_time = 0.25;

    accumulator += frame_time;

    // 1. Catch up on fixed-rate simulation steps
    while (accumulator >= target_step) {
      // If event handling needs to happen per tick, call it here or keep it in
      // renderFrame
      update();
      accumulator -= target_step;
    }

    // 2. Render exactly once per physical frame
    if (!engine.renderFrame(this)) {
      is_running = false;
      break;
    }
  }

  engine.cleanup();
  return true;
}
SDL_Texture *Engine::createTexture(int w, int h, SDL_TextureAccess accessMode) {
  return SDL_CreateTexture(m_renderer, ctx.pixelFormat, accessMode, w, h);
}

void Engine::updateTexture(SDL_Texture *tex, void *pixels, int w) {
  SDL_UpdateTexture(tex, nullptr, pixels, w * sizeof(uint32_t));
}

void Engine::renderTexture(SDL_Texture *tex, TextureRegion region) {

  SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

  // Declare the actual rectangles
  SDL_FRect captureRect;
  SDL_FRect destRect;

  // Declare pointers that default to nullptr
  const SDL_FRect *capturePtr = nullptr;
  const SDL_FRect *destPtr = nullptr;

  // Set up capture rect if used
  if (region.targetUsed) {
    captureRect = {.x = (float)region.targetPosition.x,
                   .y = (float)region.targetPosition.y,
                   .w = (float)region.target.x,
                   .h = (float)region.target.y};
    capturePtr = &captureRect; // Point to our rect
  }

  // Set up destination rect if used
  if (region.destUsed) {
    destRect = {.x = (float)region.pos.position.x,
                .y = (float)region.pos.position.y,
                .w = (float)region.base.x * region.pos.scale.x,
                .h = (float)region.base.y * region.pos.scale.y};
    destPtr = &destRect; // Point to our rect
  }

  // Pass the pointers (will be nullptr if targetUsed/destUsed were false)
  SDL_RenderTexture(m_renderer, tex, capturePtr, destPtr);
}
