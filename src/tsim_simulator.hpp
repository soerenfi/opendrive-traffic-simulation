#ifndef __TSIM_SIMULATOR_HPP__
#define __TSIM_SIMULATOR_HPP__

#include <thread>
#include <utility>
#include <vector>

#include "osi_publisher.hpp"
#include "renderer_sfml.hpp"

namespace tsim {
class Map;
class TrafficObject;
class Simulator {
public:
  explicit Simulator(std::shared_ptr<Map> map)
      : m_map(std::move(map)), m_renderer(this), m_osiPublisher(this){};
  ~Simulator();
  Simulator(const Simulator &other) = delete;
  Simulator(Simulator &&other) = delete;
  Simulator operator=(const Simulator &other) = delete;
  Simulator operator=(Simulator &&other) = delete;

  void run();
  void addVehicle();
  void addThread(std::thread &&thread);

  std::vector<std::shared_ptr<TrafficObject>> getObjects() {
    return m_objects;
  };
  std::shared_ptr<Map> getMap() { return m_map; };

private:
  std::shared_ptr<Map> m_map;
  Renderer m_renderer;
  OsiPublisher m_osiPublisher;

  std::vector<std::shared_ptr<TrafficObject>> m_objects;
  std::vector<std::thread> m_threads;
};
} // namespace tsim
#endif // __TSIM_SIMULATOR_HPP__
