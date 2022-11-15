#include "Block.h"

#include "Core/Colors.h"
#include "Constants.h"


Block::Block(BlockID id, PlayerTeam team, const sf::Vector2f& position)
	: m_BlockID(id), m_Team(team)
{
	setOutlineColor(sf::Color::Black);
	setOutlineThickness(-2.0f);

	setSize({ BLOCK_SIZE, BLOCK_SIZE});
	setOrigin({ 0.5f * BLOCK_SIZE, 0.5f * BLOCK_SIZE });

	setFillColor(DarkColourFromTeam(m_Team));

	setPosition(position);
}
