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

sf::Vector2f Connection::GetPastPlayerPos(float t)
{
	assert(t < STATE_HISTORY_DURATION && "Can't see that far into the past");

	// find out which two states in the history t is between
	float t0 = 0.0f;
	size_t frameIndex = 0;
	for (auto& frame : m_PlayerStateHistory)
	{
		// does the next frame take us too far back
		if (t0 + frame.dt > t) break;

		t0 += frame.dt;
		frameIndex++;
	}

	// now we need to interpolate between frames in the history
	PlayerStateFrame& frame0 = m_PlayerStateHistory[frameIndex];
	PlayerStateFrame& frame1 = m_PlayerStateHistory[frameIndex + 1];

	// how much to interpolate?
	float interpolation = (t - t0) / frame0.dt;
	return Lerp(frame0.position, frame1.position, interpolation);
}

void Connection::AddToStateQueue(const UpdateMessage& updateMessage)
{
	PlayerStateFrame newStateFrame{ updateMessage };
	auto it = m_PlayerStateHistory.begin();
	for (; it != m_PlayerStateHistory.end(); it++)
	{
		PlayerStateFrame& f = *it;
		if (newStateFrame.sendTimestamp > f.sendTimestamp)
		{
			it = m_PlayerStateHistory.insert(it, newStateFrame);
			break;
		}
	}
	if (it == m_PlayerStateHistory.end())
		m_PlayerStateHistory.push_back(newStateFrame);

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
