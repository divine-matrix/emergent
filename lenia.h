
#ifndef RAPTOR_ENGINE_LENIA_AUTOMATON_HPP
#define RAPTOR_ENGINE_LENIA_AUTOMATON_HPP

#include <algorithm>
#include <cmath>
#include <vector>

namespace raptor {

/**
 * @brief Configuration parameters for a specific Lenia creature species.
 * Default values target the famous moving "Orbium" creature.
 */
struct LeniaConfig {
  int radius = 12;      // Neighborhood perception radius
  float dt = 0.1f;      // Time step increment
  float mu = 0.15f;     // Growth function mean (ideal density)
  float sigma = 0.015f; // Growth function standard deviation (tolerance)
};

/**
 * @brief Lenia Continuous Cellular Automaton simulation component.
 * Designed to integrate into the Raptor / Rapier engine pipeline alongside
 * discrete systems like Brian's Brain and Langton's Ant.
 */
class LeniaAutomaton {
public:
  LeniaAutomaton(int width, int height,
                 const LeniaConfig &config = LeniaConfig{})
      : m_width(width), m_height(height), m_config(config) {
    m_grid.resize(m_width * m_height, 0.0f);
    m_next_grid.resize(m_width * m_height, 0.0f);
    precompute_kernel();
  }

  ~LeniaAutomaton() = default;

  /**
   * @brief Computes the next time step of the Lenia simulation.
   * Performs a 2D toroidal spatial convolution across the continuous state
   * space.
   */
  void step() {
    const int k_dim = 2 * m_config.radius + 1;

    // Loop over every cell in the world grid layout
    for (int y = 0; y < m_height; ++y) {
      for (int x = 0; x < m_width; ++x) {

        float neighborhood_density = 0.0f;

        // Convolve using the pre-computed circular ring kernel
        for (int ky = -m_config.radius; ky <= m_config.radius; ++ky) {
          // Toroidal space wrapping logic
          int wrapped_y = (y + ky + m_height) % m_height;
          int k_row_offset = (ky + m_config.radius) * k_dim;
          int g_row_offset = wrapped_y * m_width;

          for (int kx = -m_config.radius; kx <= m_config.radius; ++kx) {
            int wrapped_x = (x + kx + m_width) % m_width;

            float cell_value = m_grid[g_row_offset + wrapped_x];
            float weight = m_kernel[k_row_offset + (kx + m_config.radius)];

            neighborhood_density += cell_value * weight;
          }
        }

        // Growth function: Smooth Gaussian bell curve mapped to range
        // [-1.0, 1.0]
        float diff = neighborhood_density - m_config.mu;
        float exponent =
            -(diff * diff) / (2.0f * m_config.sigma * m_config.sigma);
        float growth = 2.0f * std::exp(exponent) - 1.0f;

        // Update cell state using differential scaling, clamped strictly
        // between 0 and 1
        int current_idx = y * m_width + x;
        float next_val = m_grid[current_idx] + m_config.dt * growth;
        m_next_grid[current_idx] = std::clamp(next_val, 0.0f, 1.0f);
      }
    }

    // Fast pointer swap instead of copying buffer arrays
    m_grid.swap(m_next_grid);
  }

  /**
   * @brief Clears the entire world grid back to empty states.
   */
  void clear() {
    std::fill(m_grid.begin(), m_grid.end(), 0.0f);
    std::fill(m_next_grid.begin(), m_next_grid.end(), 0.0f);
  }

  /**
   * @brief Spawns an initial Orbium creature seed in the center of the grid.
   */
  void seed_orbium() {
    clear();
    int cx = m_width / 2;
    int cy = m_height / 2;
    int seed_radius = 7;

    for (int sy = -seed_radius; sy <= seed_radius; ++sy) {
      for (int sx = -seed_radius; sx <= seed_radius; ++sx) {
        if (std::sqrt(sx * sx + sy * sy) <= seed_radius) {
          int target_x = (cx + sx + m_width) % m_width;
          int target_y = (cy + sy + m_height) % m_height;
          m_grid[target_y * m_width + target_x] = 0.55f;
        }
      }
    }
  }

  // --- Accessors for rendering mapping pipelines ---
  const std::vector<float> &get_grid() const { return m_grid; }
  int get_width() const { return m_width; }
  int get_height() const { return m_height; }

private:
  /**
   * @brief Generates a smooth, normalized, donut-shaped 2D polynomial kernel.
   */
  void precompute_kernel() {
    const int k_dim = 2 * m_config.radius + 1;
    m_kernel.resize(k_dim * k_dim, 0.0f);
    float total_weight = 0.0f;

    for (int ky = -m_config.radius; ky <= m_config.radius; ++ky) {
      for (int kx = -m_config.radius; kx <= m_config.radius; ++kx) {
        // Compute normalized radial distance from center
        float dist = std::sqrt(kx * kx + ky * ky) / m_config.radius;

        if (dist > 0.0f && dist <= 1.0f) {
          // Standard Lenia polynomial distribution bump
          float window = 4.0f * dist * (1.0f - dist);
          float weight = std::pow(window, 4.0f);

          m_kernel[(ky + m_config.radius) * k_dim + (kx + m_config.radius)] =
              weight;
          total_weight += weight;
        }
      }
    }

    // Normalize weights so they integrate up to exactly 1.0
    if (total_weight > 0.0f) {
      for (auto &weight : m_kernel) {
        weight /= total_weight;
      }
    }
  }

  int m_width;
  int m_height;
  LeniaConfig m_config;

  std::vector<float> m_grid;
  std::vector<float> m_next_grid;
  std::vector<float> m_kernel;
};

} // namespace raptor

#endif // RAPTOR_ENGINE_LENIA_AUTOMATON_HPP
