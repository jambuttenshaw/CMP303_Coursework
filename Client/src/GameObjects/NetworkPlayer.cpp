#include "NetworkPlayer.h"

#include "MathUtils.h"
#include "Log.h"


NetworkPlayer::NetworkPlayer(ClientID clientID)
	: m_ClientID(clientID), m_DebugLines(sf::LinesStrip, 2)
{
	m_DebugLines[0].color = sf::Color::Red;
	m_DebugLines[1].color = sf::Color::Red;
	//m_DebugLines[2].color = sf::Color::Red;
}

NetworkPlayer::~NetworkPlayer()
{
}

void NetworkPlayer::Update(float simulationTime)
{
	m_CurrentSimulationTime = simulationTime;
	// predict position
	if (m_StateQueue.size() >= 3)
	{
		StateRecord& state0 = m_StateQueue[0];
		StateRecord& state1 = m_StateQueue[1];
		StateRecord& state2 = m_StateQueue[2];

		float interpolation = (simulationTime - state0.time) / (m_NextUpdateTime - state0.time);

		// LATENCY ISNT NOTICEABLE BUT CHANGES IN DIRECTION LOOK TERRIBLE - interpolation isnt really helping
		//sf::Vector2f lerpedPos = Lerp(
		//	state0.position,
		//	PredictPosition(state0, state1, m_NextUpdateTime),
		//	interpolation);

		// LOOKS GOOD BUT LATENCY IS NOTICEABLE
		//sf::Vector2f lerpedPos = Lerp(
		//	state1.position,
		//	state0.position,
		//	interpolation);

		sf::Vector2f v1 = PredictPosition(state1, state2, simulationTime);
		sf::Vector2f v2 = PredictPosition(state0, state1, simulationTime);
		sf::Vector2f lerped = Lerp(v1, v2, interpolation);

		setPosition(lerped);

		setRotation(LerpAngleDegrees(state1.rotation, state0.rotation, interpolation));
	}
}

void NetworkPlayer::NetworkUpdate(const UpdateMessage& data, float timestamp)
{
	sf::Vector2f newPos{ data.x, data.y };

	if (m_StateQueue.size() < 3)
	{
		setPosition(newPos);
		setRotation(data.rotation);
	}

	m_StateQueue.push_front({ newPos, data.rotation, timestamp });
	m_NextUpdateTime = timestamp + UPDATE_FREQUENCY;

	while (m_StateQueue.size() > m_StateRecordDepth) m_StateQueue.pop_back();
}

void NetworkPlayer::RenderDebugLines(sf::RenderWindow& window)
{
	if (m_StateQueue.size() >= 2)
	{
		//m_DebugLines[0].position = m_StateQueue[0].position;
		m_DebugLines[0].position = m_StateQueue[0].position;
		m_DebugLines[1].position = PredictPosition(m_StateQueue[0], m_StateQueue[1], m_CurrentSimulationTime);

		window.draw(m_DebugLines);
	}
}

sf::Vector2f NetworkPlayer::PredictPosition(const StateRecord& state0, const StateRecord& state1, float currentSimTime)
{
	sf::Vector2f velocity = (state0.position - state1.position) / (state0.time - state1.time);
	return  state0.position + velocity * (currentSimTime - state0.time);
}
