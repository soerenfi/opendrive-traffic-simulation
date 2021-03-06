# OpenDrive Traffic Simulator
Simple traffic simulator based on ASAM OpenDrive Road description.
Current functionality:
Number of traffic participants is selected by user. Each traffic Participant is spawned on a random road and starts moving. Once the current Road is driven, a random successor road is selected from the Map representation and the Participant continues driving.

[ASAM OpenDrive](https://releases.asam.net/OpenDRIVE/1.6.0/ASAM_OpenDRIVE_BS_V1-6-0.html)

> This document is the copyrighted property of ASAM e.V. In alteration to the regular license terms, ASAM allows unrestricted distribution of this standard. §2 (1) of ASAM’s regular license terms is therefore substituted by the following clause: "The licensor grants everyone a basic, non-exclusive and unlimited license to use the standard ASAM OpenDRIVE".

## Project Structure

### main

The main thread reads in filename for opendrive File from command line arguments, instanciates the opendrive parser and passes the filename to it. It then instanciates the Simulator with the generated map. It adds vehicles/traffic participants to simulator and runs the simulation.
### opendrive_parser

parses opendrive xml using tinyxml2 and uses mapbuilder to create a tsim::Map from the opendrive contents. Creates discrete road/lane marking points from analytic opendrive road description.

### renderer_sfml

renders contents of tsim::Map (road/lane markings) and tsim::Vehicles.

### tsim_map_builder

Friend class of tsim::Map. encapsulates methods for populating the tsim::Map that are not needed during Runtime.
e.g. ```addLane```, ```addRoadSuccessor```, ...
also creates links between Roads, Lanes, Lanesections so that the necessary search algorithms are run before Runtime. (successors/predecessors)

### tsim_map

contains class Definitions for Map, Road, Lane, LaneSection, Junctions that describe the simulation map. Also provides methods to simulation users (Vehicles/Objects) to help them navigate the map.

### tsim_object

Abstract base class for Simulation objects and derived vehicle class. Abstract class owns object type independent properties (member variables), vehicle Instanciation owns vehicle specific properties. "Simulate" function is virtual in base class and defines object behavior (movement).

### tsim_simulator

the simulator owns the map, the object threads and the renderer. The "run" Function starts simulation of each object and rendering.
### tsim_util

Contains geometric and mathematic utilities, such as the class "Point" which is used for Map and vehicle position definition.

## Expected Behavior

OpenDrive map is loaded and displayed correctly. Defined number of Vehicles is displayed and moving along the road lanes in a consistent manner (i.e. without jumping around).
## Building

cmake 3.15+

    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/ null
    sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ xenial main'
    sudo apt update
    sudo apt install cmake

tinyxml2 https://github.com/leethomason/tinyxml2
sfml for graphical representation (included in thirdparty)
OpenDrive File acquired from here: https://github.com/carla-simulator/opendrive-test-files/blob/master/OpenDrive/Town01.xodr. More Information on OpenDrive can be found here: https://www.asam.net/standards/detail/opendrive/

    git submodule init
    git submodule update
    mkdir build
    cd build
    cmake ..
    make

## Project Rubric

* The project demonstrates an understanding of C++ functions and control structures. (e.g. opendrive_parser.cpp, parse functions for/if-else structures - opendrive_parser.cpp)
* The project reads data from a file and process the data, or the program writes data to a file (data is read from opendrive file and processed - opendrive_parser.cpp)
* The project accepts user input and processes the input (filename can be specified as cli input - main.cpp)
* The project uses Object Oriented Programming techniques (e.g class definition of Map objects - tsim_map.hpp)
* Classes use appropriate access specifiers for class members (private member variables of map objects - tsim_map.hpp)
* Class constructors utilize member initialization lists - tsim_object.cpp -line 10)
* Classes abstract implementation details from their interfaces (class TrafficObject/Vehicle  - tsim_object.hpp)
* Classes follow an appropriate inheritance hierarchy  - tsim_object.hpp -line 38)
* Templates generalize functions in the project (signum function - tsim_util.hpp - line 60)
* The project makes use of references in function declarations (tsim_map.hpp - line 34)
* The project follows the Rule of 5
The project uses move semantics to move data, instead of copying it, where possible. -> TODO

Remark: The Project employs the "Builder Pattern" for generation of the Road Map. Usage of heap allocated map members supports this. 

## Further Work

* pan& zoom for larger maps

* non-constant lane offsets

* lane changes

* mutex based intersection/traffic light handling

* Actual (e.g. PID based) lane following of traffic participants