#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"

#include "Core/Constants.h"


class Player : public sf::ConvexShape
{
public:
	Player();
	virtual ~Player() = default;

	inline void SetTeam(PlayerTeam team) { m_Team = team; UpdatePlayerColor(); }
	inline const PlayerTeam& GetTeam() const { return m_Team; }

	inline const sf::Vector2f& GetDimensions() const { return { m_Size, m_Size }; }

private:

	void UpdatePlayerColor();

protected:
	const float m_Size = 30.0f;

	PlayerTeam m_Team = PlayerTeam::None;
};
