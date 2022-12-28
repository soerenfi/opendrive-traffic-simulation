#ifndef __RENDERER_SFML_HPP__
#define __RENDERER_SFML_HPP__

#include <vector>

#include "tsim_map.hpp"
//
#include <SFML/Graphics.hpp>

constexpr std::size_t SCREEN_WIDTH{1600};
constexpr std::size_t SCREEN_HEIGHT{900};

namespace tsim {
class Simulator;
class TrafficObject;
}  // namespace tsim

class Renderer {
public:
  explicit Renderer(tsim::Simulator* sim);
  ~Renderer() = default;

  void render();

  void drawLanes();
  void drawVehicles() const;

  void findMaxMinValues();

private:
  std::unique_ptr<sf::RenderWindow> m_window;

  tsim::Simulator* m_simulator;
  std::shared_ptr<tsim::Map> m_map;
  std::vector<std::shared_ptr<tsim::TrafficObject>> m_objects;

  double m_minX{0};
  double m_maxX{0};
  double m_minY{0};
  double m_maxY{0};

  double m_scale;
  int m_frameCount = 0;
  std::size_t m_targetFrameDuration{1000 / 60};
  int m_close{0};
  sf::Color m_backgroundColor{74, 83, 97};
  const std::size_t m_screenWidth{SCREEN_WIDTH};
  const std::size_t m_screenHeight{SCREEN_HEIGHT};
};

#endif  // __RENDERER_SFML_HPP__