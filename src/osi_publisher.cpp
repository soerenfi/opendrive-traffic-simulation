#include "osi_publisher.hpp"

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>

#include "tsim_object.hpp"
#include "tsim_simulator.hpp"

OsiPublisher::OsiPublisher(tsim::Simulator* sim)
    : m_simulator(sim) {
  eCAL::Initialize(0, nullptr, "TSim OSI Publisher");
  m_publisher = std::make_unique<eCAL::protobuf::CPublisher<osi3::SensorView>>("osi3_SensorView");
};

void OsiPublisher::start() {
  m_simulator->addThread(std::thread(&OsiPublisher::publish, this));
}

void OsiPublisher::publish() {
  while (eCAL::Ok()) {
    osi3::SensorView sensorView;

    for (auto object : m_simulator->getObjects()) {
      auto* osiObject = sensorView.mutable_global_ground_truth()->add_moving_object();
      osiObject->mutable_base()->mutable_position()->set_x(object->getPosition().x);
      osiObject->mutable_base()->mutable_position()->set_y(object->getPosition().y);
      osiObject->mutable_base()->mutable_position()->set_z(object->getPosition().z);
    }

    m_publisher->Send(sensorView);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
