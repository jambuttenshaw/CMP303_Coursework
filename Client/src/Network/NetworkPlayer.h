#pragma once

#include "GameObjects/Player.h"
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

	void RenderDebugLines(sf::RenderWindow& window);

public:

	static void SettingsGUI();

private:

	sf::Vector2f PredictPosition(const StateRecord& state0, const StateRecord& state1, float currentSimTime);

private:
	const ClientID m_ClientID;

	float m_NextUpdateTime = 0.0f;

	const size_t m_StateRecordDepth = 3;
	std::deque<StateRecord> m_StateQueue;

	// debug lines
	sf::VertexArray m_DebugLines;
	float m_CurrentSimulationTime = 0.0f;

private:

	static bool s_EnablePrediction;
	static bool s_EnableInterpolation;
	static bool s_ClampInterpolationParameter;
	static bool s_EnableOutOfOrderChecks;

};