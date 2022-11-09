#include "Player.h"

Player::Player()
{
	setPointCount(3);
	setPoint(0, { 0, 0 });
	setPoint(1, { m_Size, 0.5f * m_Size });
	setPoint(2, { 0, m_Size });

	setOutlineThickness(-2.0f);
	setOutlineColor(sf::Color::Black);

	setOrigin(m_Size * 0.5f, m_Size * 0.5f);

	UpdatePlayerColor();
}

void Player::UpdatePlayerColor()
{
	setFillColor(ColourFromTeam(m_Team));
}
