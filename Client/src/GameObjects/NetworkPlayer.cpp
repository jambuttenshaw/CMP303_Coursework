#include "NetworkPlayer.h"

#include "MathUtils.h"


NetworkPlayer::NetworkPlayer(ClientID clientID)
	: m_ClientID(clientID)
{
}

NetworkPlayer::~NetworkPlayer()
{
}

void NetworkPlayer::Update(float simulationTime)
{
	// predict position
	if (m_StateQueue.size() >= 3)
	{
		StateRecord& state0 = m_StateQueue[0];
		StateRecord& state1 = m_StateQueue[1];
		StateRecord& state2 = m_StateQueue[2];

		float interpolation = (simulationTime - state0.time) / (m_NextUpdateTime - state0.time);

		//sf::Vector2f lerpedPos = Lerp(
		//	PredictPosition(state1, state2, m_NextUpdateTime),
		//	PredictPosition(state0, state1, m_NextUpdateTime),
		//	interpolation);
		sf::Vector2f lerpedPos = Lerp(
			state0.position,
			PredictPosition(state0, state1, m_NextUpdateTime),
			interpolation);
		//sf::Vector2f lerpedPos = Lerp(
		//	state1.position,
		//	state0.position,
		//	interpolation);

		setPosition(lerpedPos);
	}
}

void NetworkPlayer::NetworkUpdate(const UpdateMessage& data, float timestamp)
{
	sf::Vector2f newPos{ data.x, data.y };

	if (m_StateQueue.size() < 3)
	{
		setPosition(newPos);
	}
	setRotation(data.rotation);

	m_StateQueue.push_front({ newPos, data.rotation, timestamp });
	m_NextUpdateTime = timestamp + UPDATE_FREQUENCY;

	while (m_StateQueue.size() > m_StateRecordDepth) m_StateQueue.pop_back();
}

sf::Vector2f NetworkPlayer::PredictPosition(const StateRecord& state0, const StateRecord& state1, float currentSimTime)
{
	sf::Vector2f velocity = (state0.position - state1.position) / (state0.time - state1.time);
	return  state0.position + velocity * (currentSimTime - state0.time);
}
