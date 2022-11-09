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
	return angle * 0.01745f;
}

float Clamp(float v, float min, float max)
{
	return std::max(std::min(v, max), min);
}

float Clamp01(float v)
{
	return Clamp(v, 0.0f, 1.0f);
}

float Lerp(float a, float b, float t)
{
	t = Clamp01(t);
	return a * (1.0f - t) + b * t;
}

sf::Vector2f Lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t)
{
	return sf::Vector2f(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t));
}
