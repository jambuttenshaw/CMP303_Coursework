#include "NetworkSystem.h"

#include "Log.h"


NetworkSystem::NetworkSystem()
{
	m_Socket.setBlocking(false);
	srand(time(0));
}

NetworkSystem::~NetworkSystem()
{
}

void NetworkSystem::Connect()
{
	if (Connected())
	{
		LOG_WARN("Attempting to connect while already connected!");
		return;
	}

	MessageHeader header = CreateHeader(MessageCode::Connect);
	sf::Packet packet = header.Create();
	SendPacketToServer(packet, true);
}

void NetworkSystem::Disconnect()
{
	if (!Connected())
	{
		LOG_WARN("Attempting to disconnect while already disconnected!");
		return;
	}

	MessageHeader header = CreateHeader(MessageCode::Disconnect);
	sf::Packet packet = header.Create();
	SendPacketToServer(packet, true);
}


void NetworkSystem::Update(float dt)
{
	ProcessOutgoing(dt);
	ProcessIncoming();
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

	if (m_WaitingOnReply)
	{
		m_ResendTimer += dt;
		if (m_ResendTimer > ResendTimeout)
		{
			m_ResendTimer = 0.0f;
			ResendLastPacketToServer();
		}
	}
}

void NetworkSystem::ProcessIncoming()
{
	if (!m_SocketBound) return;
	
	// check if the server has sent any messages
	sf::Packet packet;
	sf::IpAddress fromAddress;
	unsigned short fromPort;
	sf::Socket::Status status = m_Socket.receive(packet, fromAddress, fromPort);

	if (status == sf::Socket::Done)
	{
		// message recieved

		// extract header
		MessageHeader header;
		header.Extract(packet);

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
		if (!Connected())
		{
			if (header.messageCode == MessageCode::Connect)
			{
				OnConnectResponse(header, packet);
			}
			else
			{
				LOG_ERROR("Receiving a message other than connect before connection is made!");
				return;
			}
		}

		if (header.clientID != m_ClientID)
		{
			LOG_ERROR("Recieved message addressed to another client!");
			return;
		}

		// process message
		switch (header.messageCode)
		{
		case MessageCode::Connect:			break;
		case MessageCode::Disconnect:		OnDisconnectResponse(header, packet); break;
		default:							LOG_WARN("Unknown message code"); break;
		}
	}
	else if (status == sf::Socket::Error)
	{
		LOG_ERROR("Error occurred while receiving from server!");
	}
}


void NetworkSystem::SendPacketToServer(sf::Packet& packet, bool expectReply)
{
	if (m_WaitingOnReply) return;
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

void NetworkSystem::ResendLastPacketToServer()
{
	sf::Packet packet{ m_LastMessage };

	sf::Socket::Status status = m_Socket.send(packet, ServerAddress, ServerPort);
	if (status != sf::Socket::Done)
	{
		LOG_ERROR("Sending message to server failed!");
	}
}


/* PROCESS RESPONSES */

void NetworkSystem::OnConnectResponse(const MessageHeader& header, sf::Packet& packet)
{
	if (Connected())
	{
		LOG_WARN("Server sending connect response while already connected!");
	}
	if (header.clientID == INVALID_CLIENT_ID)
	{
		LOG_ERROR("Connecting to server failed!");
		return;
	}

	m_ClientID = header.clientID;
	m_SimulationTime = header.time;

	LOG_INFO("Connected with ID {}", static_cast<int>(m_ClientID));
}

void NetworkSystem::OnDisconnectResponse(const MessageHeader& header, sf::Packet& packet)
{
	if (header.clientID == INVALID_CLIENT_ID)
	{
		LOG_ERROR("Connecting to server failed!");
		return;
	}

	m_ClientID = INVALID_CLIENT_ID;
	m_SimulationTime = 0.0f;

	LOG_INFO("Disconnected");
}
