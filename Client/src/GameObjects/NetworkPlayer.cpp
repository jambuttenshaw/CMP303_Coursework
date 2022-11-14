#include "NetworkPlayer.h"

#include "MathUtils.h"


NetworkPlayer::NetworkPlayer(ClientID clientID)
	: m_ClientID(clientID)
{
}

NetworkPlayer::~NetworkPlayer()
{
}

void NetworkPlayer::Update(float dt)
{
	// lerp to current position
	if (m_PositionQueue.size())
	{
		sf::Vector2f lerpedPos = Lerp(getPosition(), m_PositionQueue.front().position, 0.2f);
		setPosition(lerpedPos);
	}
}

void NetworkPlayer::NetworkUpdate(const UpdateMessage& data, float timestamp)
{
	sf::Vector2f newPos{ data.x, data.y };

	setRotation(data.rotation);
	if (m_PositionQueue.empty()) setPosition(newPos);

	m_PositionQueue.push_front({ newPos, timestamp });
	while (m_PositionQueue.size() > m_PositionRecordDepth) m_PositionQueue.pop_back();

}
