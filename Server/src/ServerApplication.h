#pragma once

#include <SFML/Network.hpp>
#include <vector>
#include <queue>

#include "Network/NetworkTypes.h"
#include "Connection.h"
#include "Log.h"



class ServerApplication
{
public:
	ServerApplication();
	~ServerApplication();

	void Run();

private:

	void ProcessConnect();
	void ProcessIntroduction(Connection* client, const MessageHeader& header, sf::Packet& packet);
	void ProcessDisconnect(Connection* client, const MessageHeader& header, sf::Packet& packet);
	void ProcessUpdate(Connection* client, const MessageHeader& header, sf::Packet& packet);
	void ProcessChangeTeam(Connection* client, const MessageHeader& header, sf::Packet& packet);

	Connection* FindClientWithID(ClientID id);

	template<typename T>
	void SendMessageToClientUdp(Connection* client, MessageCode code, T& message)
	{
		if (!client->CanSendUdp()) return;

		sf::Packet packet;
		MessageHeader header{ client->GetID(), code, m_ServerClock.getElapsedTime().asSeconds() };
		packet << header << message;

		auto status = m_UdpSocket.send(packet, client->GetIP(), client->GetUdpPort());
		if (status != sf::Socket::Done)
		{
			LOG_ERROR("Failed to send udp packet to client ID: {}", client->GetID());
		}
	}

private:
	sf::TcpListener m_ListenSocket;
	sf::UdpSocket m_UdpSocket;
	
	std::vector<Connection*> m_Clients;
	Connection* m_NewConnection = nullptr;

	std::queue<ClientID> m_NextClientID;

	sf::Clock m_ServerClock;

	// gameplay
	unsigned int m_RedTeamPlayerCount = 0, m_BlueTeamPlayerCount = 0;
};
