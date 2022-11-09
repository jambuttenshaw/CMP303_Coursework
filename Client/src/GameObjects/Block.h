#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"


class Block : public sf::RectangleShape
{
public:
	Block(PlayerTeam team, const sf::Vector2f& position);
	~Block() = default;

	inline PlayerTeam GetTeam() const { return m_Team; }

private:
	const float m_Size = 20.0f;

	PlayerTeam m_Team = PlayerTeam::None;
};