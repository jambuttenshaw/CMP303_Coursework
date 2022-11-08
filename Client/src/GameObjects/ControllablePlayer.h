#pragma once

#include "Player.h"


class ControllablePlayer : public Player
{
public:
	ControllablePlayer(const sf::Window& window);
	virtual ~ControllablePlayer() = default;

	void Update(float dt);

private:
	const float m_MoveSpeed = 150.0f;
	const sf::Window& m_Window;

};
