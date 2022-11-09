#pragma once

#include <SFML/Graphics.hpp>

#include "Core/Colors.h"
#include "GameObjects/ControllablePlayer.h"

#include <vector>

class Projectile;
class Block;


class ClientApplication
{
public:
	ClientApplication();
	~ClientApplication();

	void Run();

private:

	void Update(float dt);
	
	void Render();
	void GUI();

	bool CanPlaceBlock();
	void PlaceBlock();
	
	void TryFireProjectile();

	bool OnTeamTurf(const sf::Vector2f& pos, PlayerTeam team) const;

private:
	sf::RenderWindow m_Window;
	sf::Clock m_Clock;

	GameState m_GameState = GameState::BuildMode;

	ControllablePlayer m_Player;

	sf::RectangleShape m_RedBackground, m_BlueBackground;
	float m_TurfLine = 0.0f;

	std::vector<Projectile*> m_Projectiles;
	std::vector<Block*> m_Blocks;

	Block* m_GhostBlock = nullptr;
	const float m_BlockPlaceRadius = 40.0f;
};