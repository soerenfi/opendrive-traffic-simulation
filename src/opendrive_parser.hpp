#ifndef __OPENDRIVE_PARSER_HPP__
#define __OPENDRIVE_PARSER_HPP__

#include <tinyxml2.h>

#include "tsim_map_builder.hpp"

namespace parser {

class OpenDriveParser {
public:
  std::shared_ptr<tsim::Map> parse(const std::string& filename);

private:
  void parseHeader();
  void parseRoads();
  void parseJunctions();
  void roadConnections();
  void parseLaneSections();
  void parseLanes();
  void laneConnections();

  void parseLane(tsim::Lane* lane, const tinyxml2::XMLElement* odrLane);
  void parseLaneGroup(std::shared_ptr<tsim::LaneSection> lane_section, const tinyxml2::XMLElement* group);
  void laneConnections(std::shared_ptr<tsim::LaneSection> lane_section, const tinyxml2::XMLElement* group);

  // calculate geometrics
  void calculateRoadPoints(tsim::Road* road, const tinyxml2::XMLElement* odr_road);
  void calculateLanePoints(tsim::Lane* lane, const tinyxml2::XMLElement* odrLane);
  void calculateLaneBoundaryPoints(tsim::Lane* lane, const tinyxml2::XMLElement* odrLane);

  std::vector<tsim::Point> calculateStraight(double x, double y, double hdg, double length, double offset = 0);
  std::vector<tsim::Point> calculateArc(double x, double y, double hdg, double length, double arc, double offset = 0);

  // specific enum parsers
  tsim::LaneType parseLaneType(const char* lt);

private:


  tinyxml2::XMLDocument m_xmlDoc;
  tsim::MapBuilder m_mapBuilder;
};
}  // namespace parser

#endif  // __OPENDRIVE_PARSER_HPP__