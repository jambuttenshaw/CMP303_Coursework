#pragma once

#include "Player.h"
#include "Network/NetworkTypes.h"

#include <deque>


class NetworkPlayer : public Player
{
	struct StateRecord
	{
		sf::Vector2f position;
		float rotation;
		float time;
	};

public:
	NetworkPlayer(ClientID clientID);
	virtual ~NetworkPlayer();

	inline ClientID GetID() const { return m_ClientID; }

	void Update(float dt);
	void NetworkUpdate(const UpdateMessage& data, float timestamp);

private:

	sf::Vector2f PredictPosition(const StateRecord& state0, const StateRecord& state1, float currentSimTime);

private:
	const ClientID m_ClientID;

	float m_NextUpdateTime = 0.0f;

	const size_t m_StateRecordDepth = 3;
	std::deque<StateRecord> m_StateQueue;
};