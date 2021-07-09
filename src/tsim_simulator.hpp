#ifndef __TSIM_SIMULATOR_HPP__
#define __TSIM_SIMULATOR_HPP__

#include <thread>
#include <vector>

#include "renderer_sfml.hpp"

namespace tsim {
class Map;
class TrafficObject;
class Simulator {
   public:
    Simulator(std::shared_ptr<Map> map) : map_(map), renderer_(this){};
    ~Simulator();
    Simulator(const Simulator& other) = delete;
    Simulator(Simulator&& other) = delete;
    Simulator operator=(const Simulator& other) = delete;
    Simulator operator=(Simulator&& other) = delete;

    void run();
    void addVehicle();
    void addThread(std::thread&& thread);

    std::vector<std::shared_ptr<TrafficObject>> objects() { return objects_; };
    std::shared_ptr<Map> map() { return map_; };

   private:
    std::shared_ptr<Map> map_;
    // MapsRenderer renderer_;
    Renderer renderer_;
    // Renderer renderer_;

    std::vector<std::shared_ptr<TrafficObject>> objects_;
    std::vector<std::thread> threads_;
};
}  // namespace tsim
#endif  // __TSIM_SIMULATOR_HPP__
