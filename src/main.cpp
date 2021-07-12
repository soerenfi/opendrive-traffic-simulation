
#include <algorithm>
#include <iostream>

#include "opendrive_parser.hpp"
#include "tsim_object.hpp"
#include "tsim_simulator.hpp"

int main(int argc, char* argv[]) {
    std::string filename{"../xodr/Town01.xodr"};
    // std::string filename{"../xodr/TownBig.xodr"};

    std::vector<std::string> args(argv + 1, argv + argc);
    if (argc > 1) filename = args.front();

    std::cout << "loading OpenDrive file " << filename << std::endl;

    parser::OpenDriveParser parser;
    auto map = parser.parse(filename);

    tsim::Simulator sim(map);
    std::size_t num_vehicles = 10;

    for (std::size_t i = 0; i < num_vehicles; i++) sim.addVehicle();

    sim.run();
}