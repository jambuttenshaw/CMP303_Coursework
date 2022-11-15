#include "Player.h"

#include "Constants.h"


Player::Player()
{
	setPointCount(3);
	setPoint(0, { 0, 0 });
	setPoint(1, { PLAYER_SIZE, 0.5f * PLAYER_SIZE });
	setPoint(2, { 0, PLAYER_SIZE });

	setOutlineThickness(-2.0f);
	setOutlineColor(sf::Color::Black);

	setOrigin(PLAYER_SIZE * 0.5f, PLAYER_SIZE * 0.5f);

	UpdatePlayerColor();
}

void Player::UpdatePlayerColor()
{
	setFillColor(ColourFromTeam(m_Team));
}
