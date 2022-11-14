#pragma once

#include "Player.h"
#include "Network/NetworkTypes.h"

#include <deque>


class NetworkPlayer : public Player
{
public:
	NetworkPlayer(ClientID clientID);
	virtual ~NetworkPlayer();

	inline ClientID GetID() const { return m_ClientID; }

	void Update(float dt);
	void NetworkUpdate(const UpdateMessage& data, float timestamp);

private:
	const ClientID m_ClientID;


	struct PositionRecord
	{
		sf::Vector2f position;
		float time;
	};
	const size_t m_PositionRecordDepth = 2;
	std::deque<PositionRecord> m_PositionQueue;
};