#pragma once

#include <SFML/Graphics.hpp>


class PlayerIndicator : public sf::ConvexShape
{
public:
	PlayerIndicator();
	virtual ~PlayerIndicator() = default;
	
	void Update();

	inline sf::Vector2f GetDimensions() const { return { m_Size, m_Size }; }

protected:
	const float m_Offset = 12.0f;
	const float m_Size = 12.0f;
};
