#include "SDL3/SDL_events.h"
#include "SDL3/SDL_stdinc.h"
#include "engine/render.h"
#include "langtonant.h"
#include <iostream>

const int TARGET_FPS = 60;
const Uint64 FRAME_TARGET_MS =
    1000 / TARGET_FPS; // ~16.666 milliseconds per frame

class EmergentGarden : public App {
public:
  void init() override {};
  void update() override {};
  void drawUI() override {};
  void on_event(SDL_Event event, bool *running) override {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      *running = false;
      break;
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
