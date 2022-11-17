#include "Connection.h"

#include "Log.h"


Connection::Connection()
{
	m_Socket.setBlocking(false);
}

Connection::~Connection()
{
	m_Socket.disconnect();
}

void Connection::OnTcpConnected(ClientID id, sf::Uint8 playerNum)
{
	m_ID = id;
	m_PlayerNumber = playerNum;
	m_TcpPort = m_Socket.getLocalPort();
	m_ClientIP = m_Socket.getRemoteAddress();
}

void Connection::SetUdpPort(unsigned short clientPort)
{
	m_UdpPort = clientPort;
}

void Connection::SendPacketTcp(sf::Packet& packet)
{
	sf::Socket::Status status;
	do
	{
		status = m_Socket.send(packet);
	} while (status == sf::Socket::Partial);

	if (status != sf::Socket::Done)
		LOG_ERROR("Error sending packet to client!");
}

void Connection::SendMessageTcp(MessageCode code)
{
	sf::Packet packet;
	MessageHeader header{ m_ID, code };
	packet << header;

	SendPacketTcp(packet);
}
