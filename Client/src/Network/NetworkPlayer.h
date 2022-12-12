#pragma once

#include "GameObjects/Player.h"
#include "Network/NetworkTypes.h"

#include <deque>

// an object representing player objects controlled by other clients over the network
class NetworkPlayer : public Player
{
public:
	// a network player has a unqiue client id
	NetworkPlayer(ClientID clientID);
	virtual ~NetworkPlayer();

	inline ClientID GetID() const { return m_ClientID; }

	// update the player every frame
	void Update(float dt);
	// recieve an update of the players state over the network
	void NetworkUpdate(const UpdateMessage& data, float timestamp);

public:

	static void SettingsGUI();

private:
	// calculate a predicted position using two previous states and the current time
	sf::Vector2f PredictPosition(const PlayerStateFrame& state0, const PlayerStateFrame& state1, float simTime);

private:
	// a unique id assigned by the server
	const ClientID m_ClientID;

	float m_CurrentSimulationTime = 0.0f;
	float m_LastUpdateTime = 0.0f;
	float m_NextUpdateTime = 0.0f;

	// clients don't need to 'wind back time' to make decisions, they only use the history for interpolation
	const int m_MaxStateHistorySize = 3;
	std::deque<PlayerStateFrame> m_StateHistory;

private:

	static bool s_EnableInterpolation;
	static bool s_EnablePrediction;

};