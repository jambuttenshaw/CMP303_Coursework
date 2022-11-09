#include "Block.h"

#include "Core/Colors.h"


Block::Block(PlayerTeam team, const sf::Vector2f& position)
	: m_Team(team)
{
	setOutlineColor(sf::Color::Black);
	setOutlineThickness(-2.0f);

	setSize({ m_Size, m_Size });
	setOrigin({ 0.5f * m_Size, 0.5f * m_Size });

	setFillColor(DarkColourFromTeam(m_Team));

	setPosition(position);
}
