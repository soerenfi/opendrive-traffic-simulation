#include "renderer_sfml.hpp"

#include <chrono>
#include <iostream>
#include <thread>
//
#include "tsim_map.hpp"
#include "tsim_object.hpp"
#include "tsim_simulator.hpp"

Renderer::Renderer(tsim::Simulator* sim)
    : m_simulator(sim) {
  sf::ContextSettings settings;
  settings.antialiasingLevel = 16;

  m_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "TrafficSim",
                                                sf::Style::Default, settings);
  m_map = m_simulator->getMap();
  findMaxMinValues();
  m_scale = std::max((m_maxY - m_minY), (m_maxX - m_minX)) * 1.1f;

  sf::View view2;
  view2.setCenter(sf::Vector2f((std::abs(m_maxX) - std::abs(m_minX)) / 2, -(std::abs(m_maxY) - std::abs(m_minY)) / 2));
  view2.setSize(sf::Vector2f(m_scale * SCREEN_WIDTH / SCREEN_HEIGHT, m_scale));

  m_window->setView(view2);
}

void Renderer::findMaxMinValues() {
  for (const auto& road : m_map->roads()) {
    for (const auto& pt : road->points()) {
      if (pt.x > m_maxX) {
        m_maxX = pt.x;
      }
      if (pt.x < m_minX) {
        m_minX = pt.x;
      }
      if (pt.y > m_maxY) {
        m_maxY = pt.y;
      }
      if (pt.y < m_minY) {
        m_minY = pt.y;  // use std::sort?
      }
    }
  }
}

void Renderer::render() {
  auto step = std::chrono::milliseconds(1000 / 60);
  while (m_window->isOpen()) {
    auto now = std::chrono::steady_clock::now();
    auto target = now + step;
    sf::Event event;
    while (m_window->pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        m_window->close();
      }
      break;
    }
    m_window->clear(m_backgroundColor);

    m_objects = m_simulator->getObjects();

    drawLanes();
    drawVehicles();

    m_window->display();

    std::this_thread::sleep_until(target);
  }
}

void Renderer::drawLanes() {
  for (const auto& road : m_map->roads()) {
    for (const auto& section : road->sections()) {
      for (const auto& lane : section->lanes()) {
        if (lane->laneType() == tsim::LaneType::eDRIVING) {
          sf::VertexArray lines(sf::LineStrip, lane->boundaryPoints().size());
          std::size_t cnt{0};
          for (const auto& pt : lane->boundaryPoints()) {
            lines[cnt].position = sf::Vector2f(pt.x, -pt.y);
            cnt++;
          }
          m_window->draw(lines);
        }
      }
    }
  }
}

void Renderer::drawVehicles() const {
  sf::RectangleShape rectangle;
  float size = 2.0f;
  rectangle.setSize(sf::Vector2f(size, size));
  // rectangle.setFillColor(sf::Color::Red);
  // rectangle.setOutlineThickness(0);
  for (const auto object : m_objects) {
    rectangle.setPosition(object->getPosition().x - (size / 2), -object->getPosition().y - (size / 2));
    m_window->draw(rectangle);
  }
}