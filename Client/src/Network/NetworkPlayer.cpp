#include "NetworkPlayer.h"

#include "MathUtils.h"
#include "Log.h"

#include "imgui.h"


// static var definitions
bool NetworkPlayer::s_EnableInterpolation = true;
bool NetworkPlayer::s_EnablePrediction = false;
bool NetworkPlayer::s_EnableOutOfOrderChecks = false;


NetworkPlayer::NetworkPlayer(ClientID clientID)
	: m_ClientID(clientID)
{
}

NetworkPlayer::~NetworkPlayer()
{
}

void NetworkPlayer::Update(float simulationTime)
{
	m_CurrentSimulationTime = simulationTime;

	if (m_StateHistory.size() == m_MaxStateHistorySize)
	{
		sf::Vector2f finalPos;
		float finalRot;

		PlayerStateFrame& state0 = m_StateHistory[0];
		PlayerStateFrame& state1 = m_StateHistory[1];
		PlayerStateFrame& state2 = m_StateHistory[2];

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
			float interpolation = (simulationTime - m_LastUpdateTime) / (m_NextUpdateTime - m_LastUpdateTime);

			finalPos = Lerp(v1, v2, interpolation);
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
	PlayerStateFrame newStateFrame{ data };
	m_StateHistory.push_front(newStateFrame);

	while (m_StateHistory.size() > m_MaxStateHistorySize) m_StateHistory.pop_back();

	m_LastUpdateTime = timestamp;
	m_NextUpdateTime = timestamp + UPDATE_FREQUENCY;
}

sf::Vector2f NetworkPlayer::PredictPosition(const PlayerStateFrame& state0, const PlayerStateFrame& state1, float simTime)
{
	sf::Vector2f velocity = (state0.position - state1.position) / state0.dt;
	return  state0.position + velocity * (simTime - m_LastUpdateTime);
}


void NetworkPlayer::SettingsGUI()
{
	ImGui::Checkbox("Interpolation", &s_EnableInterpolation);
	ImGui::Checkbox("Prediction", &s_EnablePrediction);
	ImGui::Checkbox("Out of Order Checks", &s_EnableOutOfOrderChecks);
}