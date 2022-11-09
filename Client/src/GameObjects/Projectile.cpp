#include "Projectile.h"

#include "MathUtils.h"


Projectile::Projectile(PlayerTeam team, const sf::Vector2f& pos, float direction)
	: m_Team(team)
{
	setRadius(m_Radius);
	setOutlineColor(sf::Color::Black);
	setOutlineThickness(1.0f);
	setFillColor(DarkColourFromTeam(team));

	setPosition(pos);

	float directionRads = DegToRad(direction);
	m_Direciton = { cosf(directionRads), sinf(directionRads) };
}

void Projectile::Update(float dt)
{
	auto pos = getPosition();
	pos += m_Direciton * m_Speed * dt;
	setPosition(pos);
}

bool Projectile::OffScreen(const sf::Vector2f& screenSize) const
{
	auto pos = getPosition();

	if (pos.x + 2.0f * m_Radius < 0.0f || pos.x > screenSize.x)
		return true;
	if (pos.y + 2.0f * m_Radius < 0.0f || pos.y > screenSize.y)
		return true;

	return false;
}
