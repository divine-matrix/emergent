#pragma once
#include "base.h"
#include "imgui.h"
#include "simulation.h"
#include <cstdint>
#include <random>
#include <utility>

class Brain : public Simulation {
public:
  // Raw format 0xAABBGGRR. Index 0 = dead ... 4 = alive (warm white)
  static constexpr uint32_t stateColors[5] = {
      0xFF1A0D0A, // dead: near-black blue
      0xFF3C0A5A, // deep purple
      0xFF2828C8, // red
      0xFF1E96FF, // orange
      0xFFC8F0FF, // alive: warm white
  };
  static constexpr int SIM_W = 100, SIM_H = 75; // each cell = 8x8 pixels

  uint8_t *board = new uint8_t[SIM_W * SIM_H]();
  uint8_t *boardBuffer = new uint8_t[SIM_W * SIM_H]();

  std::mt19937 rng;
  uint32_t seed = 1;
  float density = 0.2f;       // fraction of cells that start alive
  float stepsPerTick = 0.25f; // 0.25 * 60 Hz = 15 generations/sec
  float stepAccumulator = 0.0f;
  int dyingStages = 1;   // 1 = classic Brian's Brain
  float noise = 0.0005f; // fraction of cells sparked alive per generation

  Brain(GlobalContext &ctx) : Simulation(ctx, SIM_W, SIM_H) { reset(); }
  ~Brain() {
    delete[] board;
    delete[] boardBuffer;
  }
  Brain(const Brain &) = delete;
  Brain &operator=(const Brain &) = delete;

  // The "alive" state value: dying stages are alive-1 ... 1, dead is 0
  int alive() const { return dyingStages + 1; }

  // Map any state onto the 5-color palette
  uint32_t colorOf(uint8_t s) const {
    return stateColors[s == 0 ? 0 : (s * 4) / alive()];
  }

  int getAliveSurroundingSquares(int x, int y) const {
    const int a = alive();
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
      const int ny = (y + dy + SIM_H) % SIM_H;
      const uint8_t *row = board + ny * SIM_W;
      for (int dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dy == 0)
          continue;
        count += (row[(x + dx + SIM_W) % SIM_W] == a);
      }
    }
    return count;
  }

  void step() {
    const uint8_t a = (uint8_t)alive();
    for (int y = 0; y < SIM_H; y++) {
      for (int x = 0; x < SIM_W; x++) {
        const int i = y * SIM_W + x;
        const uint8_t c = board[i];
        boardBuffer[i] = (c == 0)
                             ? (getAliveSurroundingSquares(x, y) == 2 ? a : 0)
                             : (uint8_t)(c - 1); // alive -> ... -> 1 -> dead
      }
    }
    std::swap(board, boardBuffer);

    // Spark a few random dead cells alive so the pattern never fully settles
    std::uniform_int_distribution<int> cell(0, SIM_W * SIM_H - 1);
    const int sparks = (int)(noise * SIM_W * SIM_H + 0.5f);
    for (int s = 0; s < sparks; s++) {
      int i = cell(rng);
      if (board[i] == 0)
        board[i] = a;
    }
  }

  void update() override {
    stepAccumulator += stepsPerTick;
    int steps = (int)stepAccumulator;
    stepAccumulator -= steps;
    if (steps == 0)
      return;

    for (int s = 0; s < steps; s++)
      step();

    // Only repaint the texture once per tick
    for (int i = 0; i < SIM_W * SIM_H; i++)
      tex.m_tex[i] = colorOf(board[i]);
  }

  void drawUI() override {
    ImGui::SliderFloat("Steps per tick", &stepsPerTick, 0.05f, 4.0f, "%.2f",
                       ImGuiSliderFlags_Logarithmic |
                           ImGuiSliderFlags_AlwaysClamp);
    if (ImGui::SliderInt("Dying stages", &dyingStages, 1, 3))
      reset(); // state values change meaning, so restart
    ImGui::SliderFloat("Noise", &noise, 0.0f, 0.005f, "%.4f");
    ImGui::SliderFloat("Start density", &density, 0.02f, 0.5f, "%.2f");
    ImGui::InputScalar("Seed", ImGuiDataType_U32, &seed);
    if (ImGui::Button("New random seed")) {
      seed = std::random_device{}();
      reset();
    }
  }

  void reset() override {
    rng.seed(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (int i = 0; i < SIM_W * SIM_H; i++) {
      board[i] = (dist(rng) < density) ? (uint8_t)alive() : 0;
      tex.m_tex[i] = colorOf(board[i]);
    }
    stepAccumulator = 0.0f;
  }
};
