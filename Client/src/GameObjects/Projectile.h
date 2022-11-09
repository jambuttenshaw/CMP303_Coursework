#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"
#include "Core/Colors.h"


class Projectile : public sf::CircleShape
{
public:
	Projectile(PlayerTeam team, const sf::Vector2f& pos, float direction);
	~Projectile() = default;

	void Update(float dt);

	bool OffScreen(const sf::Vector2f& screenSize) const;

	inline PlayerTeam GetTeam() const { return m_Team; }

private:
	const float m_Radius = 3.0f;
	const float m_Speed = 750.0f;

	PlayerTeam m_Team = PlayerTeam::None;
	sf::Vector2f m_Direciton{ 0.0f, 0.0f };
};
