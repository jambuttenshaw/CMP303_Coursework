#include "NetworkSystem.h"

#include "GameObjects\ControllablePlayer.h"
#include "GameObjects\NetworkPlayer.h"


NetworkSystem::NetworkSystem()
{
	m_TcpSocket.setBlocking(false);
	m_UdpSocket.setBlocking(false);

	m_UdpSocket.bind(sf::Socket::AnyPort);
}

NetworkSystem::~NetworkSystem()
{
	if (Connected()) Disconnect();
}

void NetworkSystem::Init(ControllablePlayer* player, std::vector<NetworkPlayer*>* networkPlayers)
{
	m_Player = player;
	m_NetworkPlayers = networkPlayers;
}

void NetworkSystem::Connect()
{
	if (Connected())
	{
		LOG_WARN("Attempting to connect while already connected!");
		return;
	}

	auto status = m_TcpSocket.connect(ServerAddress, ServerPort);
	if (status != sf::Socket::Done)
	{
		// wait to recieve client ID from server
		m_ConnectionState = ConnectionState::Connecting;
	}
	else
	{
		LOG_ERROR("Error connecting to server!");
	}
}

void NetworkSystem::Disconnect()
{
	if (!Connected())
	{
		LOG_WARN("Attempting to disconnect while already disconnected!");
		return;
	}

	MessageHeader header = CreateHeader(MessageCode::Disconnect);
	sf::Packet packet;
	packet << header;
	SendPacketToServerTcp(packet);
}


void NetworkSystem::Update(float dt)
{
	// handle tcp traffic

	if (m_ConnectionState == ConnectionState::Connected)
	{
		ProcessIncomingUdp();
		ProcessOutgoingUdp(dt);
		ProcessIncomingTcp();
	}
	else if (m_ConnectionState == ConnectionState::Connecting)
	{
		// handle waiting for client ID
		sf::Packet packet;
		auto status = m_TcpSocket.receive(packet);
		if (status == sf::Socket::Done)
		{
			// received data
			MessageHeader header;
			packet >> header;

			if (header.messageCode == MessageCode::Connect)
			{
				if (header.clientID == INVALID_CLIENT_ID)
				{
					// server has rejected connection
					m_ConnectionState = ConnectionState::Disconnected;
					LOG_INFO("Server rejected connection");
				}
				else
					OnConnect(header, packet);
			}
			else
			{
				LOG_ERROR("Waiting for connect message from server, but received a different message");
			}
		}
	}
}


void NetworkSystem::ProcessIncomingUdp()
{
	sf::Packet packet;
	sf::IpAddress fromAddr;
	unsigned short fromPort;
	sf::Socket::Status status = m_UdpSocket.receive(packet, fromAddr, fromPort);
	if (status == sf::Socket::Done)
	{
		MessageHeader header;
		packet >> header;

		if (header.clientID != m_ClientID)
		{
			LOG_WARN("Received message addressed to a different client!");
			return;
		}

		switch (header.messageCode)
		{
		case MessageCode::Update:		OnRecieveUpdate(header, packet); break;
		default:						LOG_WARN("Received unexpected message code!"); break;
		}
	}
	else if (status == sf::Socket::Error)
	{
		LOG_ERROR("UDP error occurred while trying to receive from server");
	}
}


void NetworkSystem::ProcessOutgoingUdp(float dt)
{
	// send periodic updates to the server
	m_UpdateTimer += dt;
	if (m_UpdateTimer > UpdateTickSpeed)
	{
		m_UpdateTimer = 0.0f;

		// send an update to the server
		MessageHeader header{ m_ClientID, MessageCode::Update, m_SimulationTime };

		auto playerPos = m_Player->getPosition();
		UpdateMessage messageBody
		{
			m_ClientID,
			playerPos.x, playerPos.y,
			m_Player->getRotation()
		};

		sf::Packet packet;
		packet << header << messageBody;

		SendPacketToServerUdp(packet);
	}
}

void NetworkSystem::ProcessIncomingTcp()
{
	sf::Packet packet;
	auto status = m_TcpSocket.receive(packet);
	if (status == sf::Socket::Done)
	{
		// received data
		MessageHeader header;
		packet >> header;

		if (header.clientID != m_ClientID)
		{
			LOG_ERROR("Recieved message addressed to a different client!");
			return;
		}

		switch (header.messageCode)
		{
		case MessageCode::Disconnect:			OnDisconnect(header, packet); break;
		case MessageCode::PlayerConnected:		OnOtherPlayerConnect(header, packet); break;
		case MessageCode::PlayerDisconnected:	OnOtherPlayerDisconnect(header, packet); break;
		default:								LOG_WARN("Recieved unexpected message code");
		}

	}
}


void NetworkSystem::SendPacketToServerTcp(sf::Packet& packet)
{
	sf::Socket::Status status;
	do
	{
		status = m_TcpSocket.send(packet);
	} while (status == sf::Socket::Partial);

	if (status != sf::Socket::Done)
		LOG_ERROR("Error sending packet to server!");
}

	
void NetworkSystem::SendPacketToServerUdp(sf::Packet& packet)
{
	sf::Socket::Status status = m_UdpSocket.send(packet, ServerAddress, ServerPort);
	if (status != sf::Socket::Done)
	{
		LOG_ERROR("Sending message to server failed!");
	}
}

// PROCESS RESPONSES

void NetworkSystem::OnConnect(const MessageHeader& header, sf::Packet& packet)
{
	m_ConnectionState = ConnectionState::Connected;
	m_ClientID = header.clientID;
	m_SimulationTime = header.time;

	ConnectMessage messageBody;
	packet >> messageBody;

	LOG_INFO("Connected with ID {}", static_cast<int>(m_ClientID));
	LOG_INFO("There are {} other players already connected", messageBody.numPlayers);

	m_Player->SetTeam(messageBody.team);

	for (auto i = 0; i < messageBody.numPlayers; i++)
	{
		NetworkPlayer* newPlayer = new NetworkPlayer(messageBody.playerIDs[i]);
		newPlayer->SetTeam(messageBody.playerTeams[i]);
		m_NetworkPlayers->push_back(newPlayer);
	}

	// introduce client to server
	MessageHeader replyHeader{ m_ClientID, MessageCode::Introduction, m_SimulationTime };
	IntroductionMessage replyBody{ static_cast<sf::Uint16>(m_UdpSocket.getLocalPort()) };
	sf::Packet reply;
	reply << replyHeader << replyBody;
	SendPacketToServerTcp(reply);
}


void NetworkSystem::OnDisconnect(const MessageHeader& header, sf::Packet& packet)
{
	m_ConnectionState = ConnectionState::Disconnected;
	m_ClientID = INVALID_CLIENT_ID;
	m_SimulationTime = 0.0f;

	LOG_INFO("Disconnected");

	for (auto player : *m_NetworkPlayers)
		delete player;
	m_NetworkPlayers->clear();

	m_Player->SetTeam(PlayerTeam::None);
}

void NetworkSystem::OnOtherPlayerConnect(const MessageHeader& header, sf::Packet& packet)
{
	// extract message body
	PlayerConnectedMessage messageBody;
	packet >> messageBody;

	LOG_INFO("Player ID {} has joined", messageBody.playerID);

	NetworkPlayer* newPlayer = new NetworkPlayer(messageBody.playerID);
	newPlayer->SetTeam(messageBody.team);
	m_NetworkPlayers->push_back(newPlayer);
}

void NetworkSystem::OnOtherPlayerDisconnect(const MessageHeader& header, sf::Packet& packet)
{
	// extract message body
	PlayerDisconnectedMessage messageBody;
	packet >> messageBody;

	LOG_INFO("Player ID {} has left", messageBody.playerID);

	auto it = m_NetworkPlayers->begin();
	for (; it != m_NetworkPlayers->end(); it++)
	{
		if ((*it)->GetID() == messageBody.playerID) break;
	}
	if (it != m_NetworkPlayers->end())
	{
		delete (*it);
		m_NetworkPlayers->erase(it);
	}
	else
		LOG_WARN("Player {} doesn't exist!", messageBody.playerID);
}

void NetworkSystem::OnRecieveUpdate(const MessageHeader& header, sf::Packet& packet)
{
	UpdateMessage messageBody;
	packet >> messageBody;

	NetworkPlayer* player = FindNetworkPlayerWithID(messageBody.playerID);
	if (player) player->NetworkUpdate(messageBody);
}



NetworkPlayer* NetworkSystem::FindNetworkPlayerWithID(ClientID id)
{
	for (auto player : *m_NetworkPlayers)
	{
		if (player->GetID() == id) return player;
	}
	return nullptr;
}

