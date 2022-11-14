#pragma once

#include "Network\NetworkTypes.h"


class Connection
{
public:
	Connection();
	~Connection();

	sf::TcpSocket& GetSocket() { return m_Socket; }
	ClientID GetID() const { return m_ID; }
	const sf::IpAddress& GetIP() const { return m_ClientIP; }
	unsigned short GetTcpPort() const { return m_TcpPort; }
	unsigned short GetUdpPort() const { return m_UdpPort; }

	bool CanSendUdp() const { return m_UdpPort != (unsigned short)(-1); }

	PlayerState& GetPlayerState() { return m_PlayerState; }

	void OnTcpConnected(ClientID id);
	void SetUdpPort(unsigned short clientPort);

	void SendPacketTcp(sf::Packet& packet);
	template<typename T>
	void SendMessageTcp(MessageCode code, T& message, float time)
	{
		sf::Packet packet;
		MessageHeader header{ m_ID, code, time };
		packet << header << message;

		SendPacketTcp(packet);
	}

private:
	sf::TcpSocket m_Socket;

	// network properties
	ClientID m_ID = INVALID_CLIENT_ID;
	sf::IpAddress m_ClientIP = sf::IpAddress::None;
	unsigned short m_TcpPort = -1;
	unsigned short m_UdpPort = -1;

	// in-game player properties
	PlayerState m_PlayerState;
};
