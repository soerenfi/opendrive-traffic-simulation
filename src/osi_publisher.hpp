
#pragma once

#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>
#include <osi3/osi_sensorview.pb.h>

namespace tsim {
class Simulator;
}

class OsiPublisher {
public:
  explicit OsiPublisher(tsim::Simulator* sim);
  ~OsiPublisher() = default;

  void start();

private:
  void publish();
  tsim::Simulator* m_simulator;
  osi3::SensorView m_sensorView;

  std::unique_ptr<eCAL::protobuf::CPublisher<osi3::SensorView>> m_publisher;
};