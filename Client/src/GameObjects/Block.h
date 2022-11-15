#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"


class Block : public sf::RectangleShape
{
public:
	Block(BlockID id, PlayerTeam team, const sf::Vector2f& position);
	~Block() = default;

	inline BlockID GetID() const { return m_BlockID; }
	inline PlayerTeam GetTeam() const { return m_Team; }

private:
	const BlockID m_BlockID;
	const PlayerTeam m_Team;
};
