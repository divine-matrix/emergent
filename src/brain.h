#include "base.h"
#include "imgui.h"
#include "simulation.h"
#include <algorithm>
#include <cstdint>
#include <random>
#include <utility>

class Brain : public Simulation {
public:
  // Raw format is 0xAABBGGRR: black, then dark red up to full red
  static constexpr uint32_t stateColors[5] = {
      0xFF000000, 0xFF000040, 0xFF000080, 0xFF0000BF, 0xFF0000FF};
  static constexpr int SIM_W = 100, SIM_H = 75;

  uint8_t *board = new uint8_t[SIM_W * SIM_H]();
  uint8_t *boardBuffer = new uint8_t[SIM_W * SIM_H]();
  uint32_t seed = 1;

  Brain(GlobalContext &ctx) : Simulation(ctx, SIM_W, SIM_H) { reset(); }
  ~Brain() {
    delete[] board;
    delete[] boardBuffer;
  }
  Brain(const Brain &) = delete;
  Brain &operator=(const Brain &) = delete;

  int getAliveSurroundingSquares(int x, int y) const {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
      const int ny = (y + dy + SIM_H) % SIM_H;
      const uint8_t *row = board + ny * SIM_W;
      for (int dx = -1; dx <= 1; dx++) {
        if (dx == 0 && dy == 0)
          continue;
        count += (row[(x + dx + SIM_W) % SIM_W] == 4);
      }
    }
    return count;
  }

  void update() override {
    for (int y = 0; y < SIM_H; y++) {
      for (int x = 0; x < SIM_W; x++) {
        const int i = y * SIM_W + x;
        const uint8_t c = board[i];
        uint8_t next;
        if (c == 0)
          next = (getAliveSurroundingSquares(x, y) == 2) ? 4 : 0; // birth
        else
          next = c - 1; // alive(4) -> 3 -> 2 -> 1 -> dead(0)
        boardBuffer[i] = next;
        tex.m_tex[i] = stateColors[next];
      }
    }
    std::swap(board, boardBuffer); // the new generation becomes the current one
  }

  void drawUI() override { ImGui::Text("Brian's Brain"); }

  void reset() override {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 4);
    for (int i = 0; i < SIM_W * SIM_H; i++) {
      board[i] = (uint8_t)dist(rng);
      tex.m_tex[i] = stateColors[board[i]];
    }
  }
};
