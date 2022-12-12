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

public:
	// allow the player to move without user input for testing purposes
	static void SettingsGUI();
	static bool AutomoveEnabled() { return s_EnableAutomove; }
	static sf::Vector2f Automove(float dt);

private:
	sf::RenderWindow& m_Window;

	static bool s_EnableAutomove;
};
