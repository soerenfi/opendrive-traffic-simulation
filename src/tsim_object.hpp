#ifndef __TSIM_OBJECT_HPP__
#define __TSIM_OBJECT_HPP__

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
    TrafficObject(std::shared_ptr<Map> map, Simulator* sim);
    virtual ~TrafficObject() = default;

    virtual void simulate(){};

    const Point& position() const { return position_; };

   protected:
    std::shared_ptr<Map> map_;
    Simulator* simulator_;

    Point position_{0.0f, 0.0f};

    uint16_t id_;
    static uint16_t id_counter_;
};

class Vehicle : public TrafficObject {
   public:
    Vehicle(std::shared_ptr<Map> map, Simulator* sim);

    virtual void simulate() override;

   private:
    void drive();

    std::shared_ptr<Road> current_road_;
    std::shared_ptr<Lane> current_lane_;
};

}  // namespace tsim

#endif  // __TSIM_OBJECT_HPP__
