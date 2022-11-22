#include "NetworkPlayer.h"

#include "MathUtils.h"
#include "Log.h"

#include "imgui.h"


// static var definitions
bool NetworkPlayer::s_EnablePrediction = false;
bool NetworkPlayer::s_EnableInterpolation = false;
bool NetworkPlayer::s_ClampInterpolationParameter = false;
bool NetworkPlayer::s_EnableOutOfOrderChecks = false;


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

	if (m_StateQueue.size() == m_StateRecordDepth)
	{
		sf::Vector2f finalPos;
		float finalRot;

		StateRecord& state0 = m_StateQueue[0];
		StateRecord& state1 = m_StateQueue[1];
		StateRecord& state2 = m_StateQueue[2];
		
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

		sf::Vector2f v1, v2;
		if (s_EnablePrediction)
		{
			v1 = PredictPosition(state1, state2, simulationTime);
			v2 = PredictPosition(state0, state1, simulationTime);
		}
		else
		{
			v1 = state1.position;
			v2 = state0.position;
		}
		
		if (s_EnableInterpolation)
		{
			float interpolation = (simulationTime - state0.time) / (m_NextUpdateTime - state0.time);

			finalPos = s_ClampInterpolationParameter ? Lerp(v1, v2, interpolation) : LerpNoClamp(v1, v2, interpolation);
			finalRot = LerpAngleDegrees(state1.rotation, state0.rotation, interpolation);
		}
		else
		{
			finalPos = v2;
			finalRot = state0.rotation;
		}

		setPosition(finalPos);
		setRotation(finalRot);
	}
}

void NetworkPlayer::NetworkUpdate(const UpdateMessage& data, float timestamp)
{
	sf::Vector2f newPos{ data.x, data.y };

	if (m_StateQueue.size() < m_StateRecordDepth)
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



void NetworkPlayer::SettingsGUI()
{
	ImGui::Checkbox("Interpolation", &s_EnableInterpolation);
	ImGui::Checkbox("Clamp Interpolation Parameter", &s_ClampInterpolationParameter);
	ImGui::Checkbox("Prediction", &s_EnablePrediction);
	ImGui::Checkbox("Out of Order Checks", &s_EnableOutOfOrderChecks);
}