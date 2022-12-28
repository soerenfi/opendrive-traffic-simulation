#include "tsim_simulator.hpp"
//
#include "tsim_object.hpp"
namespace tsim {

Simulator::~Simulator() {
  std::for_each(m_threads.begin(), m_threads.end(), [](std::thread& t) {
    t.join();
  });
}
void Simulator::run() {
  for (auto const& obj : m_objects) {
    obj->simulate();
  }
  m_osiPublisher.start();
  m_renderer.render();
}
void Simulator::addThread(std::thread&& thread) {
  m_threads.emplace_back(std::move(thread));
}
void Simulator::addVehicle() {
  m_objects.emplace_back(std::make_shared<Vehicle>(m_map, this, m_objects.size()));
}
}  // namespace tsim