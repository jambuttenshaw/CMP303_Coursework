#include "NetworkPlayer.h"

#include "MathUtils.h"
#include "Log.h"

#include "imgui.h"


// static var definitions
bool NetworkPlayer::s_EnableInterpolation = true;
bool NetworkPlayer::s_EnablePrediction = false;


NetworkPlayer::NetworkPlayer(ClientID clientID)
	: m_ClientID(clientID)
{
}

NetworkPlayer::~NetworkPlayer()
{
}

void NetworkPlayer::Update(float simulationTime)
{
	// update simulation time
	m_CurrentSimulationTime = simulationTime;

	// check if we have recieved enough data to perform interpolation and/or prediction
	if (m_StateHistory.size() == m_MaxStateHistorySize)
	{
		sf::Vector2f finalPos;
		float finalRot;

		// get references to the previous frames
		PlayerStateFrame& state0 = m_StateHistory[0];
		PlayerStateFrame& state1 = m_StateHistory[1];
		PlayerStateFrame& state2 = m_StateHistory[2];

		sf::Vector2f v1, v2;
		if (s_EnablePrediction)
		{
			// get two predictions of the current position based off of the previous 3 frames
			// using linear prediction
			v1 = PredictPosition(state1, state2, simulationTime);
			v2 = PredictPosition(state0, state1, simulationTime);
		}
		else
		{
			// no prediction enabled: just use the previous two frames of data
			v1 = state1.position;
			v2 = state0.position;
		}
		
		if (s_EnableInterpolation)
		{
			// calculate the interpolation factor as how far we are between updates
			float interpolation = (simulationTime - m_LastUpdateTime) / (m_NextUpdateTime - m_LastUpdateTime);

			// lerp between frames/predicted positions
			finalPos = Lerp(v1, v2, interpolation);
			// we never predict rotation, only interpolate it and it looks and feels totally fine
			finalRot = LerpAngleDegrees(state1.rotation, state0.rotation, interpolation);
		}
		else
		{
			// no interpolation, just assign the position and rotation
			finalPos = v2;
			finalRot = state0.rotation;
		}

		// update game object
		setPosition(finalPos);
		setRotation(finalRot);
	}
}

void NetworkPlayer::NetworkUpdate(const UpdateMessage& data, float timestamp)
{
	// recieve a new update from over the network
	PlayerStateFrame newStateFrame{ data };
	// add to the history
	m_StateHistory.push_front(newStateFrame);

	// remove any out of data data
	while (m_StateHistory.size() > m_MaxStateHistorySize) m_StateHistory.pop_back();

	// update the timestamps
	m_LastUpdateTime = timestamp;
	// this is only a guess; but it is usually good enough to look fine under reasonable network conditions
	m_NextUpdateTime = timestamp + UPDATE_FREQUENCY;
}

sf::Vector2f NetworkPlayer::PredictPosition(const PlayerStateFrame& state0, const PlayerStateFrame& state1, float simTime)
{
	// perform linear prediction based off of two previous frames and the current simulation time
	sf::Vector2f velocity = (state0.position - state1.position) / state0.dt;
	return  state0.position + velocity * (simTime - m_LastUpdateTime);
}


void NetworkPlayer::SettingsGUI()
{
	ImGui::Checkbox("Interpolation", &s_EnableInterpolation);
	ImGui::Checkbox("Prediction", &s_EnablePrediction);
}