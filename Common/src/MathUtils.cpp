#include "MathUtils.h"

#include <cmath>


float SqrLength(const sf::Vector2f& v)
{
	return v.x * v.x + v.y * v.y;
}

float Length(const sf::Vector2f& v)
{
	return std::sqrtf(SqrLength(v));
}

void Normalize(sf::Vector2f& v)
{
	float length = Length(v);
	if (length > 0.0f)
	{
		v.x /= length;
		v.y /= length;
	}
}

sf::Vector2f Normalized(const sf::Vector2f& v)
{
	float length = Length(v);
	if (length > 0.0f)
		return { v.x / length, v.y / length };
	
	return v;
}

float RadToDeg(float angle)
{
	return angle * 57.2958f;
}

float DegToRad(float angle)
{
	return angle * 0.01745;
}
