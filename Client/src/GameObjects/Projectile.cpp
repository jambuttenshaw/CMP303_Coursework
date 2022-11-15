#include "Projectile.h"

#include "MathUtils.h"
#include "Constants.h"


Projectile::Projectile(ProjectileID id, PlayerTeam team, const sf::Vector2f& pos, float direction)
	: Projectile(id, team, pos, { cosf(DegToRad(direction)), sinf(DegToRad(direction)) })
{
}

Projectile::Projectile(ProjectileID id, PlayerTeam team, const sf::Vector2f& pos, const sf::Vector2f& direction)
	: m_ProjectileID(id), m_Team(team), m_Direciton(direction)
{
	setRadius(PROJECTILE_RADIUS);
	setOrigin(0.5f * PROJECTILE_RADIUS, 0.5f * PROJECTILE_RADIUS);

	setOutlineColor(sf::Color::Black);
	setOutlineThickness(1.0f);
	setFillColor(DarkColourFromTeam(team));

	setPosition(pos);
}

void Projectile::Update(float dt)
{
	auto pos = getPosition();
	pos += m_Direciton * PROJECTILE_MOVE_SPEED * dt;
	setPosition(pos);
}
