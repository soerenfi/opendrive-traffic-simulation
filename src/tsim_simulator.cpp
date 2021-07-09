#include "tsim_simulator.hpp"
//
#include "tsim_object.hpp"
namespace tsim {

Simulator::~Simulator() {
    std::for_each(threads_.begin(), threads_.end(), [](std::thread& t) { t.join(); });
}
void Simulator::run() {
    for (auto const& obj : objects_) {
        obj->simulate();
    }
    renderer_.render();
}
void Simulator::addThread(std::thread&& thread) { threads_.emplace_back(std::move(thread)); }
void Simulator::addVehicle() { objects_.emplace_back(std::make_shared<Vehicle>(map_, this)); }
}  // namespace tsim