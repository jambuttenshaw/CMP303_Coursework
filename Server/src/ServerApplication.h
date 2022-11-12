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

	void ProcessConnect(const MessageHeader& header, sf::Packet& packet, const sf::IpAddress& ip, const unsigned short port);
	void ProcessDisconnect(const MessageHeader& header, sf::Packet& packet);

	void ProcessUpdate(const MessageHeader& header, sf::Packet& packet);

	Client& FindClientWithID(ClientID id);

private:
	sf::UdpSocket m_Socket;
	
	std::vector<Client> m_Clients;
	std::queue<ClientID> m_NextClientID;

	sf::Clock m_ServerClock;
};
