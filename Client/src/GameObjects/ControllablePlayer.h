#pragma once

#include "Player.h"

#include <vector>


class ControllablePlayer : public Player
{
public:
	ControllablePlayer(sf::RenderWindow& window);
	virtual ~ControllablePlayer();

	sf::Vector2f CalculateMovement(float dt);
	void UpdateRotation();

private:
	const float m_MoveSpeed = 150.0f;

	sf::RenderWindow& m_Window;
};
