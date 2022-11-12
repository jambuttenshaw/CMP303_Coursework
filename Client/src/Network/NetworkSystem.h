#pragma once

#include <SFML/Network.hpp>
#include "Network/NetworkTypes.h"

class NetworkSystem
{
public:
	NetworkSystem();
	~NetworkSystem();

	void Update(float dt);

	inline bool Connected() const { return (m_ClientID != INVALID_CLIENT_ID); }
	void Connect();
	void Disconnect();

private:
	void ProcessOutgoing(float dt);
	void ProcessIncoming();

	void SendPacketToServer(sf::Packet& packet, bool expectReply);
	void ResendLastPacketToServer();

	// callbacks from messages
	void OnConnect(const MessageHeader& header, sf::Packet& packet);
	void OnDisconnect(const MessageHeader& header, sf::Packet& packet);
	void OnOtherPlayerConnect(const MessageHeader& header, sf::Packet& packet);
	void OnOtherPlayerDisconnect(const MessageHeader& header, sf::Packet& packet);

	inline MessageHeader CreateHeader(MessageCode messageCode) const { return MessageHeader{ m_ClientID, messageCode, m_SimulationTime, m_Sequence }; }

private:
	sf::UdpSocket m_Socket;
	bool m_SocketBound = false;

	sf::Uint8 m_ClientID = INVALID_CLIENT_ID;

	sf::Uint32 m_Sequence = 0;

	// keep a copy of the last message in case it has to be re-sent
	sf::Packet m_LastMessage;
	bool m_WaitingOnReply = false;
	sf::Uint32 m_ReplySequence = 0;

	// time since since simulation began: synchonized with the server
	float m_SimulationTime = 0.0f;

	float m_ResendTimer = 0.0f;
	float m_UpdateTimer = 0.0f;
};
