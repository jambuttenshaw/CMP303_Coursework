#include "NetworkSystem.h"

#include "Log.h"


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
	SendPacketToServer(packet);
}


void NetworkSystem::Update(float dt)
{
	// handle tcp traffic

	if (m_ConnectionState == ConnectionState::Connected)
	{
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

	// handle udp traffic
	ProcessOutgoing(dt);
}


void NetworkSystem::ProcessOutgoing(float dt)
{
	// send periodic updates to the server
	if (Connected())
	{
		m_UpdateTimer += dt;
		if (m_UpdateTimer > UpdateTickSpeed)
		{
			m_UpdateTimer = 0.0f;

			//SendUpdateToServer();
		}
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

void NetworkSystem::ProcessIncoming()
{
	/*
	//if (!m_SocketBound) return;
	//if (!m_Connected) return;
	
	// check if the server has sent any messages
	sf::Packet packet;
	//sf::IpAddress fromAddress;
	//unsigned short fromPort;
	sf::Socket::Status status = m_TcpSocket.receive(packet);

	if (status == sf::Socket::Done)
	{
		// message recieved

		// extract header
		MessageHeader header;
		packet >> header;
		
		if (m_WaitingOnReply)
		{
			if (m_ReplySequence == header.sequence)
			{
				// we have recieved the reply to the last message we sent
				m_WaitingOnReply = false;
			}
			else
			{
				// ignore incoming messages until we get the reply we are looking for
				LOG_WARN("Ignoring incoming message: waiting on a different reply");
				return;
			}
		}
		
		// connect is a special case
		if (!m_Connected)
		{
			if (header.messageCode == MessageCode::Connect)
				OnConnect(header, packet);
			else
				LOG_ERROR("Receiving a message other than connect before connection is made!");
			return;
		}

		if (header.clientID != m_ClientID)
		{
			LOG_ERROR("Recieved message addressed to another client!");
			return;
		}

		// process message
		switch (header.messageCode)
		{
		case MessageCode::Connect:				break;
		case MessageCode::Disconnect:			OnDisconnect(header, packet); break;
		case MessageCode::PlayerConnected:		OnOtherPlayerConnect(header, packet); break;
		case MessageCode::PlayerDisconnected:	OnOtherPlayerDisconnect(header, packet); break;
		default:								LOG_WARN("Unknown message code {0}", static_cast<int>(header.messageCode)); break;
		}
	}
	else if (status == sf::Socket::Error)
	{
		LOG_ERROR("Error occurred while receiving from server!");
	}
	*/
}


void NetworkSystem::SendPacketToServer(sf::Packet& packet)
{
	sf::Socket::Status status;
	do
	{
		status = m_TcpSocket.send(packet);
	} while (status == sf::Socket::Partial);

	if (status != sf::Socket::Done)
		LOG_ERROR("Error sending packet to server!");
}

	/*
void NetworkSystem::SendPacketToServer(sf::Packet& packet, bool expectReply)
{
	m_SocketBound = true;

	if (expectReply)
	{
		// copy the message so it can be sent again if needed
		m_LastMessage = sf::Packet(packet);
		m_WaitingOnReply = true;
		m_ReplySequence = m_Sequence;
	}
	
	sf::Socket::Status status = m_Socket.send(packet, ServerAddress, ServerPort);
	if (status != sf::Socket::Done)
	{
		LOG_ERROR("Sending message to server failed!");
	}
	m_Sequence++;
}
*/

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

	// introduce client to server
	MessageHeader replyHeader{ m_ClientID, MessageCode::Introduction, m_SimulationTime };
	IntroductionMessage replyBody{ static_cast<sf::Uint16>(m_UdpSocket.getLocalPort()) };
	sf::Packet reply;
	reply << replyHeader << replyBody;
	SendPacketToServer(reply);
}


void NetworkSystem::OnDisconnect(const MessageHeader& header, sf::Packet& packet)
{
	m_ConnectionState = ConnectionState::Disconnected;
	m_ClientID = INVALID_CLIENT_ID;
	m_SimulationTime = 0.0f;

	LOG_INFO("Disconnected");
}

void NetworkSystem::OnOtherPlayerConnect(const MessageHeader& header, sf::Packet& packet)
{
	// extract message body
	PlayerConnectedMessage messageBody;
	packet >> messageBody;

	LOG_INFO("Player ID {} has joined", messageBody.playerID);
}

void NetworkSystem::OnOtherPlayerDisconnect(const MessageHeader& header, sf::Packet& packet)
{
	// extract message body
	PlayerDisconnectedMessage messageBody;
	packet >> messageBody;

	LOG_INFO("Player ID {} has left", messageBody.playerID);
}
