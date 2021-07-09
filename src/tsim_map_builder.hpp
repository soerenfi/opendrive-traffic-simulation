#ifndef __TSIM_MAP_BUILDER_HPP__
#define __TSIM_MAP_BUILDER_HPP__

#include <memory>

#include "tsim_map.hpp"

namespace tsim {
class Map;
class Road;
class Junction;
class LaneSection;
class MapBuilder {
   public:
    MapBuilder() { map_ = std::make_shared<Map>(); }
    MapBuilder(const MapBuilder& other) = delete;
    MapBuilder(MapBuilder&& other) = delete;
    MapBuilder operator=(const MapBuilder& other) = delete;
    MapBuilder operator=(MapBuilder& other) = delete;

    std::shared_ptr<Road> addRoad(int id, int junction);
    std::shared_ptr<Road> getRoad(int id);
    std::shared_ptr<Junction> addJunction(int id);
    std::shared_ptr<Junction> getJunction(int id);

    void road_addRoadPoints(Road* road, std::vector<Point> points);
    void road_addPredecessor(Road* road, std::shared_ptr<Road> predecessor);
    void road_addSuccessor(Road* road, std::shared_ptr<Road> successor);

    std::vector<std::shared_ptr<Road>> junction_findConnectingRoads(Junction* junction, Road* road);
    std::shared_ptr<JunctionConnection> junction_addConnection(
        Junction* junction, uint32_t incoming_road, uint32_t connecting_road);
    void connection_addLaneLink(JunctionConnection* connection, int from, int to);

    std::shared_ptr<LaneSection> road_addLaneSection(std::shared_ptr<Road> road, double s_offset);
    void laneSection_addPredecessor(LaneSection* lane_section, std::shared_ptr<LaneSection> predecessor);
    void laneSection_addSuccessor(LaneSection* lane_section, std::shared_ptr<LaneSection> successor);
    std::shared_ptr<Lane> laneSection_addLane(
        std::shared_ptr<LaneSection> lane_section, uint32_t id, double offset, double width, LaneType type);

    void lane_addLanePoints(Lane* lane, std::vector<Point> points);
    void lane_addLaneBoundaryPoints(Lane* lane, std::vector<Point> points);
    void lane_addPredecessor(Lane* lane, std::shared_ptr<Lane> predecessor);
    void lane_addSuccessor(Lane* lane, std::shared_ptr<Lane> successor);

    std::shared_ptr<Map> getMap();

   private:
    std::shared_ptr<Map> map_;
};
}  // namespace tsim
#endif  // __TSIM_MAP_BUILDER_HPP__