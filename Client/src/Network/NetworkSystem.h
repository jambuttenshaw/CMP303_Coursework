#pragma once

#include <SFML/Network.hpp>
#include "Network/NetworkTypes.h"

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

	void Update(float dt);

	inline bool Connected() const { return (m_ClientID != INVALID_CLIENT_ID); }
	void Connect();
	void Disconnect();

private:
	void ProcessOutgoing(float dt);
	void ProcessIncomingTcp();
	void ProcessIncoming();

	void SendPacketToServer(sf::Packet& packet);
	//void SendPacketToServer(sf::Packet& packet, bool expectReply);
	//void ResendLastPacketToServer();

	// callbacks from messages
	void OnConnect(const MessageHeader& header, sf::Packet& packet);
	void OnDisconnect(const MessageHeader& header, sf::Packet& packet);
	void OnOtherPlayerConnect(const MessageHeader& header, sf::Packet& packet);
	void OnOtherPlayerDisconnect(const MessageHeader& header, sf::Packet& packet);

	inline MessageHeader CreateHeader(MessageCode messageCode) const { return MessageHeader{ m_ClientID, messageCode, m_SimulationTime }; }

private:
	sf::TcpSocket m_TcpSocket;
	sf::UdpSocket m_UdpSocket;

	ConnectionState m_ConnectionState = ConnectionState::Disconnected;
	sf::Uint8 m_ClientID = INVALID_CLIENT_ID;

	sf::Uint32 m_Sequence = 0;

	// time since since simulation began: synchonized with the server
	float m_SimulationTime = 0.0f;

	float m_UpdateTimer = 0.0f;
};
