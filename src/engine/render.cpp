#include "render.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "base.h"
#include <iostream>

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

  return true;
}

void Engine::cleanup() {
  SDL_DestroyRenderer(m_renderer);
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
  for (RenderObject obj : ctx.drawCalls) {
    obj.draw();
  }
  SDL_RenderPresent(m_renderer);
}

bool Engine::renderFrame(App *app) {
  if (!_handle_event(app)) {
    return false;
  }
  app->update();
  draw();
  app->drawUI();
  return true;
}

bool App::run() {
  ctx.g_renderer = &engine;

  if (!engine.renderInit("Emergent Garden", 800, 600)) {
    std::cout << "An error occurred in the initialization of the engine."
              << std::endl;
    SDL_Quit();
    return true;
  }

  init();

  while (true) {
    Uint64 frame_start_time = SDL_GetTicks();

    if (!engine.renderFrame(this)) {
      break;
    }
    // VSync to 60fps
    Uint64 frame_duration_ms = SDL_GetTicks() - frame_start_time;

    // 4. If the frame finished early, delay until the target time is hit
    if (frame_duration_ms < FRAME_TARGET_MS) {
      SDL_Delay(FRAME_TARGET_MS - frame_duration_ms);
    }
  }

  engine.cleanup();
  SDL_Quit();
  return true;
}
