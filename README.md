# OpenDrive Traffic Simulator
Simple traffic simulator based on ASAM OpenDrive Road description.
Current functionality:
Number of traffic participants is selected by user. Each traffic Participant is spawned on a random road and starts moving. Once the current Road is driven, a random successor road is selected from the Map representation and the Participant continues driving.

[ASAM OpenDrive](https://releases.asam.net/OpenDRIVE/1.6.0/ASAM_OpenDRIVE_BS_V1-6-0.html)

> This document is the copyrighted property of ASAM e.V. In alteration to the regular license terms, ASAM allows unrestricted distribution of this standard. §2 (1) of ASAM’s regular license terms is therefore substituted by the following clause: "The licensor grants everyone a basic, non-exclusive and unlimited license to use the standard ASAM OpenDRIVE".

## Building

Prerequisites:

cmake 3.15+

    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/ null
    sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ xenial main'
    sudo apt update
    sudo apt install cmake

tinyxml2 https://github.com/leethomason/tinyxml2
sfml for graphical representation
OpenDrive File acquired from here: https://github.com/carla-simulator/opendrive-test-files/blob/master/OpenDrive/Town01.xodr

    git submodule init
    git submodule update
    mkdir build
    cd build
    cmake ..
    make

## Project Rubric

* The project demonstrates an understanding of C++ functions and control structures.
* The project reads data from a file and process the data, or the program writes data to a file.
* The project accepts user input and processes the input.
* The project uses Object Oriented Programming techniques.
* Classes use appropriate access specifiers for class members.
* Class constructors utilize member initialization lists.
* Classes abstract implementation details from their interfaces. -> TODO
* Classes follow an appropriate inheritance hierarchy.
* Templates generalize functions in the project.
* The project makes use of references in function declarations.
* The project follows the Rule of 5. -> TODO
The project uses move semantics to move data, instead of copying it, where possible. -> TODO


The Project employs the "Builder Pattern" for generation of the Road Map.

## Further Work

pan& zoom for larger maps
non-constant lane offsets
Multiple Lanes and lane changes
mutex based intersection/traffic light handling
Actual (e.g. PID based) lane following of traffic participants