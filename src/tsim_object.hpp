#ifndef __TSIM_OBJECT_HPP__
#define __TSIM_OBJECT_HPP__

#include <glm/glm.hpp>

#include <algorithm>
#include <cstdint>
#include <thread>
#include <vector>

#include "tsim_map.hpp"
#include "tsim_simulator.hpp"
#include "tsim_util.hpp"

namespace tsim {

class TrafficObject {
public:
  TrafficObject(std::shared_ptr<Map> map, Simulator* sim, int id);
  virtual ~TrafficObject() = default;
  TrafficObject(const TrafficObject& other) = delete;  // TODO(soeren): implement
  TrafficObject(TrafficObject&& other) = delete;
  TrafficObject operator=(const TrafficObject& other) = delete;
  TrafficObject operator=(TrafficObject&& other) = delete;

  virtual void simulate(){};

  const glm::vec3& getPosition() const {
    return m_position;
  };

protected:
  std::shared_ptr<Map> m_map;
  Simulator* m_simulator;

  glm::vec3 m_position{0.0f, 0.0f, 0.0f};
  glm::vec3 m_dimension{4.5f, 2.0f, 1.8f};
  glm::vec3 m_orientation{0.0f, 0.0f, 0.0f};

  uint16_t m_id;
};

class Vehicle : public TrafficObject {
public:
  Vehicle(std::shared_ptr<Map> map, Simulator* sim, int id);

  void simulate() override;

private:
  void drive();

  std::shared_ptr<Road> m_currentRoad;
  std::shared_ptr<Lane> m_currentLane;
};

}  // namespace tsim

#endif  // __TSIM_OBJECT_HPP__
