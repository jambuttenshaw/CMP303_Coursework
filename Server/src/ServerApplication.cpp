#include "ServerApplication.h"

#include "Log.h"
#include "Network/NetworkTypes.h"


ServerApplication::ServerApplication()
{
	// set up server socket
	m_Socket.bind(ServerPort);
	m_Socket.setBlocking(false);

	for (ClientID id = 0; id < INVALID_CLIENT_ID; id++)
		m_NextClientID.push(id);

	m_Clients.reserve(MaxNumClients);
}

ServerApplication::~ServerApplication()
{
}

void ServerApplication::Run()
{
	m_ServerClock.restart();

	while (true)
	{
		// try to recieve data
		sf::Packet packet;
		sf::Packet outgoingData;
		sf::IpAddress incomingAddress;
		unsigned short incomingPort;

		sf::Socket::Status status = m_Socket.receive(packet, incomingAddress, incomingPort);
		if (status == sf::Socket::Done)
		{
			// data was recieved
			LOG_TRACE("Message recieved");
			
			MessageHeader header;
			header.Extract(packet);

			switch (header.messageCode)
			{
			case MessageCode::Connect:
			{				
				ClientID newClientID;
				if (m_Clients.size() < MaxNumClients)
				{
					newClientID = m_NextClientID.front();
					m_NextClientID.pop();
				}
				else
					// reject this clients connection
					newClientID = INVALID_CLIENT_ID;

				MessageHeader h{ newClientID, MessageCode::Connect, m_ServerClock.getElapsedTime().asSeconds(), header.sequence };
				sf::Packet response = h.Create();

				m_Clients.push_back({ newClientID, incomingAddress, incomingPort });

				m_Socket.send(response, incomingAddress, incomingPort);
				break;
			}
			case MessageCode::PlayerConnected:
				break;
			case MessageCode::Disconnect:
			{
				MessageHeader h{ header.clientID, MessageCode::Disconnect, m_ServerClock.getElapsedTime().asSeconds(), header.sequence };
				sf::Packet response = h.Create();
				
				m_NextClientID.push(header.clientID);

				m_Socket.send(response, incomingAddress, incomingPort);
				break;
			}
			case MessageCode::PlayerDisconnected:
				break;
			case MessageCode::Update:
				break;
			case MessageCode::ShootRequest:
				break;
			case MessageCode::PlaceRequest:
				break;
			case MessageCode::ShootRequestDenied:
				break;
			case MessageCode::PlaceRequestDenied:
				break;
			case MessageCode::Shoot:
				break;
			case MessageCode::Place:
				break;
			case MessageCode::PlayerDeath:
				break;
			default:
				break;
			}
		}
	}

}
