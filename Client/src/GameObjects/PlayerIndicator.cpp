#include "PlayerIndicator.h"

#include "Core/Colors.h"


PlayerIndicator::PlayerIndicator()
{
	setPointCount(3);
	setPoint(0, { 0, 0 });
	setPoint(1, { -0.5f * m_Size, -m_Size });
	setPoint(2, { 0.5f * m_Size, -m_Size });

	setFillColor(PlayerIndicatorColor);

	setOutlineThickness(-1.0f);
	setOutlineColor(sf::Color::Black);

	setOrigin(0.0f, m_Offset);
}