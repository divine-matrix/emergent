#pragma once
#include "imgui.h"
#include "simulation.h"
#include <algorithm>
#include <cstdint>
#include <cstring>

class LangtonBoard {
public:
  // Raw format is 0xAABBGGRR: opaque black, opaque white
  static constexpr uint32_t stateColors[2] = {0xFF000000, 0xFFFFFFFF};

  Vector2I dimensions;
  uint8_t *board;
  ProceduralTextureRender &tex; // owned by the Simulation

  LangtonBoard(ProceduralTextureRender &t, int width, int height)
      : dimensions(width, height), board(new uint8_t[width * height]()),
        tex(t) {
    clear();
  }
  ~LangtonBoard() { delete[] board; }

  LangtonBoard(const LangtonBoard &) = delete;
  LangtonBoard &operator=(const LangtonBoard &) = delete;

  inline int idx(const Vector2I &pos) const {
    return pos.getAsInt(dimensions.x);
  }
  uint8_t getSquare(const Vector2I &pos) const { return board[idx(pos)]; }

  void setSquare(const Vector2I &pos, uint8_t state) {
    int i = idx(pos);
    board[i] = state;
    tex.m_tex[i] = stateColors[state];
  }

  void clear() {
    const int n = dimensions.x * dimensions.y;
    std::memset(board, 0, n);
    std::fill(tex.m_tex, tex.m_tex + n, stateColors[0]);
  }
};

class Ant {
public:
  Vector2I position;
  int rotation; // 0 = north, 1 = east, 2 = south, 3 = west
  LangtonBoard *board;

  Ant(LangtonBoard *b)
      : position(b->dimensions.x / 2, b->dimensions.y / 2), rotation(0),
        board(b) {}

  static constexpr int DX[4] = {0, 1, 0, -1};
  static constexpr int DY[4] = {-1, 0, 1, 0};

  void reset() {
    position = Vector2I(board->dimensions.x / 2, board->dimensions.y / 2);
    rotation = 0;
  }

  inline void stepForward(Vector2I &pos, int rot) {
    pos.x += DX[rot & 3];
    pos.y += DY[rot & 3];
  }

  void evaluate() {
    uint8_t cell = board->getSquare(position);

    if (cell == 0)
      rotation = (rotation + 1) % 4; // black: turn right
    else
      rotation = (rotation + 3) % 4; // white: turn left

    board->setSquare(position, cell ^ 1);
    stepForward(position, rotation);

    position.x = (position.x + board->dimensions.x) % board->dimensions.x;
    position.y = (position.y + board->dimensions.y) % board->dimensions.y;
  }
};

class LangtonSimulation : public Simulation {
public:
  // Declared after the Simulation base, so `tex` already exists here
  LangtonBoard board;
  Ant ant;

  float stepsPerTick = 30.0f;
  float stepAccumulator = 0.0f;
  int iterationCounter = 0;

  LangtonSimulation(GlobalContext &ctx)
      : Simulation(ctx, WINDOW_W, WINDOW_H), board(tex, WINDOW_W, WINDOW_H),
        ant(&board) {}

  void update() override {
    stepAccumulator += stepsPerTick;
    int steps = (int)stepAccumulator;
    stepAccumulator -= steps;

    for (int i = 0; i < steps; i++) {
      ant.evaluate();
      iterationCounter++;
    }

    // Mark the ant's current position in red
    tex.m_tex[board.idx(ant.position)] = Color(255, 0, 0, 255).getRawColor();
  }

  void drawUI() override {
    ImGui::SliderFloat("Steps per tick", &stepsPerTick, 0.03f, 2000.0f, "%.2f",
                       ImGuiSliderFlags_Logarithmic |
                           ImGuiSliderFlags_AlwaysClamp);
    ImGui::Text("Iteration counter: %d", iterationCounter);
  }

  void reset() override {
    board.clear();
    ant.reset();
    stepAccumulator = 0.0f;
    iterationCounter = 0;
  }
};
