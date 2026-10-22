#pragma once
#include "SDL3/SDL_render.h"
#include "base.h"
#include <SDL3/SDL.h>
#include <cstdint>

class App;

class Engine {
public:
  Engine(GlobalContext &context) : ctx(context) {};
  int width, height;
  bool renderInit(const char *title, int width, int height);
  bool _handle_event(App *app);
  void draw();
  void cleanup();
  bool renderFrame(App *app);

  // Texture access helper functions

  SDL_Texture *createTexture(int w, int h, SDL_TextureAccess accessMode);
  void updateTexture(SDL_Texture *tex, void *pixels, int w);
  void renderTexture(SDL_Texture *tex, TextureRegion region);

private:
  GlobalContext &ctx;
  SDL_Window *m_window = nullptr;
  SDL_Renderer *m_renderer = nullptr;
};

class App {
public:
  GlobalContext ctx;  // App owns the master global context
  Engine engine{ctx}; // Engine gets initialized with App's context

  int TARGET_FPS = 60;
  uint64_t FRAME_TARGET_MS =
      1000 / TARGET_FPS; // ~16.666 milliseconds per frame

  bool run();
  virtual ~App() {};
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void drawUI() = 0;
  virtual void on_event(SDL_Event event, bool *running) = 0;
};
