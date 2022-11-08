#pragma once

#include <SFML/Graphics.hpp>

#include "Core/Constants.h"
#include "GameObjects/ControllablePlayer.h"


class Application
{
public:
	Application();
	~Application();

	void Run();

private:

	void Update(float dt);
	
	void Render();
	void GUI();

private:
	sf::RenderWindow m_Window;
	sf::Clock m_Clock;

	ControllablePlayer m_Player;

	sf::RectangleShape m_RedBackground, m_BlueBackground;
};