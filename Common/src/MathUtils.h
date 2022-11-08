#pragma once

#include <SFML/System.hpp>

float SqrLength(const sf::Vector2f& v);
float Length(const sf::Vector2f& v);

void Normalize(sf::Vector2f& v);
sf::Vector2f Normalized(const sf::Vector2f& v);

float RadToDeg(float angle);
float DegToRad(float angle);
