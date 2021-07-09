#include <bits/stdint-uintn.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "tsim_map.hpp"
namespace tsim {

std::shared_ptr<Lane> LaneSection::lane(int id) const {
    auto it = std::find_if(lanes_.begin(), lanes_.end(), [id](std::shared_ptr<Lane> lane) { return lane->id() == id; });
    if (it == lanes_.end())
        throw std::logic_error("lane " + std::to_string(id) + "not found in Road " + std::to_string(this->road_->id()));
    return *it;
}
std::shared_ptr<Lane> Road::getFirstLane() {
    auto lane_section = sections_.front();
    auto lanes = lane_section->lanes();
    return lanes.front();
};

std::shared_ptr<Road> Map::findRoadById(int id) {
    auto it = std::find_if(roads_.begin(), roads_.end(), [id](std::shared_ptr<Road> road) { return road->id() == id; });
    if (it != roads_.end()) return (*it);
    return nullptr;
}
std::shared_ptr<Junction> Map::findJunctionById(int id) {
    auto it = std::find_if(junctions_.begin(), junctions_.end(), [id](std::shared_ptr<Junction> junction) {
        return junction->id() == id;
    });
    if (it != junctions_.end()) return (*it);
    return nullptr;
}

}  // namespace tsim