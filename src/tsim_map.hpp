#ifndef __TSIM_MAP_HPP__
#define __TSIM_MAP_HPP__

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "tsim_util.hpp"

namespace tsim {

enum class LaneGroup { left, right, center };
enum class LaneType { sidewalk, shoulder, driving, restricted, median, parking, none };
enum class RoadType { road, junction };

class Map;
class Road;
class LaneSection;
class Lane {
   public:
    Lane(std::shared_ptr<LaneSection> lane_section) : lane_section_(lane_section){};
    Lane(const Lane& other) = delete;
    Lane(Lane&& other) = delete;
    Lane operator=(Lane& other) = delete;
    Lane operator=(Lane&& other) = delete;

    const Point startPoint() { return lane_points_.front(); }
    const double width() { return width_; }
    const double id() { return id_; }
    LaneType laneType() { return type_; }

    const std::vector<Point>& points() const { return lane_points_; }
    const std::vector<Point>& boundaryPoints() const { return lane_boundary_points_; }

    std::vector<std::shared_ptr<Lane>> predecessors() { return predecessors_; }
    std::vector<std::shared_ptr<Lane>> successors() { return successors_; }

   private:
    std::shared_ptr<LaneSection> lane_section_;
    std::vector<Point> lane_points_;
    std::vector<Point> lane_boundary_points_;

    std::vector<std::shared_ptr<Lane>> successors_;
    std::vector<std::shared_ptr<Lane>> predecessors_;

    int32_t id_{0};
    double offset_{0.0f};
    double width_{0.0f};
    LaneType type_;

    friend class MapBuilder;
};

class LaneSection {
   public:
    LaneSection(std::shared_ptr<Road> road) : road_(road){};
    LaneSection(const LaneSection& other) = delete;
    LaneSection(LaneSection&& other) = delete;
    LaneSection operator=(LaneSection& other) = delete;
    LaneSection operator=(LaneSection&& other) = delete;

    std::vector<std::shared_ptr<Lane>> lanes() const { return lanes_; }
    double s_offset() const { return s_offset_; }
    std::shared_ptr<Lane> lane(int id) const;
    std::shared_ptr<Road> road() { return road_; };
    std::vector<std::shared_ptr<LaneSection>> predecessors() { return predecessors_; }
    std::vector<std::shared_ptr<LaneSection>> successors() { return successors_; }

   private:
    std::shared_ptr<Road> road_;
    std::vector<std::shared_ptr<Lane>> lanes_;

    std::vector<std::shared_ptr<LaneSection>> successors_;
    std::vector<std::shared_ptr<LaneSection>> predecessors_;

    double s_offset_{0.0f};

    friend class MapBuilder;
};
struct LaneLink {
    int from_{0};
    int to_{0};
};
class JunctionConnection {
   public:
    const uint32_t incomingRoad() const { return incoming_road_; }
    const uint32_t connectingRoad() const { return connecting_road_; }
    const std::vector<LaneLink>& laneLinks() const { return lane_links_; }

    std::vector<LaneLink> lane_links_;

   private:
    uint16_t id_{0};
    uint32_t incoming_road_{0};
    uint32_t connecting_road_{0};

    friend class MapBuilder;
};
class Junction {
   public:
    Junction(std::shared_ptr<Map> map) : map_(map){};
    Junction(const Junction& other) = delete;
    Junction(Junction&& other) = delete;
    Junction operator=(const Junction& other) = delete;
    Junction operator=(Junction&& other) = delete;

    const uint32_t id() const { return id_; };
    std::vector<std::shared_ptr<JunctionConnection>> connections() { return connections_; };

   private:
    std::shared_ptr<Map> map_;

    std::vector<std::shared_ptr<JunctionConnection>> connections_;

   private:
    uint32_t id_{0};

    friend class MapBuilder;
};

class Road {
   public:
    Road(std::shared_ptr<Map> map) : map_(map){};
    Road(const Road& other) = delete;
    Road(Road&& other) = delete;
    Road operator=(const Road& other) = delete;
    Road operator=(Road&& other) = delete;

    const uint32_t id() const { return id_; };
    const Point startPoint() const { return road_points_.front(); };
    std::vector<std::shared_ptr<Road>> successors() { return successors_; };
    std::vector<std::shared_ptr<Road>> predecessors() { return predecessors_; };
    int junction() { return junction_; };
    std::shared_ptr<Lane> getFirstLane();
    std::vector<std::shared_ptr<LaneSection>> sections() const { return sections_; };  // Todo not const
    const std::vector<Point>& points() const { return road_points_; };

   private:
    std::shared_ptr<Map> map_;  // only necessary for map construction.

    std::vector<std::shared_ptr<LaneSection>> sections_;
    std::vector<std::shared_ptr<Road>> predecessors_;
    std::vector<std::shared_ptr<Road>> successors_;

    std::vector<Point> road_points_;
    double length_{0};
    uint16_t id_{0};
    int junction_{0};
    RoadType road_type_;

    friend class MapBuilder;
};

class Map {
   public:
    std::shared_ptr<Road> getRandomRoad() {
        auto road = roads_.at(std::rand() % roads_.size());
        auto lane_section = road->sections().front();  // TODO multiple lane sections
        auto lanes = lane_section->lanes();
        auto it = std::find_if(lanes.begin(), lanes.end(), [](std::shared_ptr<Lane> lane) { return lane->id() == -1; });
        if (it == lanes.end()) return getRandomRoad();
        return road;
    }
    std::vector<std::shared_ptr<Road>> roads() const { return roads_; }
    std::shared_ptr<Road> findRoadById(int id);
    std::shared_ptr<Junction> findJunctionById(int id);

   private:
    std::vector<std::shared_ptr<Road>> roads_;
    // should I use std::vector<Road> here? --> Builder pattern not possible? or std::vector<std::unique_ptr<Road>> +
    // raw pointers as member map_ for road, lane?
    std::vector<std::shared_ptr<Junction>> junctions_;

    friend class MapBuilder;
};

}  // namespace tsim

#endif  // __TSIM_MAP_HPP__
