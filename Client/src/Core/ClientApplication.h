#pragma once

#include <SFML/Graphics.hpp>

#include "Core/Colors.h"
#include "GameObjects/ControllablePlayer.h"
#include "GameObjects/PlayerIndicator.h"

#include "Network/NetworkSystem.h"

#include <vector>

class Projectile;
class Block;


class ClientApplication
{
	friend class NetworkSystem;
public:
	ClientApplication();
	~ClientApplication();

	void Run();

private:

	// the differene between handle input and update is that handle input is only called when the window is focused
	void HandleInput(float dt);
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

	// Networking
	NetworkSystem m_NetworkSystem;

	GameState m_GameState = GameState::BuildMode;

	ControllablePlayer m_Player;
	PlayerIndicator m_Indicator;
	std::vector<NetworkPlayer*> m_NetworkPlayers;

	sf::RectangleShape m_RedBackground, m_BlueBackground;
	float m_TurfLine = 0.0f;

	std::vector<Projectile*> m_Projectiles;
	std::vector<Block*> m_Blocks;

	Block* m_GhostBlock = nullptr;
	const float m_BlockPlaceRadius = 40.0f;
};