#include "ServerApplication.h"

#include "Log.h"
#include "Network/NetworkTypes.h"


ServerApplication::ServerApplication()
{
	LOG_INFO("----Server----");
	LOG_INFO("Local IP: {}", sf::IpAddress::getLocalAddress().toString());

	m_ListenSocket.setBlocking(false);
	m_UdpSocket.setBlocking(false);

	if (m_UdpSocket.bind(ServerPort) != sf::Socket::Done)
	{
		LOG_ERROR("Server failed to bind to port {}", ServerPort);
	}
	LOG_INFO("UDP: listening on port {}", ServerPort);

	if (m_ListenSocket.listen(ServerPort) != sf::Socket::Done)
	{
		LOG_ERROR("Server failed to listen on port {}", ServerPort);
	}
	LOG_INFO("TCP: listening on port {}", ServerPort);

	for (ClientID id = 0; id < INVALID_CLIENT_ID; id++)
		m_NextClientID.push(id);

	m_Clients.reserve(MAX_NUM_PLAYERS);

	m_NewConnection = new Connection;
}

ServerApplication::~ServerApplication()
{
	// disconnect all clients
	for (auto client : m_Clients)
		delete client;
}

void ServerApplication::Run()
{
	m_ServerClock.restart();

	while (true)
	{

		// listen for new connections
		if (m_ListenSocket.accept(m_NewConnection->GetSocket()) == sf::Socket::Done)
		{
			// new connection found
			// setup connection

			// check we don't have too many clients connected
			if (m_Clients.size() < MAX_NUM_PLAYERS)
			{
				ProcessConnect();
			}
			else
			{
				// reject this clients connection
				MessageHeader h{ INVALID_CLIENT_ID, MessageCode::Connect, m_ServerClock.getElapsedTime().asSeconds() };
				sf::Packet response;
				response << h;

				m_NewConnection->SendPacketTcp(response);
				m_NewConnection->GetSocket().disconnect();
			}
		}

		for (auto client : m_Clients)
		{
			// try to recieve data
			sf::Packet packet;
			sf::Socket::Status status = client->GetSocket().receive(packet);
			if (status == sf::Socket::Done)
			{
				// data was recieved
				MessageHeader header;
				packet >> header;

				switch (header.messageCode)
				{
				case MessageCode::Introduction:			ProcessIntroduction(client, header, packet); break;
				case MessageCode::Disconnect:			ProcessDisconnect(client, header, packet); break;
				case MessageCode::ChangeTeam:			ProcessChangeTeam(client, header, packet); break;
				case MessageCode::ShootRequest:			break;
				case MessageCode::PlaceRequest:			break;
				case MessageCode::LatencyPing:			break;
					// these messages are sent from the server to clients, so it would be incorrect for the server to recieve them
				case MessageCode::Connect:
				case MessageCode::PlayerConnected:
				case MessageCode::PlayerDisconnected:
				case MessageCode::ShootRequestDenied:
				case MessageCode::PlaceRequestDenied:
				case MessageCode::Shoot:
				case MessageCode::Place:
				case MessageCode::PlayerDeath:
														LOG_WARN("Received invalid message code"); break;
				case MessageCode::Update:						  
														LOG_WARN("Received update message via TCP; updates should be sent via UDP"); break;

				default:								LOG_ERROR("Unknown message code: {}", static_cast<int>(header.messageCode)); break;
				}
			}
			else if (status == sf::Socket::Done)
			{
				LOG_ERROR("Error occurred while receiving messages");
			}
		}

		// listen for incoming UDP data
		sf::Packet packet;
		sf::IpAddress fromAddr;
		unsigned short fromPort;
		auto status = m_UdpSocket.receive(packet, fromAddr, fromPort);
		if (status == sf::Socket::Done)
		{
			// data was recieved
			MessageHeader header;
			packet >> header;

			// its possible the client has been disconnected between sending an update and the server receiving it
			// so the client may no longer exist
			Connection* client = FindClientWithID(header.clientID);
			if (client)
			{
				switch (header.messageCode)
				{
				case MessageCode::Update:	ProcessUpdate(client, header, packet); break;
				default:					LOG_WARN("Received unexpected message code"); break;
				}
			}
		}
	}
}


void ServerApplication::ProcessConnect()
{
	ClientID newClientID = m_NextClientID.front();
	m_NextClientID.pop();

	// setup connection object
	m_NewConnection->OnTcpConnected(newClientID);

	// tell the client their ID
	MessageHeader h{ newClientID, MessageCode::Connect, m_ServerClock.getElapsedTime().asSeconds() };
	sf::Packet response;
	response << h;

	ConnectMessage messageBody;

	// assign their team
	PlayerState& state = m_NewConnection->GetPlayerState();
	if (m_RedTeamPlayerCount < m_BlueTeamPlayerCount)
	{
		state.team = PlayerTeam::Red;
		m_RedTeamPlayerCount++;
	}
	else
	{
		state.team = PlayerTeam::Blue;
		m_BlueTeamPlayerCount++;
	}
	messageBody.team = state.team;

	// tell them about the current world state
	messageBody.numPlayers = static_cast<sf::Uint8>(m_Clients.size());
	for (size_t i = 0; i < m_Clients.size(); i++)
	{
		messageBody.playerIDs[i] = m_Clients[i]->GetID();

		PlayerState& s = m_Clients[i]->GetPlayerState();
		messageBody.playerTeams[i] = s.team;
	}
	response << messageBody;

	m_NewConnection->SendPacketTcp(response);

	// tell all other clients a new player has connected
	for (auto& c : m_Clients)
	{
		MessageHeader h2{ c->GetID(), MessageCode::PlayerConnected, m_ServerClock.getElapsedTime().asSeconds() };
		sf::Packet p;
		p << h2;

		PlayerConnectedMessage messageBody2{ newClientID, state.team };
		p << messageBody2;

		c->SendPacketTcp(p);
	}

	m_Clients.push_back(m_NewConnection);
	LOG_INFO("Player connected with ID {}", newClientID);

	m_NewConnection = new Connection;
}

void ServerApplication::ProcessIntroduction(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	IntroductionMessage message;
	packet >> message;

	client->SetUdpPort(message.udpPort);
}



void ServerApplication::ProcessDisconnect(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	MessageHeader h{ client->GetID(), MessageCode::Disconnect, m_ServerClock.getElapsedTime().asSeconds() };
	sf::Packet response;
	response << h;

	// acknowledge the clients requests to disconnect
	client->SendPacketTcp(response);

	m_NextClientID.push(client->GetID());

	// remove client from vector
	auto it = m_Clients.begin();
	for (; it != m_Clients.end(); it++)
	{
		if (*it == client) break;
	}
	m_Clients.erase(it);

	// tell all other players a player disconnected
	for (auto& c : m_Clients)
	{
		MessageHeader h2{ c->GetID(), MessageCode::PlayerDisconnected, m_ServerClock.getElapsedTime().asSeconds() };
		sf::Packet p;
		p << h2;

		PlayerDisconnectedMessage messageBody2{ client->GetID() };
		p << messageBody2;

		c->SendPacketTcp(p);
	}

	// finally delete the client
	LOG_INFO("Player ID {} disconnected", client->GetID());
	delete client;
}

void ServerApplication::ProcessUpdate(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	// unpack packet
	UpdateMessage message;
	packet >> message;

	if (message.playerID != client->GetID())
	{
		LOG_WARN("Client sending update data with incorrect client ID");
		return;
	}

	// optimization: check for any changes here before re-sending to all other clients
	PlayerState& state = client->GetPlayerState();
	state.Update(message);

	// send out to all other clients
	for (auto& c : m_Clients)
	{
		if (c == client) continue;
		SendMessageToClientUdp(c, MessageCode::Update, message);
	}
}

void ServerApplication::ProcessChangeTeam(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	// decide if the player is allowed to change team
	// for now always allow
	
	// switch team
	PlayerState& state = client->GetPlayerState();
	if (state.team == PlayerTeam::Red)
	{
		state.team = PlayerTeam::Blue;
		m_RedTeamPlayerCount--;
		m_BlueTeamPlayerCount++;
	}
	else
	{
		state.team = PlayerTeam::Red;
		m_RedTeamPlayerCount++;
		m_BlueTeamPlayerCount--;
	}

	ChangeTeamMessage message{ client->GetID(), state.team };

	// transmit this change to all clients
	for (auto c : m_Clients)
	{
		c->SendMessageTcp(MessageCode::ChangeTeam, message, m_ServerClock.getElapsedTime().asSeconds());
	}
}


Connection* ServerApplication::FindClientWithID(ClientID id)
{
	for (auto& connection : m_Clients)
	{
		if (connection->GetID() == id) return connection;
	}
	return nullptr;
}
