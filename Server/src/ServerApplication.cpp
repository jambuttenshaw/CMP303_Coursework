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

	m_Clients.reserve(MAX_NUM_PLAYERS);
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
		sf::IpAddress fromAddr;
		unsigned short fromPort;

		sf::Socket::Status status = m_Socket.receive(packet, fromAddr, fromPort);
		if (status == sf::Socket::Done)
		{
			// data was recieved
			MessageHeader header;
			packet >> header;

			switch (header.messageCode)
			{
			case MessageCode::Connect:				ProcessConnect(header, packet, fromAddr, fromPort); break;
			case MessageCode::Disconnect:			ProcessDisconnect(header, packet); break;
			case MessageCode::Update:				ProcessUpdate(header, packet); break;
			case MessageCode::ShootRequest:			break;
			case MessageCode::PlaceRequest:			break;

			case MessageCode::PlayerConnected:		
			case MessageCode::PlayerDisconnected:	
			case MessageCode::ShootRequestDenied:	
			case MessageCode::PlaceRequestDenied:	
			case MessageCode::Shoot:				
			case MessageCode::Place:				
			case MessageCode::PlayerDeath:			
				LOG_WARN("Server recieved invalid message code"); break;

			default:								LOG_ERROR("Unknown message code: {}", static_cast<int>(header.messageCode)); break;
			}
		}
		else if (status == sf::Socket::Done)
		{ 
			LOG_ERROR("Error occurred while receiving messages");
		}
	}

}

void ServerApplication::ProcessConnect(const MessageHeader& header, sf::Packet& packet, const sf::IpAddress& ip, const unsigned short port)
{
	ClientID newClientID;
	// check we don't have too many clients connected
	if (m_Clients.size() < MAX_NUM_PLAYERS)
	{
		newClientID = m_NextClientID.front();
		m_NextClientID.pop();
	}
	else
		// reject this clients connection
		newClientID = INVALID_CLIENT_ID;

	MessageHeader h{ newClientID, MessageCode::Connect, m_ServerClock.getElapsedTime().asSeconds(), header.sequence };
	sf::Packet response;
	response << h;

	// tell the new client about the game and the other players in it in the response
	ConnectMessage messageBody;
	messageBody.numPlayers = static_cast<sf::Uint8>(m_Clients.size());
	for (size_t i = 0; i < m_Clients.size(); i++)
		messageBody.playerIDs[i] = m_Clients[i].id;

	response << messageBody;

	m_Socket.send(response, ip, port);

	// tell all other clients a new player has connected
	for (auto& c : m_Clients)
	{
		MessageHeader h2{ c.id, MessageCode::PlayerConnected, m_ServerClock.getElapsedTime().asSeconds(), 0 };
		sf::Packet p;
		p << h2;

		PlayerConnectedMessage messageBody2{ newClientID };
		p << messageBody2;

		m_Socket.send(p, c.ip, c.port);
	}

	m_Clients.push_back({ newClientID, ip, port });
}

void ServerApplication::ProcessDisconnect(const MessageHeader& header, sf::Packet& packet)
{
	// need to copy here because client will be getting erased
	Client client = FindClientWithID(header.clientID);
	
	MessageHeader h{ client.id, MessageCode::Disconnect, m_ServerClock.getElapsedTime().asSeconds(), header.sequence };
	sf::Packet response;
	response << h;

	// acknowledge the clients requests to disconnect
	m_Socket.send(response, client.ip, client.port);

	m_NextClientID.push(client.id);

	// remove client from vector
	auto it = m_Clients.begin();
	for (;it != m_Clients.end(); it++)
	{
		if ((*it).id == client.id) break;
	}
	m_Clients.erase(it);

	// tell all other players a player disconnected
	for (auto& c : m_Clients)
	{
		MessageHeader h2{ c.id, MessageCode::PlayerDisconnected, m_ServerClock.getElapsedTime().asSeconds(), 0 };
		sf::Packet p;
		p << h2;

		PlayerDisconnectedMessage messageBody2{ client.id };
		p << messageBody2;

		m_Socket.send(p, c.ip, c.port);
	}
}

void ServerApplication::ProcessUpdate(const MessageHeader& header, sf::Packet& packet)
{
	Client& client = FindClientWithID(header.clientID);

	// unpack packet
	UpdateMessage message;
	packet >> message;

	// optimization: check for any changes here before re-sending to all other clients
	client.x = message.x;
	client.y = message.y;
	client.rotation = message.rotation;

	// send out to all other clients 
	for (auto& c : m_Clients)
	{
		if (c.id == client.id) continue;

		//SendUpdate(client.id, message);
	}
}

Client& ServerApplication::FindClientWithID(ClientID id)
{
	for (auto& client : m_Clients)
	{
		if (client.id == id) return client;
	}

	LOG_ERROR("Client with ID {} doesn't exist!", id);

	// error state
	Client errClient;
	return errClient;
}
