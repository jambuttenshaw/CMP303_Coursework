#include "Connection.h"

#include "Log.h"
#include "MathUtils.h"


Connection::Connection()
{
	m_Socket.setBlocking(false);
}

Connection::~Connection()
{
	m_Socket.disconnect();
}

void Connection::AddToStateQueue(const UpdateMessage& updateMessage)
{
	PlayerStateFrame newStateFrame{ updateMessage };
	m_PlayerStateHistory.push_front(newStateFrame);

	float historyDuration = CalculateHistoryDuration();
	// work out how much extra history is currently stored
	float dt = std::max(0.0f, historyDuration - STATE_HISTORY_DURATION);

	while (m_PlayerStateHistory.size() > 1 && dt > 0)
	{
		PlayerStateFrame& oldest = m_PlayerStateHistory.back();
		if (dt >= oldest.dt)
		{
			// we can remove this entire frame
			dt -= oldest.dt;
			m_PlayerStateHistory.pop_back();
		}
		else
		{
			// we have to cut this frame short
			// assume the player travelled at a constant velocity during this frame
			float t = dt / oldest.dt;

			PlayerStateFrame& secondOldest = m_PlayerStateHistory[m_PlayerStateHistory.size() - 2];
			oldest.position = Lerp(oldest.position, secondOldest.position, t);
			oldest.rotation = LerpAngleDegrees(oldest.rotation, secondOldest.rotation, t);
			break;
		}
	}
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

float Connection::CalculateHistoryDuration()
{
	float duration = 0.0f;
	for (auto& frame : m_PlayerStateHistory) duration += frame.dt;
	return duration;
}
