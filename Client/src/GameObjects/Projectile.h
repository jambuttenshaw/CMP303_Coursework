#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"
#include "Core/Colors.h"


class Projectile : public sf::CircleShape
{
public:
	Projectile(ProjectileID id, PlayerTeam team, const sf::Vector2f& pos, float direction);
	Projectile(ProjectileID id, PlayerTeam team, const sf::Vector2f& pos, const sf::Vector2f& direction);
	~Projectile() = default;

	void Update(float dt);

	inline ProjectileID GetID() const { return m_ProjectileID; }
	inline PlayerTeam GetTeam() const { return m_Team; }

private:
	const ProjectileID m_ProjectileID;
	const PlayerTeam m_Team;
	const sf::Vector2f m_Direciton;
};
