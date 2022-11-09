#pragma once

#include <SFML/System.hpp>

float SqrLength(const sf::Vector2f& v);
float Length(const sf::Vector2f& v);

void Normalize(sf::Vector2f& v);
sf::Vector2f Normalized(const sf::Vector2f& v);

float RadToDeg(float angle);
float DegToRad(float angle);

float Clamp(float v, float min, float max);
float Clamp01(float v);

float Lerp(float a, float b, float t);
sf::Vector2f Lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t);
