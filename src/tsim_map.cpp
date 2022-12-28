
#include "tsim_map.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
namespace tsim {

std::shared_ptr<Lane> LaneSection::lane(int lid) const {
  auto iterator = std::find_if(m_lanes.begin(), m_lanes.end(), [lid](const std::shared_ptr<Lane>& lane) {
    return lane->id() == lid;
  });
  if (iterator == m_lanes.end()) {
    throw std::logic_error("lane " + std::to_string(lid) + "not found in Road " + std::to_string(this->m_road->id()));
  }
  return *iterator;
}
std::shared_ptr<Lane> Road::getFirstLane() {
  auto laneSection = m_sections.front();
  auto lanes = laneSection->lanes();
  return lanes.front();
};

std::shared_ptr<Road> Map::findRoadById(int rid) {
  auto iterator = std::find_if(m_roads.begin(), m_roads.end(), [rid](const std::shared_ptr<Road>& road) {
    return road->id() == rid;
  });
  if (iterator != m_roads.end()) {
    return (*iterator);
  }
  return nullptr;
}
std::shared_ptr<Junction> Map::findJunctionById(int jid) {
  auto iterator =
    std::find_if(m_junctions.begin(), m_junctions.end(), [jid](const std::shared_ptr<Junction>& junction) {
      return junction->id() == jid;
    });
  return (iterator != m_junctions.end() ? *iterator : nullptr);
  // if (it != m_junctions.end()) {
  //   return (*it);
  // }
  // return nullptr;
}

double LaneSection::sOffset() const {
  return m_sOffset;
}

Road::Road(std::shared_ptr<Map> map)
    : m_map(std::move(map))
    , m_roadType(RoadType::eROAD){};

std::shared_ptr<Road> Map::getRandomRoad() {
  auto road = m_roads.at(std::rand() % m_roads.size());
  auto laneSection = road->sections().front();  // TODO(soeren): multiple lane sections
  auto lanes = laneSection->lanes();
  auto iterator = std::find_if(lanes.begin(), lanes.end(), [](const std::shared_ptr<Lane>& lane) {
    return lane->id() == -1;
  });
  return (iterator != lanes.end() ? road : nullptr);
  // if (iterator == lanes.end()) {
  //   return getRandomRoad();
  // }
  // return road;
}
}  // namespace tsim