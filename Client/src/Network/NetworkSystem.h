#pragma once

#include <SFML/Network.hpp>
#include "Network/NetworkTypes.h"
#include "Log.h"

#include <vector>
#include <queue>
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
				std::function<void(float)> changeTurfLineFunc,
				unsigned int* buildModeBlocks,
				unsigned int* ammo);
	void GUI();
	void Update(float dt);

	inline bool Connected() const { return (m_ClientID != INVALID_CLIENT_ID); }

	void Connect();
	void Disconnect();

	void RequestGameStart();
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
	void OnConnect					(const MessageHeader&, sf::Packet&);
	void OnDisconnect				();
	void OnOtherPlayerConnect		(sf::Packet&);
	void OnOtherPlayerDisconnect	(sf::Packet&);
	void OnRecieveUpdate			(sf::Packet&);
	void OnPlayerChangeTeam			(sf::Packet&);
	void OnServerTimeUpdate			(sf::Packet&);
	void OnShoot					(sf::Packet&);
	void OnProjectilesDestroyed		(sf::Packet&);
	void OnShootRequestDenied		();
	void OnPlace					(sf::Packet&);
	void OnBlocksDestroyed			(sf::Packet&);
	void OnPlaceRequestDenied		();
	void OnChangeGameState			(sf::Packet&);
	void OnTurfLineMoved			(sf::Packet&);
	void OnPlayerDeath				();
	void OnGameStart				();

	void SendPing();

	inline MessageHeader CreateHeader(MessageCode messageCode) const { return MessageHeader{ m_ClientID, messageCode }; }

	NetworkPlayer* FindNetworkPlayerWithID(ClientID id);

	void GoToSpawn();

private:
	sf::IpAddress m_ServerAddress = sf::IpAddress::None;
	unsigned short m_ServerPort = (unsigned short)(-1);

	sf::TcpSocket m_TcpSocket;
	sf::UdpSocket m_UdpSocket;

	ConnectionState m_ConnectionState = ConnectionState::Disconnected;
	ClientID m_ClientID = INVALID_CLIENT_ID;
	sf::Uint8 m_PlayerNumber = -1;

	// time since since simulation began: synchonized with the server
	float m_SimulationTime = 0.0f;
	float m_LatencyPingBegin = 0.0f;

	float m_UpdateTimer = 0.0f;
	float m_LastUpdateTime = 0.0f;

	float m_RemainingGameStateDuration = 0.0f;

	bool m_GameStartRequested = false;

	// pointers to game objects and game object containers
	ControllablePlayer* m_Player = nullptr;
	std::vector<NetworkPlayer*>* m_NetworkPlayers = nullptr;

	std::vector<Projectile*>* m_Projectiles = nullptr;
	// projectile requests are sent using tcp so we know request responses
	// will be received in the same order as the requests were sent
	// so we always process the local projectile at the front of the queue
	std::queue<Projectile*> m_LocalProjectiles;

	std::vector<Block*>* m_Blocks = nullptr;
	std::queue<Block*> m_LocalBlocks;

	GameState* m_GameState = nullptr;
	std::function<void(float)> m_ChangeTurfLineFunc;

	unsigned int* m_BuildModeBlocks = nullptr;
	unsigned int* m_Ammo = nullptr;

	// gui
	char m_GUIServerIP[16] = "127.0.0.1";
	int m_GUIServerPort = 4444;
};
