#include <chrono>
#include <iostream>
#include <thread>

#include "renderer_sfml.hpp"
//
#include "tsim_map.hpp"
#include "tsim_object.hpp"
#include "tsim_simulator.hpp"

Renderer::Renderer(tsim::Simulator* sim) : simulator_(sim) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;

    window_ = std::make_unique<sf::RenderWindow>(
        sf::VideoMode(kScreenWidth, kScreenHeight), "TrafficSim", sf::Style::Default, settings);
    map_ = simulator_->map();
    findMaxMinValues();
    scale_ = std::max((max_y - min_y), (max_x - min_x)) * 1.1f;

    sf::View view2;
    view2.setCenter(sf::Vector2f((std::abs(max_x) - std::abs(min_x)) / 2, -(std::abs(max_y) - std::abs(min_y)) / 2));
    view2.setSize(sf::Vector2f(scale_ * kScreenWidth / kScreenHeight, scale_));

    window_->setView(view2);
}

void Renderer::findMaxMinValues() {
    for (const auto& road : map_->roads()) {
        for (const auto& pt : road->points()) {
            if (pt.x() > max_x) max_x = pt.x();
            if (pt.x() < min_x) min_x = pt.x();
            if (pt.y() > max_y) max_y = pt.y();
            if (pt.y() < min_y) min_y = pt.y();  // use std::sort?
        }
    }
}

void Renderer::render() {
    auto step = std::chrono::milliseconds(1000 / 60);
    while (window_->isOpen()) {
        auto now = std::chrono::steady_clock::now();
        auto target = now + step;
        sf::Event Event;
        while (window_->pollEvent(Event)) {
            if (Event.type == sf::Event::Closed) window_->close();
            break;
        }
        window_->clear(background_color_);

        objects_ = simulator_->objects();

        drawLanes();
        drawVehicles();

        window_->display();

        std::this_thread::sleep_until(target);
    }
}

void Renderer::drawLanes() {
    for (const auto& road : map_->roads()) {
        for (const auto& section : road->sections()) {
            for (const auto& lane : section->lanes()) {
                if (lane->laneType() == tsim::LaneType::driving) {
                    sf::VertexArray lines(sf::LineStrip, lane->boundaryPoints().size());
                    std::size_t ct{0};
                    for (const auto& pt : lane->boundaryPoints()) {
                        lines[ct].position = sf::Vector2f(pt.x(), -pt.y());
                        ct++;
                    }
                    window_->draw(lines);
                }
            }
        }
    }
}

void Renderer::drawVehicles() {
    sf::RectangleShape rectangle;
    float size = 2.0f;
    rectangle.setSize(sf::Vector2f(size, size));
    // rectangle.setFillColor(sf::Color::Red);
    // rectangle.setOutlineThickness(0);
    for (const auto object : objects_) {
        rectangle.setPosition(object->position().x() - (size / 2), -object->position().y() - (size / 2));
        window_->draw(rectangle);
    }
}