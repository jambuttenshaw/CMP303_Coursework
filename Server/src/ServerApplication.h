#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include <queue>

#include "Network/NetworkTypes.h"



class ServerApplication
{
public:
	ServerApplication();
	~ServerApplication();

	void Run();

private:
	sf::UdpSocket m_Socket;
	
	std::vector<Client> m_Clients;
	std::queue<ClientID> m_NextClientID;

	sf::Clock m_ServerClock;
};
