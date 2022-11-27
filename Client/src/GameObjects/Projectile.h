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

	// the id will be updated with the server-assigned value when the server confims the shoot request
	// any projectiles with INVALID_PROJECTILE_ID are local-only projectiles
	inline void UpdateID(ProjectileID id) { m_ProjectileID = id; }
	inline ProjectileID GetID() const { return m_ProjectileID; }
	inline PlayerTeam GetTeam() const { return m_Team; }

private:
	ProjectileID m_ProjectileID = INVALID_PROJECTILE_ID;
	const PlayerTeam m_Team;
	const sf::Vector2f m_Direciton;
};
