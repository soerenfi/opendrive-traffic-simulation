#ifndef __TSIM_MAP_HPP__
#define __TSIM_MAP_HPP__

#include <glm/glm.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "tsim_util.hpp"

namespace tsim {

enum class LaneGroup { eLEFT, eRIGHT, eCENTER };
enum class RoadType { eROAD, eJUNCTION };
enum class LaneType { eSIDEWALK, eSHOULDER, eDRIVING, eRESTRICTED, eMEDIAN, ePARKING, eNONE };

class Map;
class Road;
class LaneSection;

class Lane {
public:
  explicit Lane(std::shared_ptr<LaneSection> lane_section)
      : m_laneSection(std::move(lane_section)){};

  const glm::vec3& startPoint() {
    return m_lanePoints.front();
  }
  double width() const {
    return m_width;
  }
  double id() const {
    return m_id;
  }
  LaneType laneType() {
    return m_type;
  }

  const std::vector<glm::vec3>& points() const {
    return m_lanePoints;
  }
  const std::vector<Point>& boundaryPoints() const {
    return m_laneBoundaryPoints;
  }

  std::vector<std::shared_ptr<Lane>> predecessors() {
    return m_successors;
  }
  std::vector<std::shared_ptr<Lane>> successors() {
    return m_predecessors;
  }

private:
  std::shared_ptr<LaneSection> m_laneSection;
  std::vector<glm::vec3> m_lanePoints;
  std::vector<glm::vec3> m_laneBoundaryPoints;

  std::vector<std::shared_ptr<Lane>> m_successors;
  std::vector<std::shared_ptr<Lane>> m_predecessors;

  int32_t m_id{0};
  double m_offset{0.0f};
  double m_width{0.0f};
  LaneType m_type{};

  friend class MapBuilder;
};

class LaneSection {
public:
  explicit LaneSection(std::shared_ptr<Road> road)
      : m_road(std::move(road)){};

  std::vector<std::shared_ptr<Lane>> lanes() const {
    return m_lanes;
  }
  double sOffset() const;
  std::shared_ptr<Lane> lane(int lid) const;
  std::shared_ptr<Road> road() {
    return m_road;
  };
  std::vector<std::shared_ptr<LaneSection>> predecessors() {
    return m_predecessors;
  }
  std::vector<std::shared_ptr<LaneSection>> successors() {
    return m_successors;
  }

private:
  std::shared_ptr<Road> m_road;
  std::vector<std::shared_ptr<Lane>> m_lanes;

  std::vector<std::shared_ptr<LaneSection>> m_successors;
  std::vector<std::shared_ptr<LaneSection>> m_predecessors;

  double m_sOffset{0.0f};

  friend class MapBuilder;
};

struct LaneLink {
  int from{0};
  int to{0};
};

class JunctionConnection {
public:
  uint32_t incomingRoad() const {
    return m_incomingRoad;
  }
  uint32_t connectingRoad() const {
    return m_connectingRoad;
  }
  const std::vector<LaneLink>& getLaneLinks() const {
    return m_laneLinks;
  }

private:
  std::vector<LaneLink> m_laneLinks;

  uint16_t m_id{0};
  uint32_t m_incomingRoad{0};
  uint32_t m_connectingRoad{0};

  friend class MapBuilder;
};
class Junction {
public:
  explicit Junction(std::shared_ptr<Map> map)
      : m_map(std::move(map)){};

  uint32_t id() const {
    return m_id;
  };
  std::vector<std::shared_ptr<JunctionConnection>> connections() {
    return m_connections;
  };

private:
  std::shared_ptr<Map> m_map;
  std::vector<std::shared_ptr<JunctionConnection>> m_connections;
  uint32_t m_id{0};
  friend class MapBuilder;
};

class Road {
public:
  explicit Road(std::shared_ptr<Map> map);

  uint32_t id() const {
    return m_id;
  };
  const glm::vec3& startPoint() {
    return m_roadPoints.front();
  };
  std::vector<std::shared_ptr<Road>> successors() {
    return m_successors;
  };
  std::vector<std::shared_ptr<Road>> predecessors() {
    return m_predecessors;
  };
  int junction() const {
    return m_junction;
  };
  std::shared_ptr<Lane> getFirstLane();
  std::vector<std::shared_ptr<LaneSection>> sections() const {
    return m_sections;
  };  // Todo not const
  const std::vector<Point>& points() const {
    return m_roadPoints;
  };

private:
  std::shared_ptr<Map> m_map;  // only necessary for map construction.

  std::vector<std::shared_ptr<LaneSection>> m_sections;
  std::vector<std::shared_ptr<Road>> m_predecessors;
  std::vector<std::shared_ptr<Road>> m_successors;

  std::vector<glm::vec3> m_roadPoints;
  double m_length{0};
  uint16_t m_id{0};
  int m_junction{0};
  RoadType m_roadType;

  friend class MapBuilder;
};

class Map {
public:
  std::shared_ptr<Road> getRandomRoad();
  std::vector<std::shared_ptr<Road>> roads() const {
    return m_roads;
  }
  std::shared_ptr<Road> findRoadById(int rid);
  std::shared_ptr<Junction> findJunctionById(int jid);

private:
  std::vector<std::shared_ptr<Road>> m_roads;
  // should I use std::vector<Road> here? --> Builder pattern not possible? or
  // std::vector<std::unique_ptr<Road>> + raw pointers as member map_ for road,
  // lane?
  std::vector<std::shared_ptr<Junction>> m_junctions;

  friend class MapBuilder;
};

}  // namespace tsim

#endif  // __TSIM_MAP_HPP__
