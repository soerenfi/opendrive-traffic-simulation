#include "opendrive_parser.hpp"

#include <glm/glm.hpp>

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <type_traits>

#include "tsim_map.hpp"
#include "tsim_util.hpp"

namespace parser {

std::shared_ptr<tsim::Map> OpenDriveParser::parse(const std::string& filename) {
  tinyxml2::XMLError eResult = m_xmlDoc.LoadFile(filename.c_str());
  if (eResult != tinyxml2::XML_SUCCESS) {
    if (eResult == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
      throw std::runtime_error("File not found!");
    }
    throw std::runtime_error("File could not be loaded!");
  }
  auto* odr = m_xmlDoc.FirstChildElement("OpenDRIVE");
  if (odr == nullptr) {
    throw std::runtime_error("OpenDRIVE element not found");
  }

  parseHeader();
  parseRoads();
  parseJunctions();
  roadConnections();
  parseLaneSections();
  parseLanes();
  laneConnections();

  return m_mapBuilder.getMap();
}

void OpenDriveParser::parseHeader() {
  auto odr = m_xmlDoc.FirstChildElement("OpenDRIVE");
  auto rev_major = odr->FirstChildElement("header")->UnsignedAttribute("revMajor");
  auto rev_minor = odr->FirstChildElement("header")->UnsignedAttribute("revMinor");
}

void OpenDriveParser::parseRoads() {
  auto odr = m_xmlDoc.FirstChildElement("OpenDRIVE");

  for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
       odrRoad = odrRoad->NextSiblingElement("road")) {
    // add each road to map
    auto road =
      m_mapBuilder.addRoad(odrRoad->UnsignedAttribute("id"), odrRoad->IntAttribute("junction"));
    // calculate road geometries
    calculateRoadPoints(road.get(), odrRoad);
  }
}
void OpenDriveParser::parseJunctions() {
  auto* odr = m_xmlDoc.FirstChildElement("OpenDRIVE");
  for (auto* odrJunction = odr->FirstChildElement("junction"); odrJunction != nullptr;
       odrJunction = odrJunction->NextSiblingElement("junction")) {
    // add each junction to map
    auto junction = m_mapBuilder.addJunction(odrJunction->UnsignedAttribute("id"));
    for (auto* odrConnection = odrJunction->FirstChildElement("connection");
         odrConnection != nullptr;
         odrConnection = odrConnection->NextSiblingElement("connection")) {
      // add junction connections
      auto connection = m_mapBuilder.junction_addConnection(
        junction.get(), odrConnection->UnsignedAttribute("incomingRoad"),
        odrConnection->UnsignedAttribute("connectingRoad"));
      // add lane links
      for (auto* odrLaneLink = odrConnection->FirstChildElement("laneLink"); odrLaneLink != nullptr;
           odrLaneLink = odrLaneLink->NextSiblingElement("laneLink")) {
        int from = odrLaneLink->IntAttribute("from");
        int to = odrLaneLink->IntAttribute("to");
        m_mapBuilder.connection_addLaneLink(connection.get(), from, to);
      }
    }
  }
}
void OpenDriveParser::roadConnections() {
  // populate road successors/predecessors
  auto* odr = m_xmlDoc.FirstChildElement("OpenDRIVE");
  for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
       odrRoad = odrRoad->NextSiblingElement("road")) {
    auto road = m_mapBuilder.getRoad(odrRoad->UnsignedAttribute("id"));

    auto* odrPredecessor = odrRoad->FirstChildElement("link")->FirstChildElement("predecessor");
    if (odrPredecessor != nullptr) {
      // check if road is found, if not, predecessor is a junction
      auto predecessor = m_mapBuilder.getRoad(odrPredecessor->UnsignedAttribute("elementId"));
      if (predecessor) {
        m_mapBuilder.road_addPredecessor(road.get(), predecessor);
      } else {  // predecessor is a junction
        auto junction = m_mapBuilder.getJunction(odrPredecessor->UnsignedAttribute("elementId"));
        auto roads = m_mapBuilder.junction_findConnectingRoads(junction.get(), road.get());
        for (const auto& predecessors : roads) {
          m_mapBuilder.road_addPredecessor(road.get(), predecessors);
        }
      }
    }
    // check if road is found, if not, predecessor is a junction
    auto* odrSuccessor = odrRoad->FirstChildElement("link")->FirstChildElement("successor");
    if (odrSuccessor != nullptr) {
      auto successor = m_mapBuilder.getRoad(odrSuccessor->UnsignedAttribute("elementId"));
      if (successor) {
        m_mapBuilder.road_addSuccessor(road.get(), successor);
      } else {  // predecessor is a junction
        auto junction = m_mapBuilder.getJunction(odrSuccessor->UnsignedAttribute("elementId"));
        auto roads = m_mapBuilder.junction_findConnectingRoads(junction.get(), road.get());
        for (const auto& successors : roads) {
          m_mapBuilder.road_addSuccessor(road.get(), successors);
        }
      }
    }
  }
}

void OpenDriveParser::calculateRoadPoints(tsim::Road* road, const tinyxml2::XMLElement* odr_road) {
  const auto* planView = odr_road->FirstChildElement("planView");
  for (const auto* geom = planView->FirstChildElement("geometry"); geom != nullptr;
       geom = geom->NextSiblingElement("geometry")) {
    auto s = geom->DoubleAttribute("s");
    auto x = geom->DoubleAttribute("x");
    auto y = geom->DoubleAttribute("y");
    auto hdg = geom->DoubleAttribute("hdg");
    auto length = geom->DoubleAttribute("length");

    // TODO: spiral, poly3 (deprecated), parampoly3
    const auto* arc = geom->FirstChildElement("arc");

    std::vector<tsim::Point> points;
    if (arc != nullptr) {
      // Road geometry is an Arc.
      points = calculateArc(x, y, hdg, length, arc->DoubleAttribute("curvature"));
    } else {
      // Road geometry is a Straight.
      points = calculateStraight(x, y, hdg, length);
    }
    // add points for this geometry to road description
    m_mapBuilder.road_addRoadPoints(road, points);
  }
}

void OpenDriveParser::parseLaneSections() {
  auto* odr = m_xmlDoc.FirstChildElement("OpenDRIVE");
  for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
       odrRoad = odrRoad->NextSiblingElement("road")) {
    auto road = m_mapBuilder.getRoad(odrRoad->UnsignedAttribute("id"));
    auto* odrLanes = odrRoad->FirstChildElement("lanes");
    for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection");
         odrLaneSection != nullptr;
         odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
      // add all lane sections to map
      std::shared_ptr<tsim::LaneSection> laneSection =
        m_mapBuilder.road_addLaneSection(road, odrLaneSection->DoubleAttribute("s"));
    }
  }

  // laneSection connections
  for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
       odrRoad = odrRoad->NextSiblingElement("road")) {
    auto road = m_mapBuilder.getRoad(odrRoad->UnsignedAttribute("id"));
    auto* odrLanes = odrRoad->FirstChildElement("lanes");
    std::size_t laneSectionCounter{0};
    auto laneSections = road->sections();
    auto roadId = road->id();
    for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection");
         odrLaneSection != nullptr;
         odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
      // populate lane section connections
      if (odrRoad->IntAttribute("junction") == -1) {  // TODO not according to standard
        // road is not part of a junction. For first lane section, add last lane section of previous
        // road as precedessor. For last lane section, add first lane section of next road as
        // successor. Otherwise add prev/next lane section in road as successor.
        auto laneSection = laneSections.at(laneSectionCounter);
        if (laneSections.size() == 1) {
          auto roadPredecessors = road->predecessors();
          for (const auto& succ : roadPredecessors)
            m_mapBuilder.laneSection_addPredecessor(laneSection.get(), succ->sections().back());
          auto roadSuccessors = road->successors();
          for (const auto& succ : roadSuccessors)
            m_mapBuilder.laneSection_addSuccessor(laneSection.get(), succ->sections().front());
        } else if (laneSectionCounter == 0) {
          m_mapBuilder.laneSection_addSuccessor(laneSection.get(),
                                                laneSections.at(laneSectionCounter + 1));
          auto roadPredecessors = road->predecessors();
          for (const auto& succ : roadPredecessors)
            m_mapBuilder.laneSection_addPredecessor(laneSection.get(), succ->sections().back());
        } else if (laneSectionCounter == laneSections.size() - 1) {
          m_mapBuilder.laneSection_addPredecessor(laneSection.get(),
                                                  laneSections.at(laneSectionCounter - 1));
          auto roadSuccessors = road->successors();
          for (const auto& succ : roadSuccessors)
            m_mapBuilder.laneSection_addSuccessor(laneSection.get(), succ->sections().front());
        } else {
          m_mapBuilder.laneSection_addSuccessor(laneSection.get(),
                                                laneSections.at(laneSectionCounter + 1));
          m_mapBuilder.laneSection_addPredecessor(laneSection.get(),
                                                  laneSections.at(laneSectionCounter - 1));
        }
      } else {
        // road is part of a junction. Add last lane section of previous road as predecessor, first
        // lane section of subsequent road as successor
        auto laneSection = laneSections.at(laneSectionCounter);
        auto roadPredecessors = road->predecessors();
        for (const auto& succ : roadPredecessors)
          m_mapBuilder.laneSection_addPredecessor(laneSection.get(), succ->sections().back());
        auto roadSuccessors = road->successors();
        for (const auto& succ : roadSuccessors)
          m_mapBuilder.laneSection_addSuccessor(laneSection.get(), succ->sections().front());
      }
      laneSectionCounter++;
    }
  }
}
void OpenDriveParser::parseLanes() {
  auto* odr = m_xmlDoc.FirstChildElement("OpenDRIVE");
  for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
       odrRoad = odrRoad->NextSiblingElement("road")) {
    auto road = m_mapBuilder.getRoad(odrRoad->UnsignedAttribute("id"));
    auto* odrLanes = odrRoad->FirstChildElement("lanes");
    //
    std::size_t laneSectionCounter{0};
    for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection");
         odrLaneSection != nullptr;
         odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
      auto laneSections = road->sections();
      auto laneSection = laneSections.at(laneSectionCounter);

      auto* groupLeft = odrLaneSection->FirstChildElement("left");
      auto* groupRight = odrLaneSection->FirstChildElement("right");
      if (groupLeft != nullptr) {
        parseLaneGroup(laneSection, groupLeft);
      }
      if (groupRight != nullptr) {
        parseLaneGroup(laneSection, groupRight);
      }

      laneSectionCounter++;
    }
  }
}
void OpenDriveParser::laneConnections() {
  auto* odr = m_xmlDoc.FirstChildElement("OpenDRIVE");
  // populate lane successors/predecessors
  for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
       odrRoad = odrRoad->NextSiblingElement("road")) {
    auto road = m_mapBuilder.getRoad(odrRoad->UnsignedAttribute("id"));
    auto* odrLanes = odrRoad->FirstChildElement("lanes");
    std::size_t laneSectionCounter{0};
    for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection");
         odrLaneSection != nullptr;
         odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
      auto laneSections = road->sections();
      auto laneSection = laneSections.at(laneSectionCounter);
      auto* groupLeft = odrLaneSection->FirstChildElement("left");
      auto* groupRight = odrLaneSection->FirstChildElement("right");

      if (groupLeft != nullptr) {
        laneConnections(laneSection, groupLeft);
      }
      if (groupRight != nullptr) {
        laneConnections(laneSection, groupRight);
      }
      laneSectionCounter++;
    }
  }
}

void OpenDriveParser::parseLaneGroup(std::shared_ptr<tsim::LaneSection> lane_section,
                                     const tinyxml2::XMLElement* group) {
  for (auto* odrLane = group->FirstChildElement("lane"); odrLane != nullptr;
       odrLane = odrLane->NextSiblingElement("lane")) {
    // add all lanes to the map
    auto lane = m_mapBuilder.laneSection_addLane(
      lane_section, odrLane->IntAttribute("id"),
      odrLane->FirstChildElement("width")->DoubleAttribute("sOffset"),
      odrLane->FirstChildElement("width")->DoubleAttribute("a"),
      parseLaneType(odrLane->Attribute("type")));
    // calculate lane geometries
    calculateLaneBoundaryPoints(lane.get(), odrLane);
    calculateLanePoints(lane.get(), odrLane);
  }
}

void OpenDriveParser::laneConnections(std::shared_ptr<tsim::LaneSection> lane_section,
                                      const tinyxml2::XMLElement* group) {
  for (auto* odrLane = group->FirstChildElement("lane"); odrLane != nullptr;
       odrLane = odrLane->NextSiblingElement("lane")) {
    if (!strcmp(odrLane->Attribute("type"), "driving")) {  // TODO only driving Lanes
      auto lane = lane_section->lane(odrLane->UnsignedAttribute("id"));
      // auto road = m_mapBuilder.getRoad(lane_section->road()->id());
      auto road = lane_section->road();
      auto road_id = lane_section->road()->id();
      auto link = odrLane->FirstChildElement("link");
      if (link) {
        auto odr_predecessor = link->FirstChildElement("predecessor");
        if (odr_predecessor) {
          int id = odr_predecessor->IntAttribute("id");
          // find predecessor
          auto lane_section_predecessors = lane_section->predecessors();
          for (auto elem : lane_section_predecessors) {
            m_mapBuilder.lane_addPredecessor(lane.get(),
                                             elem->lane(odr_predecessor->IntAttribute("id")));
          }
        } else {
          // predecessor is a junction
          auto lane_section_predecessors = lane_section->predecessors();
          for (auto elem : lane_section_predecessors) {
            // find the junction it belongs to
            // auto junction = elem->road()->junction();
            auto junction = m_mapBuilder.getJunction(elem->road()->junction());

            if (junction) {
              // check if lane links contain current lane
              for (auto conn : junction->connections()) {
                if ((conn->incomingRoad() == lane_section->road()->id()) &&
                    (conn->connectingRoad() == elem->road()->id())) {
                  auto lls = conn->getLaneLinks();
                  std::vector<tsim::LaneLink> res;
                  std::copy_if(lls.begin(), lls.end(), std::back_inserter(res),
                               [lane](const tsim::LaneLink& ll) {
                                 return ll.from == lane->id();
                               });
                  for (auto link : res)
                    m_mapBuilder.lane_addPredecessor(lane.get(), elem->lane(link.to));
                }
              }
            }
          }
        }
        auto odr_successor = link->FirstChildElement("successor");
        if (odr_successor) {
          auto lane_section_successors = lane_section->successors();
          for (auto elem : lane_section_successors) {
            m_mapBuilder.lane_addSuccessor(lane.get(),
                                           elem->lane(odr_successor->IntAttribute("id")));
          }
        } else {
          // check if predecessor is a junction
          auto lane_section_successors = lane_section->successors();
          for (auto elem : lane_section_successors) {
            // find the junction it belongs to
            // auto junction = elem->road()->junction();
            auto junction = m_mapBuilder.getJunction(elem->road()->junction());
            // check if lane links contain current lane i
            if (junction) {
              for (auto conn : junction->connections()) {
                if ((conn->incomingRoad() == lane_section->road()->id()) &&
                    (conn->connectingRoad() == elem->road()->id())) {
                  auto lls = conn->getLaneLinks();
                  std::vector<tsim::LaneLink> res;
                  std::copy_if(lls.begin(), lls.end(), std::back_inserter(res),
                               [lane](const tsim::LaneLink& ll) {
                                 return ll.from == lane->id();
                               });
                  for (auto link : res)
                    m_mapBuilder.lane_addSuccessor(lane.get(), elem->lane(link.to));
                }
              }
            }
          }
        }
      } else {
        // no link for lane
        auto lane_section_predecessors = lane_section->predecessors();
        for (auto elem : lane_section_predecessors) {
          // find the junction it belongs to
          // auto junction = elem->road()->junction();

          auto junction = m_mapBuilder.getJunction(elem->road()->junction());
          // check if lane links contain current lane
          if (junction) {
            for (auto conn : junction->connections()) {
              if ((conn->incomingRoad() == lane_section->road()->id()) &&
                  (conn->connectingRoad() == elem->road()->id())) {
                auto lls = conn->getLaneLinks();
                std::vector<tsim::LaneLink> res;
                std::copy_if(lls.begin(), lls.end(), std::back_inserter(res),
                             [lane](const tsim::LaneLink& ll) {
                               return ll.from == lane->id();
                             });
                for (auto link : res)
                  m_mapBuilder.lane_addPredecessor(lane.get(), elem->lane(link.to));
              }
            }
          }
        }
        auto lane_section_successors = lane_section->successors();
        for (auto elem : lane_section_successors) {
          // find the junction it belongs to
          // auto junction = elem->road()->junction();
          auto junction = m_mapBuilder.getJunction(elem->road()->junction());
          // check if lane links contain current lane i
          if (junction) {
            for (auto conn : junction->connections()) {
              if ((conn->incomingRoad() == lane_section->road()->id()) &&
                  (conn->connectingRoad() == elem->road()->id())) {
                auto lls = conn->getLaneLinks();
                std::vector<tsim::LaneLink> res;
                std::copy_if(lls.begin(), lls.end(), std::back_inserter(res),
                             [lane](const tsim::LaneLink& ll) {
                               return ll.from == lane->id();
                             });
                for (auto link : res) {
                  m_mapBuilder.lane_addSuccessor(lane.get(), elem->lane(link.to));
                }
              }
            }
          }
        }
      }
    }
  }
}

void OpenDriveParser::calculateLaneBoundaryPoints(tsim::Lane* lane,
                                                  const tinyxml2::XMLElement* odrLane) {
  const auto* odrRoad = odrLane->Parent()->Parent()->Parent()->Parent();

  auto planView = odrRoad->FirstChildElement("planView");
  auto offset = lane->width() * tsim::util::sgn(lane->id());
  for (const auto* geom = planView->FirstChildElement("geometry"); geom != nullptr;
       geom = geom->NextSiblingElement("geometry")) {
    auto s = geom->DoubleAttribute("s");
    auto x = geom->DoubleAttribute("x");
    auto y = geom->DoubleAttribute("y");
    auto hdg = geom->DoubleAttribute("hdg");
    auto length = geom->DoubleAttribute("length");

    // line, spiral, arc, poly3, parampoly3
    const auto* arc = geom->FirstChildElement("arc");

    std::vector<tsim::Point> points;
    if (arc != nullptr) {
      points = calculateArc(x, y, hdg, length, arc->DoubleAttribute("curvature"), offset);

    } else {
      points = calculateStraight(x, y, hdg, length, offset);
    }
    m_mapBuilder.lane_addLaneBoundaryPoints(lane, points);
  }
}

void OpenDriveParser::calculateLanePoints(tsim::Lane* lane, const tinyxml2::XMLElement* odrLane) {
  const auto* odrRoad = odrLane->Parent()->Parent()->Parent()->Parent();

  const auto* planView = odrRoad->FirstChildElement("planView");
  auto offset = lane->width() * tsim::util::sgn(lane->id());
  offset = offset / 2;  // half lane width for lane center
  for (const auto* geom = planView->FirstChildElement("geometry"); geom != nullptr;
       geom = geom->NextSiblingElement("geometry")) {
    auto s = geom->DoubleAttribute("s");
    auto x = geom->DoubleAttribute("x");
    auto y = geom->DoubleAttribute("y");
    auto hdg = geom->DoubleAttribute("hdg");
    auto length = geom->DoubleAttribute("length");

    // line, spiral, arc, poly3, parampoly3
    const auto* arc = geom->FirstChildElement("arc");

    std::vector<tsim::Point> points;
    if (arc != nullptr) {
      points = calculateArc(x, y, hdg, length, arc->DoubleAttribute("curvature"), offset);

    } else {
      points = calculateStraight(x, y, hdg, length, offset);
    }
    m_mapBuilder.lane_addLanePoints(lane, points);
  }
}

std::vector<tsim::Point> OpenDriveParser::calculateStraight(double x, double y, double hdg,
                                                            double length, double offset) {
  std::vector<tsim::Point> points;

  tsim::Point pOffset(-offset * std::sin(hdg), offset * std::cos(hdg), 0);  // TODO
  double xOffset = -offset * std::sin(hdg);
  double yOffset = offset * std::cos(hdg);

  points.emplace_back(glm::vec3{x + xOffset, y + yOffset, 0.0f});
  auto end =
    glm::vec3{x + length * std::cos(hdg) + xOffset, y + length * std::sin(hdg) + yOffset, 0.0f};

  double roadLength{0};
  double step = 1.f;
  while (roadLength < length) {
    glm::vec3 next =
      glm::vec3{(x += step * std::cos(hdg)) + xOffset, (y += step * std::sin(hdg)) + yOffset, 0.0f};
    roadLength += step;
    points.emplace_back(next);
  }
  points.emplace_back(end);
  return points;
}

std::vector<tsim::Point> OpenDriveParser::calculateArc(double x, double y, double hdg,
                                                       double length, double arc, double offset) {
  std::vector<tsim::Point> points;
  double xOffset = -offset * std::sin(hdg);
  double yOffset = offset * std::cos(hdg);
  points.emplace_back(glm::vec3{x + xOffset, y + yOffset, 0.0f});

  auto xM = x + 1 / arc * -std::sin(hdg);
  auto yM = y + 1 / arc * std::cos(hdg);
  double road_length{0};
  double step = 0.3f;
  double start_angle = hdg - tsim::util::sgn(arc) * M_PI / 2;
  double end_angle = start_angle + length * arc;

  tsim::Point end((xM + (1 / std::abs(arc) + tsim::util::sgn(arc) * -offset) * std::cos(end_angle)),
                  (yM + (1 / std::abs(arc) + tsim::util::sgn(arc) * -offset) * std::sin(end_angle)),
                  0.0f);

  for (double angle = start_angle; tsim::util::sgn(arc) * angle < tsim::util::sgn(arc) * end_angle;
       angle += length * arc / 20) {
    glm::vec3 next = glm::vec3{
      (xM + (1 / std::abs(arc) + tsim::util::sgn(arc) * -offset) * std::cos(angle)),
      (yM + (1 / std::abs(arc) + tsim::util::sgn(arc) * -offset) * std::sin(angle)), 0.0f};
    points.emplace_back(next);
  }
  points.emplace_back(end);

  return points;
}

tsim::LaneType OpenDriveParser::parseLaneType(const char* lt) {
  if (strcmp(lt, "sidewalk") == 0) return tsim::LaneType::eSIDEWALK;
  if (strcmp(lt, "shoulder") == 0) return tsim::LaneType::eSHOULDER;
  if (strcmp(lt, "driving") == 0) return tsim::LaneType::eDRIVING;
  if (strcmp(lt, "restricted") == 0) return tsim::LaneType::eRESTRICTED;
  if (strcmp(lt, "median") == 0) return tsim::LaneType::eMEDIAN;
  if (strcmp(lt, "parking") == 0) return tsim::LaneType::ePARKING;
  if (strcmp(lt, "none") == 0) return tsim::LaneType::eNONE;
  throw std::logic_error("unknown lane type " + std::string(lt));
}
}  // namespace parser