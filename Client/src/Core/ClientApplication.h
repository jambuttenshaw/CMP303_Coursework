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

	void ChangeTurfLine(float turfLine);
	bool OnTeamTurf(const sf::Vector2f& pos, PlayerTeam team) const;


private:
	sf::RenderWindow m_Window;

	sf::Clock m_Clock;

	float m_FPS = 0.0f;
	float m_UpdateFPSTimer = 1.0f; // init value of 1 will calculate fps on first frame

	// Networking
	NetworkSystem m_NetworkSystem;

	GameState m_GameState = GameState::Lobby;

	ControllablePlayer m_Player;
	PlayerIndicator m_Indicator;
	std::vector<NetworkPlayer*> m_NetworkPlayers;

	float m_GUIWidth = 300.0f;
	sf::RectangleShape m_GUIBackground, m_RedBackground, m_BlueBackground;
	float m_TurfLine = 0.0f;

	std::vector<Projectile*> m_Projectiles;
	std::vector<Block*> m_Blocks;

	Block* m_GhostBlock = nullptr;
	sf::Vector2f m_LastPlaceLocation{ 0, 0 };
	// how many blocks the player has left to place
	unsigned int m_BuildModeBlocks = INITIAL_BUILD_MODE_BLOCKS;

	unsigned int m_Ammo = MAX_AMMO_HELD;
	float m_ReloadTimer = 0.0f;
};