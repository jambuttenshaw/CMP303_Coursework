#pragma once

#include <SFML/Network.hpp>
#include "Network/NetworkTypes.h"
#include "Log.h"

#include "GameObjects/ControllablePlayer.h"

#include <vector>
#include <functional>

class ControllablePlayer;
class NetworkPlayer;
class Projectile;
class Block;


class NetworkSystem
{
	enum class ConnectionState
	{
		Disconnected,	// tcp is not connected
		Connecting,		// tcp is connected, waiting on client ID
		Connected		// tcp is connected and we have a client ID
	};

public:
	NetworkSystem();
	~NetworkSystem();

	void Init(	ControllablePlayer* player,
				std::vector<NetworkPlayer*>* networkPlayers,
				std::vector<Projectile*>* projectiles,
				std::vector<Block*>* blocks,
				GameState* gameState,
				std::function<void(float)> changeTurfLineFunc);
	void GUI();
	void Update(float dt);

	inline bool Connected() const { return (m_ClientID != INVALID_CLIENT_ID); }

	void Connect();
	void Disconnect();

	void RequestChangeTeam();

	void RequestShoot(const sf::Vector2f& position, const sf::Vector2f& direction);
	void RequestPlaceBlock(const sf::Vector2f& position);

	void SyncSimulationTime();

private:
	void ProcessIncomingUdp();
	void ProcessOutgoingUdp(float dt);
	void ProcessIncomingTcp();

	void SendPacketToServerTcp(sf::Packet& packet);
	void SendPacketToServerUdp(sf::Packet& packet);

	// callbacks from messages
	void OnConnect(const MessageHeader& header, sf::Packet& packet);
	void OnDisconnect(const MessageHeader& header, sf::Packet& packet);
	void OnOtherPlayerConnect(const MessageHeader& header, sf::Packet& packet);
	void OnOtherPlayerDisconnect(const MessageHeader& header, sf::Packet& packet);
	void OnRecieveUpdate(const MessageHeader& header, sf::Packet& packet);
	void OnPlayerChangeTeam(const MessageHeader& header, sf::Packet& packet);
	void OnServerTimeUpdate(const MessageHeader& header, sf::Packet& packet);
	void OnShoot(const MessageHeader& header, sf::Packet& packet);
	void OnProjectilesDestroyed(const MessageHeader& header, sf::Packet& packet);
	void OnPlace(const MessageHeader& header, sf::Packet& packet);
	void OnBlocksDestroyed(const MessageHeader& header, sf::Packet& packet);
	void OnChangeGameState(const MessageHeader& header, sf::Packet& packet);
	void OnTurfLineMoved(const MessageHeader& header, sf::Packet& packet);
	void OnPlayerDeath(const MessageHeader& header, sf::Packet&);

	inline MessageHeader CreateHeader(MessageCode messageCode) const { return MessageHeader{ m_ClientID, messageCode }; }

	NetworkPlayer* FindNetworkPlayerWithID(ClientID id);

	void GoToSpawn();

private:
	sf::TcpSocket m_TcpSocket;
	sf::UdpSocket m_UdpSocket;

	ConnectionState m_ConnectionState = ConnectionState::Disconnected;
	sf::Uint8 m_ClientID = INVALID_CLIENT_ID;

	// time since since simulation began: synchonized with the server
	float m_SimulationTime = 0.0f;
	float m_LatencyPingBegin = 0.0f;

	float m_UpdateTimer = 0.0f;

	float m_RemainingGameStateDuration = 0.0f;

	// pointers to game objects and game object containers
	ControllablePlayer* m_Player = nullptr;
	std::vector<NetworkPlayer*>* m_NetworkPlayers = nullptr;
	std::vector<Projectile*>* m_Projectiles = nullptr;
	std::vector<Block*>* m_Blocks = nullptr;
	GameState* m_GameState = nullptr;
	std::function<void(float)> m_ChangeTurfLineFunc;
};
