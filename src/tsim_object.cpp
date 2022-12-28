#include "tsim_object.hpp"

#include <iostream>
#include <thread>
#include <utility>

#include "tsim_map.hpp"

namespace tsim {
using std::shared_ptr;

TrafficObject::TrafficObject(shared_ptr<Map> map, Simulator* sim, int id)
    : m_map(std::move(map))
    , m_simulator(sim)
    , m_id(id) {}

Vehicle::Vehicle(std::shared_ptr<Map> map, Simulator* sim, int id)
    : TrafficObject(std::move(map), sim, id) {
  m_currentRoad = m_map->getRandomRoad();
  m_currentLane = m_currentRoad->sections().at(0)->lane(-1);
  m_position = m_currentLane->startPoint();
}

void Vehicle::simulate() {
  m_simulator->addThread(std::thread(&Vehicle::drive, this));
};

void Vehicle::drive() {
  auto laneSize = m_currentLane->points().size() - 1;
  auto step = std::chrono::milliseconds(20);
  int laneStep = 0;
  int stepsTaken{0};
  while (true) {
    auto now = std::chrono::system_clock::now();
    auto target = now + step;
    m_position = m_currentLane->points().at(laneStep);

    if (stepsTaken < laneSize) {
      if (m_currentLane->id() < 0) {
        laneStep += 1;
      } else {
        laneStep -= 1;
      }
      stepsTaken++;
    }

    else {
      // end of road reached
      std::vector<std::shared_ptr<Lane>> nextLanes;
      if (m_currentLane->id() < 0) {
        nextLanes = m_currentLane->successors();
      } else {
        nextLanes = m_currentLane->predecessors();
      }
      // pick random road to continue driving
      auto randIt = nextLanes.begin();
      std::cout << nextLanes.size() << " currlaneid " << m_currentLane->id() << " currRoadID " << m_currentRoad->id()
                << std::endl;
      std::advance(randIt, std::rand() % nextLanes.size());
      m_currentLane = (*randIt);

      laneSize = m_currentLane->points().size() - 1;

      // jump to start/end of new road depending on driving direction
      if (m_currentLane->id() < 0) {
        laneStep = 0;
      } else {
        laneStep = m_currentLane->points().size() - 1;
      }
      stepsTaken = 0;
    }
    std::this_thread::sleep_until(target);
  }
}
}  // namespace tsim