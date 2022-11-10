#pragma once

#include <SFML/Network.hpp>


class ClientNetworkSystem
{
public:
	ClientNetworkSystem();
	~ClientNetworkSystem();

	void Update(float dt);

private:
	sf::UdpSocket m_ClientSocket;

	float m_Timer = 0.0f;

	// time since since simulation began: synchonized with the server
	float m_SimulationTime = 0.0f;
};
