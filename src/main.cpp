#include "SDL3/SDL_events.h"
#include "base.h"
#include "engine/render.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL_mouse.h>
#include <memory>

#include "langton.h"
// #include "gameoflife.h"   // future simulations go here

// ---- Simulation registry: one line per simulation ----
template <class T> std::unique_ptr<Simulation> make(GlobalContext &ctx) {
  return std::make_unique<T>(ctx);
}

struct SimEntry {
  const char *name;
  std::unique_ptr<Simulation> (*create)(GlobalContext &);
};

static const SimEntry kSims[] = {
    {"Langton's Ant", make<LangtonSimulation>},
    // {"Game of Life", make<GameOfLifeSimulation>},
};
static constexpr int kSimCount = sizeof(kSims) / sizeof(kSims[0]);

class EmergentGarden : public App {
public:
  std::unique_ptr<Simulation> sim;
  int selected = 0;
  int pending = -1; // simulation to switch to on the next update
  bool paused = false;

  void init() override { sim = kSims[selected].create(ctx); }

  void update() override {
    // Switch here, not in drawUI, so we never destroy a draw object mid-frame
    if (pending >= 0) {
      sim.reset(); // old texture unregisters itself from ctx.drawCalls
      selected = pending;
      sim = kSims[selected].create(ctx);
      pending = -1;
    }

    if (!paused && sim)
      sim->update();
  }

  void drawUI() override {
    ImGui::Begin("Diagnostics", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Separator();

    if (ImGui::BeginCombo("Simulation", kSims[selected].name)) {
      for (int i = 0; i < kSimCount; i++) {
        bool isSelected = (i == selected);
        if (ImGui::Selectable(kSims[i].name, isSelected) && !isSelected)
          pending = i;
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    if (ImGui::Button("Restart") && sim) {
      sim->reset();
      sim->view.reset();
      sim->view.apply(sim->tex.region);
    }
    ImGui::SameLine();
    if (ImGui::Button(paused ? "Resume" : "Pause"))
      paused = !paused;
    if (paused)
      ImGui::TextColored(ImVec4(1, 0, 0, 1), "Simulation paused");

    ImGui::Separator();
    if (sim)
      sim->drawUI();
    ImGui::End();
  }

  void on_event(SDL_Event event, bool *running) override {
    ImGui_ImplSDL3_ProcessEvent(&event);
    bool uiWantsMouse = ImGui::GetIO().WantCaptureMouse;

    switch (event.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
      *running = false;
      break;

    case SDL_EVENT_MOUSE_WHEEL:
      if (uiWantsMouse || !sim)
        break;
      sim->view.zoomAt(event.wheel.mouse_x, event.wheel.mouse_y, event.wheel.y);
      sim->view.apply(sim->tex.region);
      break;

    case SDL_EVENT_MOUSE_MOTION:
      if (uiWantsMouse || !sim)
        break;
      if (event.motion.state & SDL_BUTTON_LMASK) {
        sim->view.drag(event.motion.xrel, event.motion.yrel);
        sim->view.apply(sim->tex.region);
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
