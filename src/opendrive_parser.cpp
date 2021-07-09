#include <cmath>
#include <iostream>
#include <stdexcept>

#include "opendrive_parser.hpp"
#include "tsim_map.hpp"
#include "tsim_util.hpp"

using namespace tsim;
namespace parser {

std::shared_ptr<Map> OpenDriveParser::parse(std::string filename) {
    tinyxml2::XMLError eResult = xml_doc_.LoadFile(filename.c_str());
    if (eResult != tinyxml2::XML_SUCCESS) {
        if (eResult == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
            throw std::runtime_error("File not found!");
        } else {
            throw std::runtime_error("File could not be loaded!");
        }
    }
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");
    if (odr == nullptr) throw std::runtime_error("OpenDRIVE element not found");

    parseHeader();
    parseRoads();
    parseJunctions();
    roadConnections();
    parseLaneSections();
    parseLanes();
    laneConnections();

    return map_builder_.getMap();
}

void OpenDriveParser::parseHeader() {
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");
    auto rev_major = odr->FirstChildElement("header")->UnsignedAttribute("revMajor");
    auto rev_minor = odr->FirstChildElement("header")->UnsignedAttribute("revMinor");
}

void OpenDriveParser::parseRoads() {
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");

    for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
         odrRoad = odrRoad->NextSiblingElement("road")) {
        // add each road to map
        auto road = map_builder_.addRoad(odrRoad->UnsignedAttribute("id"), odrRoad->IntAttribute("junction"));
        // calculate road geometries
        calculateRoadPoints(road.get(), odrRoad);
    }
}
void OpenDriveParser::parseJunctions() {
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");
    for (auto* odrJunction = odr->FirstChildElement("junction"); odrJunction != nullptr;
         odrJunction = odrJunction->NextSiblingElement("junction")) {
        // add each junction to map
        auto junction = map_builder_.addJunction(odrJunction->UnsignedAttribute("id"));
        for (auto* odrConnection = odrJunction->FirstChildElement("connection"); odrConnection != nullptr;
             odrConnection = odrConnection->NextSiblingElement("connection")) {
            // add junction connections
            auto connection = map_builder_.junction_addConnection(
                junction.get(), odrConnection->UnsignedAttribute("incomingRoad"),
                odrConnection->UnsignedAttribute("connectingRoad"));
            // add lane links
            for (auto* odrLaneLink = odrConnection->FirstChildElement("laneLink"); odrLaneLink != nullptr;
                 odrLaneLink = odrLaneLink->NextSiblingElement("laneLink")) {
                int from = odrLaneLink->IntAttribute("from");
                int to = odrLaneLink->IntAttribute("to");
                map_builder_.connection_addLaneLink(connection.get(), from, to);
            }
        }
    }
}
void OpenDriveParser::roadConnections() {
    // populate road successors/predecessors
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");
    for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
         odrRoad = odrRoad->NextSiblingElement("road")) {
        auto road = map_builder_.getRoad(odrRoad->UnsignedAttribute("id"));

        auto odr_predecessor = odrRoad->FirstChildElement("link")->FirstChildElement("predecessor");
        if (odr_predecessor) {
            // check if road is found, if not, predecessor is a junction
            auto predecessor = map_builder_.getRoad(odr_predecessor->UnsignedAttribute("elementId"));
            if (predecessor) {
                map_builder_.road_addPredecessor(road.get(), predecessor);
            } else {  // predecessor is a junction
                auto junction = map_builder_.getJunction(odr_predecessor->UnsignedAttribute("elementId"));
                auto roads = map_builder_.junction_findConnectingRoads(junction.get(), road.get());
                for (auto predecessors : roads) map_builder_.road_addPredecessor(road.get(), predecessors);
            }
        }
        // check if road is found, if not, predecessor is a junction
        auto odr_successor = odrRoad->FirstChildElement("link")->FirstChildElement("successor");
        if (odr_successor) {
            auto successor = map_builder_.getRoad(odr_successor->UnsignedAttribute("elementId"));
            if (successor) {
                map_builder_.road_addSuccessor(road.get(), successor);
            } else {  // predecessor is a junction
                auto junction = map_builder_.getJunction(odr_successor->UnsignedAttribute("elementId"));
                auto roads = map_builder_.junction_findConnectingRoads(junction.get(), road.get());
                for (auto successors : roads) map_builder_.road_addSuccessor(road.get(), successors);
            }
        }
    }
}

void OpenDriveParser::calculateRoadPoints(Road* road, const tinyxml2::XMLElement* odr_road) {
    auto plan_view = odr_road->FirstChildElement("planView");
    for (auto* geom = plan_view->FirstChildElement("geometry"); geom != nullptr;
         geom = geom->NextSiblingElement("geometry")) {
        auto s = geom->DoubleAttribute("s");
        auto x = geom->DoubleAttribute("x");
        auto y = geom->DoubleAttribute("y");
        auto hdg = geom->DoubleAttribute("hdg");
        auto length = geom->DoubleAttribute("length");

        // TODO: spiral, poly3 (deprecated), parampoly3
        auto arc = geom->FirstChildElement("arc");

        std::vector<Point> points;
        if (arc) {
            // Road geometry is an Arc.
            points = calculateArc(x, y, hdg, length, arc->DoubleAttribute("curvature"));
        } else {
            // Road geometry is a Straight.
            points = calculateStraight(x, y, hdg, length);
        }
        // add points for this geometry to road description
        map_builder_.road_addRoadPoints(road, points);
    }
}

void OpenDriveParser::parseLaneSections() {
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");
    for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
         odrRoad = odrRoad->NextSiblingElement("road")) {
        auto road = map_builder_.getRoad(odrRoad->UnsignedAttribute("id"));
        auto* odrLanes = odrRoad->FirstChildElement("lanes");
        for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection"); odrLaneSection != nullptr;
             odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
            // add all lane sections to map
            std::shared_ptr<LaneSection> lane_section =
                map_builder_.road_addLaneSection(road, odrLaneSection->DoubleAttribute("s"));
        }
    }

    // laneSection connections
    for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
         odrRoad = odrRoad->NextSiblingElement("road")) {
        auto road = map_builder_.getRoad(odrRoad->UnsignedAttribute("id"));
        auto* odrLanes = odrRoad->FirstChildElement("lanes");
        std::size_t lane_section_counter{0};
        auto lane_sections = road->sections();
        auto road_id = road->id();
        for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection"); odrLaneSection != nullptr;
             odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
            // populate lane section connections
            if (odrRoad->IntAttribute("junction") == -1) {  // TODO not according to standard
                // road is not part of a junction. For first lane section, add last lane section of previous road as
                // precedessor. For last lane section, add first lane section of next road as successor. Otherwise
                // add prev/next lane section in road as successor.
                auto lane_section = lane_sections.at(lane_section_counter);
                if (lane_sections.size() == 1) {
                    auto road_predecessors = road->predecessors();
                    for (auto succ : road_predecessors)
                        map_builder_.laneSection_addPredecessor(lane_section.get(), succ->sections().back());
                    auto road_successors = road->successors();
                    for (auto succ : road_successors)
                        map_builder_.laneSection_addSuccessor(lane_section.get(), succ->sections().front());
                } else if (lane_section_counter == 0) {
                    map_builder_.laneSection_addSuccessor(
                        lane_section.get(), lane_sections.at(lane_section_counter + 1));
                    auto road_predecessors = road->predecessors();
                    for (auto succ : road_predecessors)
                        map_builder_.laneSection_addPredecessor(lane_section.get(), succ->sections().back());
                } else if (lane_section_counter == lane_sections.size() - 1) {
                    map_builder_.laneSection_addPredecessor(
                        lane_section.get(), lane_sections.at(lane_section_counter - 1));
                    auto road_successors = road->successors();
                    for (auto succ : road_successors)
                        map_builder_.laneSection_addSuccessor(lane_section.get(), succ->sections().front());
                } else {
                    map_builder_.laneSection_addSuccessor(
                        lane_section.get(), lane_sections.at(lane_section_counter + 1));
                    map_builder_.laneSection_addPredecessor(
                        lane_section.get(), lane_sections.at(lane_section_counter - 1));
                }
            } else {
                // road is part of a junction. Add last lane section of previous road as predecessor, first lane
                // section of subsequent road as successor
                auto lane_section = lane_sections.at(lane_section_counter);
                auto road_predecessors = road->predecessors();
                for (auto succ : road_predecessors)
                    map_builder_.laneSection_addPredecessor(lane_section.get(), succ->sections().back());
                auto road_successors = road->successors();
                for (auto succ : road_successors)
                    map_builder_.laneSection_addSuccessor(lane_section.get(), succ->sections().front());
            }
            lane_section_counter++;
        }
    }
}
void OpenDriveParser::parseLanes() {
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");
    for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
         odrRoad = odrRoad->NextSiblingElement("road")) {
        auto road = map_builder_.getRoad(odrRoad->UnsignedAttribute("id"));
        auto* odrLanes = odrRoad->FirstChildElement("lanes");
        //
        std::size_t lane_section_counter{0};
        for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection"); odrLaneSection != nullptr;
             odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
            auto lane_sections = road->sections();
            auto lane_section = lane_sections.at(lane_section_counter);

            auto* group_left = odrLaneSection->FirstChildElement("left");
            auto* group_right = odrLaneSection->FirstChildElement("right");
            if (group_left) parseLaneGroup(lane_section, group_left);
            if (group_right) parseLaneGroup(lane_section, group_right);

            lane_section_counter++;
        }
    }
}
void OpenDriveParser::laneConnections() {
    auto odr = xml_doc_.FirstChildElement("OpenDRIVE");
    // populate lane successors/predecessors
    for (auto* odrRoad = odr->FirstChildElement("road"); odrRoad != nullptr;
         odrRoad = odrRoad->NextSiblingElement("road")) {
        auto road = map_builder_.getRoad(odrRoad->UnsignedAttribute("id"));
        auto* odrLanes = odrRoad->FirstChildElement("lanes");
        std::size_t lane_section_counter{0};
        for (auto* odrLaneSection = odrLanes->FirstChildElement("laneSection"); odrLaneSection != nullptr;
             odrLaneSection = odrLaneSection->NextSiblingElement("laneSection")) {
            auto lane_sections = road->sections();
            auto lane_section = lane_sections.at(lane_section_counter);
            auto* group_left = odrLaneSection->FirstChildElement("left");
            auto* group_right = odrLaneSection->FirstChildElement("right");

            if (group_left) laneConnections(lane_section, group_left);
            if (group_right) laneConnections(lane_section, group_right);
            lane_section_counter++;
        }
    }
}

void OpenDriveParser::parseLaneGroup(std::shared_ptr<LaneSection> lane_section, const tinyxml2::XMLElement* group) {
    for (auto* odrLane = group->FirstChildElement("lane"); odrLane != nullptr;
         odrLane = odrLane->NextSiblingElement("lane")) {
        // add all lanes to the map
        auto lane = map_builder_.laneSection_addLane(
            lane_section, odrLane->IntAttribute("id"), odrLane->FirstChildElement("width")->DoubleAttribute("sOffset"),
            odrLane->FirstChildElement("width")->DoubleAttribute("a"), parseLaneType(odrLane->Attribute("type")));
        // calculate lane geometries
        calculateLaneBoundaryPoints(lane.get(), odrLane);
        calculateLanePoints(lane.get(), odrLane);
    }
}

void OpenDriveParser::laneConnections(std::shared_ptr<LaneSection> lane_section, const tinyxml2::XMLElement* group) {
    for (auto* odrLane = group->FirstChildElement("lane"); odrLane != nullptr;
         odrLane = odrLane->NextSiblingElement("lane")) {
        if (!strcmp(odrLane->Attribute("type"), "driving")) {  // TODO only driving Lanes
            auto lane = lane_section->lane(odrLane->UnsignedAttribute("id"));
            // auto road = map_builder_.getRoad(lane_section->road()->id());
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
                        map_builder_.lane_addPredecessor(lane.get(), elem->lane(odr_predecessor->IntAttribute("id")));
                    }
                } else {
                    // predecessor is a junction
                    auto lane_section_predecessors = lane_section->predecessors();
                    for (auto elem : lane_section_predecessors) {
                        // find the junction it belongs to
                        // auto junction = elem->road()->junction();
                        auto junction = map_builder_.getJunction(elem->road()->junction());

                        if (junction) {
                            // check if lane links contain current lane
                            for (auto conn : junction->connections()) {
                                if ((conn->incomingRoad() == lane_section->road()->id()) &&
                                    (conn->connectingRoad() == elem->road()->id())) {
                                    auto lls = conn->lane_links_;
                                    std::vector<LaneLink> res;
                                    std::copy_if(
                                        lls.begin(), lls.end(), std::back_inserter(res),
                                        [lane](const LaneLink& ll) { return ll.from_ == lane->id(); });
                                    for (auto link : res)
                                        map_builder_.lane_addPredecessor(lane.get(), elem->lane(link.to_));
                                }
                            }
                        }
                    }
                }
                auto odr_successor = link->FirstChildElement("successor");
                if (odr_successor) {
                    auto lane_section_successors = lane_section->successors();
                    for (auto elem : lane_section_successors) {
                        map_builder_.lane_addSuccessor(lane.get(), elem->lane(odr_successor->IntAttribute("id")));
                    }
                } else {
                    // check if predecessor is a junction
                    auto lane_section_successors = lane_section->successors();
                    for (auto elem : lane_section_successors) {
                        // find the junction it belongs to
                        // auto junction = elem->road()->junction();
                        auto junction = map_builder_.getJunction(elem->road()->junction());
                        // check if lane links contain current lane i
                        if (junction) {
                            for (auto conn : junction->connections()) {
                                if ((conn->incomingRoad() == lane_section->road()->id()) &&
                                    (conn->connectingRoad() == elem->road()->id())) {
                                    auto lls = conn->lane_links_;
                                    std::vector<LaneLink> res;
                                    std::copy_if(
                                        lls.begin(), lls.end(), std::back_inserter(res),
                                        [lane](const LaneLink& ll) { return ll.from_ == lane->id(); });
                                    for (auto link : res)
                                        map_builder_.lane_addSuccessor(lane.get(), elem->lane(link.to_));
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

                    auto junction = map_builder_.getJunction(elem->road()->junction());
                    // check if lane links contain current lane
                    if (junction) {
                        for (auto conn : junction->connections()) {
                            if ((conn->incomingRoad() == lane_section->road()->id()) &&
                                (conn->connectingRoad() == elem->road()->id())) {
                                auto lls = conn->lane_links_;
                                std::vector<LaneLink> res;
                                std::copy_if(
                                    lls.begin(), lls.end(), std::back_inserter(res),
                                    [lane](const LaneLink& ll) { return ll.from_ == lane->id(); });
                                for (auto link : res)
                                    map_builder_.lane_addPredecessor(lane.get(), elem->lane(link.to_));
                            }
                        }
                    }
                }
                auto lane_section_successors = lane_section->successors();
                for (auto elem : lane_section_successors) {
                    // find the junction it belongs to
                    // auto junction = elem->road()->junction();
                    auto junction = map_builder_.getJunction(elem->road()->junction());
                    // check if lane links contain current lane i
                    if (junction) {
                        for (auto conn : junction->connections()) {
                            if ((conn->incomingRoad() == lane_section->road()->id()) &&
                                (conn->connectingRoad() == elem->road()->id())) {
                                auto lls = conn->lane_links_;
                                std::vector<LaneLink> res;
                                std::copy_if(
                                    lls.begin(), lls.end(), std::back_inserter(res),
                                    [lane](const LaneLink& ll) { return ll.from_ == lane->id(); });
                                for (auto link : res) map_builder_.lane_addSuccessor(lane.get(), elem->lane(link.to_));
                            }
                        }
                    }
                }
            }
        }
    }
}

void OpenDriveParser::calculateLaneBoundaryPoints(Lane* lane, const tinyxml2::XMLElement* odrLane) {
    auto* odrRoad = odrLane->Parent()->Parent()->Parent()->Parent();

    auto plan_view = odrRoad->FirstChildElement("planView");
    auto offset = lane->width() * util::sgn(lane->id());
    for (auto* geom = plan_view->FirstChildElement("geometry"); geom != nullptr;
         geom = geom->NextSiblingElement("geometry")) {
        auto s = geom->DoubleAttribute("s");
        auto x = geom->DoubleAttribute("x");
        auto y = geom->DoubleAttribute("y");
        auto hdg = geom->DoubleAttribute("hdg");
        auto length = geom->DoubleAttribute("length");

        // line, spiral, arc, poly3, parampoly3
        auto arc = geom->FirstChildElement("arc");

        std::vector<Point> points;
        if (arc != nullptr)
            points = calculateArc(x, y, hdg, length, arc->DoubleAttribute("curvature"), offset);

        else
            points = calculateStraight(x, y, hdg, length, offset);
        map_builder_.lane_addLaneBoundaryPoints(lane, points);
    }
}

void OpenDriveParser::calculateLanePoints(Lane* lane, const tinyxml2::XMLElement* odrLane) {
    auto* odrRoad = odrLane->Parent()->Parent()->Parent()->Parent();

    auto plan_view = odrRoad->FirstChildElement("planView");
    auto offset = lane->width() * util::sgn(lane->id());
    offset = offset / 2;  // half lane width for lane center
    for (auto* geom = plan_view->FirstChildElement("geometry"); geom != nullptr;
         geom = geom->NextSiblingElement("geometry")) {
        auto s = geom->DoubleAttribute("s");
        auto x = geom->DoubleAttribute("x");
        auto y = geom->DoubleAttribute("y");
        auto hdg = geom->DoubleAttribute("hdg");
        auto length = geom->DoubleAttribute("length");

        // line, spiral, arc, poly3, parampoly3
        auto arc = geom->FirstChildElement("arc");

        std::vector<Point> points;
        if (arc != nullptr)
            points = calculateArc(x, y, hdg, length, arc->DoubleAttribute("curvature"), offset);

        else
            points = calculateStraight(x, y, hdg, length, offset);
        map_builder_.lane_addLanePoints(lane, points);
    }
}

std::vector<Point> OpenDriveParser::calculateStraight(double x, double y, double hdg, double length, double offset) {
    std::vector<Point> points;

    Point p_offset(-offset * std::sin(hdg), offset * std::cos(hdg), 0);  // TODO
    double x_offset = -offset * std::sin(hdg);
    double y_offset = offset * std::cos(hdg);

    points.emplace_back(Point(x + x_offset, y + y_offset));
    auto end = Point(x + length * std::cos(hdg) + x_offset, y + length * std::sin(hdg) + y_offset);

    double road_length{0};
    double step = 1.f;
    while (road_length < length) {
        Point next(((x += step * std::cos(hdg)) + x_offset), ((y += step * std::sin(hdg)) + y_offset));
        road_length += step;
        points.emplace_back(next);
    }
    points.emplace_back(end);
    return points;
}

std::vector<Point> OpenDriveParser::calculateArc(
    double x, double y, double hdg, double length, double arc, double offset) {
    std::vector<Point> points;
    double x_offset = -offset * std::sin(hdg);
    double y_offset = offset * std::cos(hdg);
    points.emplace_back(Point(x + x_offset, y + y_offset));

    auto x_m = x + 1 / arc * -std::sin(hdg);
    auto y_m = y + 1 / arc * std::cos(hdg);
    double road_length{0};
    double step = 0.3f;
    double start_angle = hdg - util::sgn(arc) * M_PI / 2;
    double end_angle = start_angle + length * arc;

    Point end(
        (x_m + (1 / std::abs(arc) + util::sgn(arc) * -offset) * std::cos(end_angle)),
        (y_m + (1 / std::abs(arc) + util::sgn(arc) * -offset) * std::sin(end_angle)));

    for (double angle = start_angle; util::sgn(arc) * angle < util::sgn(arc) * end_angle; angle += length * arc / 20) {
        Point next(
            (x_m + (1 / std::abs(arc) + util::sgn(arc) * -offset) * std::cos(angle)),
            (y_m + (1 / std::abs(arc) + util::sgn(arc) * -offset) * std::sin(angle)));
        points.emplace_back(next);
    }
    points.emplace_back(end);

    return points;
}

LaneType OpenDriveParser::parseLaneType(const char* lt) {
    if (!strcmp(lt, "sidewalk")) return LaneType::sidewalk;
    if (!strcmp(lt, "shoulder")) return LaneType::shoulder;
    if (!strcmp(lt, "driving")) return LaneType::driving;
    if (!strcmp(lt, "restricted")) return LaneType::restricted;
    if (!strcmp(lt, "median")) return LaneType::median;
    if (!strcmp(lt, "parking")) return LaneType::parking;
    if (!strcmp(lt, "none")) return LaneType::none;
    throw std::logic_error("unknown lane type " + std::string(lt));
}
}  // namespace parser