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

float LerpAngleDegrees(float a, float b, float t)
{
	float difference = fabsf(b - a);
	if (difference > 180)
	{
		// We need to add on to one of the values.
		if (b > a)
		{
			// We'll add it on to start...
			a += 360;
		}
		else
		{
			// Add it on to end.
			b += 360;
		}
	}

	// Interpolate it.
	float value = (a + ((b - a) * t));

	// note: value is not definitely in range [0, 360)
	return value;
}

float LerpNoClamp(float a, float b, float t)
{
	return a * (1.0f - t) + b * t;
}

sf::Vector2f LerpNoClamp(const sf::Vector2f& a, const sf::Vector2f& b, float t)
{
	return sf::Vector2f(LerpNoClamp(a.x, b.x, t), LerpNoClamp(a.y, b.y, t));
}

float Smoothstep(float edge0, float edge1, float x)
{
	if (x < edge0)
		return 0.0f;

	if (x >= edge1)
		return 1.0f;

	// Scale/bias into [0..1] range
	x = (x - edge0) / (edge1 - edge0);

	return x * x * (3.0f - 2.0f * x);
}
