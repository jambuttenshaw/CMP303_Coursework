#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"

#include "Core/Colors.h"


class Player : public sf::ConvexShape
{
public:
	Player();
	virtual ~Player() = default;

	inline void SetTeam(PlayerTeam team) { m_Team = team; UpdatePlayerColor(); }
	inline const PlayerTeam& GetTeam() const { return m_Team; }

private:

	void UpdatePlayerColor();

protected:
	PlayerTeam m_Team = PlayerTeam::None;
};
