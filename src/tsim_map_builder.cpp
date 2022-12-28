#include "tsim_map_builder.hpp"

#include <memory>

#include "tsim_map.hpp"

namespace tsim {

std::shared_ptr<Road> MapBuilder::addRoad(int id, int junction) {
  std::shared_ptr<tsim::Road> road = std::make_shared<tsim::Road>(m_map);
  road->m_id = id;
  road->m_junction = junction;
  if (junction != -1)
    road->m_roadType = RoadType::eJUNCTION;
  else
    road->m_roadType = RoadType::eROAD;
  m_map->m_roads.push_back(road);
  return road;
}
std::shared_ptr<Road> MapBuilder::getRoad(int id) {
  auto it =
    std::find_if(m_map->m_roads.begin(), m_map->m_roads.end(), [id](std::shared_ptr<Road> road) {
      return road->id() == id;
    });
  if (it != m_map->m_roads.end()) return (*it);
  return nullptr;
}

std::shared_ptr<Junction> MapBuilder::getJunction(int id) {
  auto it = std::find_if(m_map->m_junctions.begin(), m_map->m_junctions.end(),
                         [id](std::shared_ptr<Junction> junction) {
                           return junction->id() == id;
                         });
  if (it != m_map->m_junctions.end()) return (*it);
  return nullptr;
}

std::vector<std::shared_ptr<Road>> MapBuilder::junction_findConnectingRoads(Junction* junction,
                                                                            Road* road) {
  std::vector<std::shared_ptr<JunctionConnection>> connectingRoads;
  std::copy_if(junction->m_connections.begin(), junction->m_connections.end(),
               std::back_inserter(connectingRoads),
               [road](const std::shared_ptr<JunctionConnection>& connection) {
                 return connection->m_incomingRoad == road->id();
               });

  std::vector<std::shared_ptr<Road>> ret;
  std::transform(connectingRoads.cbegin(), connectingRoads.cend(), std::back_inserter(ret),
                 [&](const std::shared_ptr<JunctionConnection>& elem) {
                   return m_map->findRoadById(elem->m_connectingRoad);
                 });
  return ret;
}

void MapBuilder::road_addPredecessor(Road* road, std::shared_ptr<Road> predecessor) {
  road->m_predecessors.push_back(predecessor);
};
void MapBuilder::road_addSuccessor(Road* road, std::shared_ptr<Road> successor) {
  road->m_successors.push_back(successor);
};
void MapBuilder::road_addRoadPoints(Road* road, std::vector<Point> points) {
  road->m_roadPoints.insert(road->m_roadPoints.end(), points.begin(), points.end());
}
std::shared_ptr<LaneSection> MapBuilder::road_addLaneSection(std::shared_ptr<Road> road,
                                                             double s_offset) {
  std::shared_ptr<LaneSection> lane_section = std::make_shared<LaneSection>(road);
  lane_section->m_sOffset = s_offset;
  road->m_sections.push_back(lane_section);
  return lane_section;
}
std::shared_ptr<Lane> MapBuilder::laneSection_addLane(std::shared_ptr<LaneSection> lane_section,
                                                      uint32_t id, double offset, double width,
                                                      LaneType type) {
  std::shared_ptr<Lane> lane = std::make_shared<Lane>(lane_section);
  lane->m_id = id;
  lane->m_offset = offset;
  lane->m_width = width;
  lane->m_type = type;
  lane_section->m_lanes.push_back(lane);
  return lane;
}
void MapBuilder::laneSection_addPredecessor(LaneSection* lane_section,
                                            std::shared_ptr<LaneSection> predecessor) {
  lane_section->m_predecessors.push_back(predecessor);
}
void MapBuilder::laneSection_addSuccessor(LaneSection* lane_section,
                                          std::shared_ptr<LaneSection> successor) {
  lane_section->m_successors.push_back(successor);
}
void MapBuilder::lane_addLanePoints(Lane* lane, std::vector<Point> points) {
  lane->m_lanePoints.insert(lane->m_lanePoints.end(), points.begin(), points.end());
}
void MapBuilder::lane_addLaneBoundaryPoints(Lane* lane, std::vector<Point> points) {
  lane->m_laneBoundaryPoints.insert(lane->m_laneBoundaryPoints.end(), points.begin(), points.end());
}
void MapBuilder::lane_addPredecessor(Lane* lane, std::shared_ptr<Lane> predecessor) {
  lane->m_predecessors.push_back(predecessor);
}
void MapBuilder::lane_addSuccessor(Lane* lane, std::shared_ptr<Lane> successor) {
  lane->m_successors.push_back(successor);
}

std::shared_ptr<Junction> MapBuilder::addJunction(int id) {
  std::shared_ptr<Junction> junction = std::make_shared<Junction>(m_map);
  junction->m_id = id;
  m_map->m_junctions.push_back(junction);
  return junction;
}
std::shared_ptr<JunctionConnection> MapBuilder::junction_addConnection(Junction* junction,
                                                                       uint32_t incoming_road,
                                                                       uint32_t connecting_road) {
  std::shared_ptr<JunctionConnection> connection = std::make_shared<JunctionConnection>();
  connection->m_incomingRoad = incoming_road;
  connection->m_connectingRoad = connecting_road;
  junction->m_connections.push_back(connection);
  return connection;
}

void MapBuilder::connection_addLaneLink(JunctionConnection* connection, int from, int to) {
  LaneLink lane_link;
  lane_link.from = from;
  lane_link.to = to;
  connection->m_laneLinks.push_back(lane_link);
}

std::shared_ptr<Map> MapBuilder::getMap() {
  return std::move(m_map);
}

}  // namespace tsim