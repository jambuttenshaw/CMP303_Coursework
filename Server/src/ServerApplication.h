#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include <queue>

#include "Network/NetworkTypes.h"
#include "GameObjects.h"
#include "Connection.h"
#include "Log.h"



class ServerApplication
{
public:
	ServerApplication();
	~ServerApplication();

	void Run();

private:
	// process executed every iteration of the update loop
	void SimulateGameObjects(float dt);
	void UpdateGameState(float dt);

	// begin and end the game
	void StartGame();
	void EndGame();

	void DestroyProjectile(ProjectileState* projectile);
	void DestroyBlock(BlockState* block);

	// callbacks for messages
	void ProcessConnect();
	void ProcessIntroduction(Connection* client, sf::Packet& packet);
	void ProcessDisconnect(Connection* client);
	void ProcessUpdate(Connection* client, sf::Packet& packet);
	void ProcessChangeTeam(Connection* client);
	void ProcessGetServerTime(Connection* client);
	void ProcessShootRequest(Connection* client, sf::Packet& packet);
	void ProcessPlaceRequest(Connection* client, sf::Packet& packet);
	void ProcessGameStartRequest(Connection* client);

	Connection* FindClientWithID(ClientID id);

	// send via udp
	template<typename T>
	void SendMessageToClientUdp(Connection* client, MessageCode code, T& message)
	{
		if (!client->CanSendUdp()) return;

		sf::Packet packet;
		MessageHeader header{ client->GetID(), code };
		packet << header << message;

		auto status = m_UdpSocket.send(packet, client->GetIP(), client->GetUdpPort());
		if (status != sf::Socket::Done)
			LOG_ERROR("Failed to send udp packet to client ID: {}", client->GetID());
	}
	void SendMessageToClientUdp(Connection* client, MessageCode code)
	{
		if (!client->CanSendUdp()) return;

		sf::Packet packet;
		MessageHeader header{ client->GetID(), code };
		packet << header;

		auto status = m_UdpSocket.send(packet, client->GetIP(), client->GetUdpPort());
		if (status != sf::Socket::Done)
			LOG_ERROR("Failed to send udp packet to client ID: {}", client->GetID());
	}

	// get the next id in the queue
	ClientID NextClientID();
	ProjectileID NextProjectileID();
	BlockID NextBlockID();

	bool VerifyProjecitleShoot(const sf::Vector2f& position, const PlayerStateFrame& player);
	bool VerifyBlockPlacement(const sf::Vector2f& position, const PlayerStateFrame& player, PlayerTeam team);

	bool OnTeamTurf(const sf::Vector2f& p, PlayerTeam team);
	
	void CheckForBlocksAcrossTurfLine();

private:
	// the servers sockets
	sf::TcpListener m_ListenSocket;
	sf::UdpSocket m_UdpSocket;
	
	// all connected clients
	std::vector<Connection*> m_Clients;
	Connection* m_NewConnection = nullptr;

	// a queue is used to recycle client ID's
	std::queue<ClientID> m_NextClientID;
	ProjectileID m_NextProjectileID = 0;
	BlockID m_NextBlockID = 0;

	// clock and timers
	sf::Clock m_ServerClock;
	float m_SimulationTime = 0.0f;
	float m_UpdateTimer = 0.0f;
	float m_PingTimer = 0.0f;

	// gameplay
	unsigned int m_RedTeamPlayerCount = 0, m_BlueTeamPlayerCount = 0;

	GameState m_GameState = GameState::Lobby;
	float m_BuildModeDuration = INITIAL_BUILD_MODE_DURATION;
	float m_FightModeDuration = INITIAL_FIGHT_MODE_DURATION;
	float m_StateTimer = 0.0f;
	float m_StateDuration = 0.0f;
	int m_RoundNum = 0;

	// turf line starts at halfway
	float m_TurfLine = 0.5f * WORLD_WIDTH;

	// objects simulated by the server
	std::vector<ProjectileState*> m_Projectiles;
	std::vector<BlockState*> m_Blocks;
};
