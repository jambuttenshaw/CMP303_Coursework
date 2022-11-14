#pragma once

#include <SFML/Network.hpp>
#include "Network/NetworkTypes.h"
#include "Log.h"

#include "GameObjects/ControllablePlayer.h"
#include "GameObjects/NetworkPlayer.h"

#include <vector>

class ControllablePlayer;
class NetworkPlayer;


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

	void Init(ControllablePlayer* player, std::vector<NetworkPlayer*>* networkPlayers);

	void Update(float dt);

	inline bool Connected() const { return (m_ClientID != INVALID_CLIENT_ID); }

	void Connect();
	void Disconnect();

	void RequestChangeTeam();

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

	inline MessageHeader CreateHeader(MessageCode messageCode) const { return MessageHeader{ m_ClientID, messageCode, m_SimulationTime }; }

	NetworkPlayer* FindNetworkPlayerWithID(ClientID id);

private:
	sf::TcpSocket m_TcpSocket;
	sf::UdpSocket m_UdpSocket;

	ConnectionState m_ConnectionState = ConnectionState::Disconnected;
	sf::Uint8 m_ClientID = INVALID_CLIENT_ID;

	// time since since simulation began: synchonized with the server
	float m_SimulationTime = 0.0f;

	float m_UpdateTimer = 0.0f;

	ControllablePlayer* m_Player = nullptr;
	std::vector<NetworkPlayer*>* m_NetworkPlayers = nullptr;
};
