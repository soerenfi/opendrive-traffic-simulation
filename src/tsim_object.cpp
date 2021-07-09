#include <iostream>
#include <thread>

#include "tsim_map.hpp"
#include "tsim_object.hpp"

namespace tsim {
uint16_t TrafficObject::id_counter_{0};

TrafficObject::TrafficObject(std::shared_ptr<Map> map, Simulator* sim) : map_(map), simulator_(sim) {
    id_ = id_counter_++;
}

Vehicle::Vehicle(std::shared_ptr<Map> map, Simulator* sim) : TrafficObject(map, sim) {
    current_road_ = map_->getRandomRoad();
    current_lane_ = current_road_->sections().at(0)->lane(-1);
    position_ = current_lane_->startPoint();
}

void Vehicle::simulate() { simulator_->addThread(std::thread(&Vehicle::drive, this)); };

void Vehicle::drive() {
    auto lane_size = current_lane_->points().size() - 1;
    auto step = std::chrono::milliseconds(20);
    int lane_step = 0;
    int steps_taken{0};
    while (true) {
        auto now = std::chrono::system_clock::now();
        auto target = now + step;
        position_ = current_lane_->points().at(lane_step);

        if (steps_taken < lane_size) {
            if (current_lane_->id() < 0) {
                lane_step += 1;
            } else {
                lane_step -= 1;
            }
            steps_taken++;
        }

        else {
            // end of road reached
            // std::vector<std::shared_ptr<Road>> next_roads;
            std::vector<std::shared_ptr<Lane>> next_lanes;
            if (current_lane_->id() < 0) {
                // next_roads = current_road_->successors();
                next_lanes = current_lane_->successors();
            } else {
                // next_roads = current_road_->predecessors();
                next_lanes = current_lane_->predecessors();
            }
            // pick random road to continue driving
            auto rand_it = next_lanes.begin();
            std::advance(rand_it, std::rand() % next_lanes.size());
            current_lane_ = (*rand_it);

            lane_size = current_lane_->points().size() - 1;

            // jump to start/end of new road depending on driving direction
            if (current_lane_->id() < 0) {
                lane_step = 0;

            } else {
                lane_step = current_lane_->points().size() - 1;
            }
            steps_taken = 0;
        }
        std::this_thread::sleep_until(target);
    }
}
}  // namespace tsim