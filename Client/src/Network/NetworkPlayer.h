#pragma once

#include "GameObjects/Player.h"
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

public:

	static void SettingsGUI();

private:

	sf::Vector2f PredictPosition(const PlayerStateFrame& state0, const PlayerStateFrame& state1, float simTime);

private:
	const ClientID m_ClientID;

	float m_CurrentSimulationTime = 0.0f;
	float m_LastUpdateTime = 0.0f;
	float m_NextUpdateTime = 0.0f;

	// clients don't need to 'wind back time', they only use the history for interpolation
	const int m_MaxStateHistorySize = 3;
	std::deque<PlayerStateFrame> m_StateHistory;

private:

	static bool s_EnableInterpolation;
	static bool s_EnablePrediction;
	static bool s_EnableOutOfOrderChecks;

};