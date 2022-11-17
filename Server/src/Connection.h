#pragma once

#include "Network\NetworkTypes.h"


class Connection
{
public:
	Connection();
	~Connection();

	sf::TcpSocket& GetSocket() { return m_Socket; }
	ClientID GetID() const { return m_ID; }
	sf::Uint8 GetPlayerNumber() const { return m_PlayerNumber; }
	const sf::IpAddress& GetIP() const { return m_ClientIP; }
	unsigned short GetTcpPort() const { return m_TcpPort; }
	unsigned short GetUdpPort() const { return m_UdpPort; }

	bool CanSendUdp() const { return m_UdpPort != (unsigned short)(-1); }

	PlayerState& GetPlayerState() { return m_PlayerState; }

	bool IsReady() const { return m_Ready; }
	void SetReady(bool ready) { m_Ready = ready; }

	void OnTcpConnected(ClientID id, sf::Uint8 playerNum);
	void SetUdpPort(unsigned short clientPort);

	void SendPacketTcp(sf::Packet& packet);
	void SendMessageTcp(MessageCode code);
	template<typename T>
	void SendMessageTcp(MessageCode code, T& message)
	{
		sf::Packet packet;
		MessageHeader header{ m_ID, code };
		packet << header << message;

		SendPacketTcp(packet);
	}

private:
	sf::TcpSocket m_Socket;

	// network properties
	ClientID m_ID = INVALID_CLIENT_ID;
	sf::Uint8 m_PlayerNumber = -1;

	sf::IpAddress m_ClientIP = sf::IpAddress::None;
	unsigned short m_TcpPort = -1;
	unsigned short m_UdpPort = -1;

	// in-game player properties
	PlayerState m_PlayerState;

	// ready for game to start
	bool m_Ready = false;
};
