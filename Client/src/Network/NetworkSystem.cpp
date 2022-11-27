#include "NetworkSystem.h"

#include <imgui.h>

#include "NetworkPlayer.h"
#include "GameObjects\ControllablePlayer.h"
#include "GameObjects\Projectile.h"
#include "GameObjects\Block.h"


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

void NetworkSystem::Init(ControllablePlayer* player,
	std::vector<NetworkPlayer*>* networkPlayers,
	std::vector<Projectile*>* projectiles,
	std::vector<Block*>* blocks,
	GameState* gameState,
	std::function<void(float)> changeTurfLineFunc)
{
	m_Player = player;
	m_NetworkPlayers = networkPlayers;
	m_Projectiles = projectiles;
	m_Blocks = blocks;
	m_GameState = gameState;
	m_ChangeTurfLineFunc = changeTurfLineFunc;
}

void NetworkSystem::GUI()
{
	ImGui::Text("Simulation time: %.3f", m_SimulationTime);
	if (Connected())
	{
		ImGui::Separator();
		if (ImGui::Button("Disconnect")) Disconnect();
		ImGui::Text("Client ID: %d", m_ClientID);
		ImGui::Text("Player Number: %d/%d", m_PlayerNumber, m_NetworkPlayers->size() + 1);

		ImGui::Separator();
		ImGui::Text("Game State: %s", GameStateToStr(*m_GameState));
		ImGui::Text("Remaining State Duration: %0.1f", m_RemainingGameStateDuration);

		ImGui::Separator();
		if (ImGui::Button("Change Team")) RequestChangeTeam();
		
		if (*m_GameState == GameState::Lobby)
		{
			if (m_GameStartRequested)
			{
				ImGui::Text("Waiting on other players to be ready...");
			}
			else
			{
				if (ImGui::Button("Ready To Start")) RequestGameStart();
			}
		}
	}
	else
	{
		ImGui::InputText("Server IP", m_GUIServerIP, 16);
		ImGui::InputInt("Server Port", &m_GUIServerPort);
		if (ImGui::Button("Connect"))
		{
			m_ServerAddress = sf::IpAddress{ m_GUIServerIP };
			m_ServerPort = static_cast<unsigned short>(m_GUIServerPort);
			Connect();
		}
	}
}

void NetworkSystem::Update(float dt)
{
	m_SimulationTime += dt;

	// handle tcp traffic
	if (m_ConnectionState == ConnectionState::Connected)
	{
		ProcessIncomingUdp();
		ProcessOutgoingUdp(dt);
		ProcessIncomingTcp();

		if (m_RemainingGameStateDuration > 0.0f)
			m_RemainingGameStateDuration -= dt;
		else
			m_RemainingGameStateDuration = 0.0f;

		for (auto& player : *m_NetworkPlayers)
			player->Update(m_SimulationTime);
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


#pragma region Functions

void NetworkSystem::Connect()
{
	if (Connected())
	{
		LOG_WARN("Attempting to connect while already connected!");
		return;
	}

	auto status = m_TcpSocket.connect(m_ServerAddress, m_ServerPort);
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
	MessageHeader header = CreateHeader(MessageCode::Disconnect);
	sf::Packet packet;
	packet << header;
	SendPacketToServerTcp(packet);
}

void NetworkSystem::RequestGameStart()
{
	MessageHeader header = CreateHeader(MessageCode::GameStart);
	sf::Packet p;
	p << header;
	SendPacketToServerTcp(p);

	m_GameStartRequested = true;
}

void NetworkSystem::RequestChangeTeam()
{
	MessageHeader header = CreateHeader(MessageCode::ChangeTeam);
	sf::Packet packet;
	packet << header;
	SendPacketToServerTcp(packet);
}

void NetworkSystem::RequestShoot(const sf::Vector2f& position, const sf::Vector2f& direction)
{
	if (!Connected()) return;

	MessageHeader header = CreateHeader(MessageCode::Shoot);

	ShootMessage shootMessage
	{
		INVALID_PROJECTILE_ID, // will be assigned by server
		m_ClientID,
		m_Player->GetTeam(), 
		position.x, position.y,
		direction.x, direction.y,
		m_SimulationTime
	};

	sf::Packet packet;
	packet << header << shootMessage;
	SendPacketToServerTcp(packet);

	// spawn the local copy of the projectile
	Projectile* localProjectile = new Projectile(shootMessage.id, shootMessage.team, { shootMessage.x, shootMessage.y }, { shootMessage.dirX, shootMessage.dirY });
	m_Projectiles->push_back(localProjectile);
	m_LocalProjectiles.push(localProjectile);
}

void NetworkSystem::RequestPlaceBlock(const sf::Vector2f& position)
{
	if (!Connected()) return;

	MessageHeader header = CreateHeader(MessageCode::Place);

	PlaceMessage placeMessage
	{
		INVALID_BLOCK_ID, // will be assigned by server
		m_ClientID,
		m_Player->GetTeam(),
		position.x, position.y
	};

	sf::Packet packet;
	packet << header << placeMessage;
	SendPacketToServerTcp(packet);

	Block* localBlock = new Block(placeMessage.id, placeMessage.team, { placeMessage.x, placeMessage.y });
	m_Blocks->push_back(localBlock);
	m_LocalBlocks.push(localBlock);
}

void NetworkSystem::SyncSimulationTime()
{
	MessageHeader header = CreateHeader(MessageCode::GetServerTime);
	sf::Packet packet;
	packet << header;
	SendPacketToServerTcp(packet);
	m_LatencyPingBegin = m_SimulationTime;
}

#pragma endregion

#pragma region Messaging

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
		case MessageCode::Ping:			SendPing(); break;
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
	if (m_UpdateTimer > UPDATE_FREQUENCY)
	{
		m_UpdateTimer = 0.0f;

		float dt = std::max(0.0f, m_SimulationTime - m_LastUpdateTime);
		m_LastUpdateTime = m_SimulationTime;

		// send an update to the server
		MessageHeader header = CreateHeader(MessageCode::Update);

		auto playerPos = m_Player->getPosition();
		UpdateMessage messageBody
		{
			m_ClientID,
			playerPos.x, playerPos.y,
			m_Player->getRotation(),
			dt
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
		case MessageCode::Disconnect:			OnDisconnect			(header, packet); break;
		case MessageCode::PlayerConnected:		OnOtherPlayerConnect	(header, packet); break;
		case MessageCode::PlayerDisconnected:	OnOtherPlayerDisconnect	(header, packet); break;
		case MessageCode::ChangeTeam:			OnPlayerChangeTeam		(header, packet); break;
		case MessageCode::GetServerTime:		OnServerTimeUpdate		(header, packet); break;
		case MessageCode::Shoot:				OnShoot					(header, packet); break;
		case MessageCode::ProjectilesDestroyed:	OnProjectilesDestroyed	(header, packet); break;
		case MessageCode::ShootRequestDenied:	OnShootRequestDenied	(header, packet); break;
		case MessageCode::Place:				OnPlace					(header, packet); break;
		case MessageCode::BlocksDestroyed:		OnBlocksDestroyed		(header, packet); break;
		case MessageCode::PlaceRequestDenied:	OnPlaceRequestDenied	(header, packet); break;
		case MessageCode::ChangeGameState:		OnChangeGameState		(header, packet); break;
		case MessageCode::TurfLineMoved:		OnTurfLineMoved			(header, packet); break;
		case MessageCode::PlayerDeath:			OnPlayerDeath			(header, packet); break;
		case MessageCode::GameStart:			OnGameStart				(header, packet); break;
		default:								LOG_WARN("Recieved unexpected message code"); break;
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
	sf::Socket::Status status = m_UdpSocket.send(packet, m_ServerAddress, m_ServerPort);
	if (status != sf::Socket::Done)
	{
		LOG_ERROR("Sending message to server failed!");
	}
}

#pragma endregion

#pragma region Process Responses

void NetworkSystem::OnConnect(const MessageHeader& header, sf::Packet& packet)
{
	m_ConnectionState = ConnectionState::Connected;
	m_ClientID = header.clientID;

	ConnectMessage connectMessage;
	packet >> connectMessage;

	m_PlayerNumber = connectMessage.playerNumber;
	m_Player->SetTeam(connectMessage.team);
	GoToSpawn();

	for (auto i = 0; i < connectMessage.numPlayers; i++)
	{
		NetworkPlayer* newPlayer = new NetworkPlayer(connectMessage.playerIDs[i]);
		newPlayer->SetTeam(connectMessage.playerTeams[i]);
		m_NetworkPlayers->push_back(newPlayer);
	}

	for (auto i = 0; i < connectMessage.numBlocks; i++)
	{
		Block* newBlock = new Block(connectMessage.blockIDs[i], connectMessage.blockTeams[i], { connectMessage.blockXs[i] , connectMessage.blockYs[i] });
		m_Blocks->push_back(newBlock);
	}

	*m_GameState = connectMessage.gameState;
	m_RemainingGameStateDuration = connectMessage.remainingStateDuration;
	m_ChangeTurfLineFunc(connectMessage.turfLine);

	// introduce client to server
	
	MessageHeader replyHeader = CreateHeader(MessageCode::Introduction);
	IntroductionMessage replyBody{ static_cast<sf::Uint16>(m_UdpSocket.getLocalPort()) };
	sf::Packet reply;
	reply << replyHeader << replyBody;
	SendPacketToServerTcp(reply);

	// also request the simulation time
	SyncSimulationTime();

	LOG_INFO("Connected with ID {}", static_cast<int>(m_ClientID));
	LOG_INFO("There are {} other players already connected", connectMessage.numPlayers);
	LOG_INFO("I am player {}", m_PlayerNumber);
}


void NetworkSystem::OnDisconnect(const MessageHeader& header, sf::Packet& packet)
{
	m_ConnectionState = ConnectionState::Disconnected;
	m_ClientID = INVALID_CLIENT_ID;
	m_SimulationTime = 0.0f;
	m_Player->SetTeam(PlayerTeam::None);
	m_GameStartRequested = false;
	*m_GameState = GameState::Lobby;
	m_RemainingGameStateDuration = 0;

	for (auto player : *m_NetworkPlayers)
		delete player;
	m_NetworkPlayers->clear();
	for (auto projectile : *m_Projectiles)
		delete projectile;
	m_Projectiles->clear();
	for (auto block : *m_Blocks)
		delete block;
	m_Blocks->clear();
	
	LOG_INFO("Disconnected");
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

	// packet will contain updates for potentially multiple players
	// loop while the packet still contains data
	while (!packet.endOfPacket())
	{
		packet >> messageBody;

		if (messageBody.playerID == m_ClientID) continue;

		NetworkPlayer* player = FindNetworkPlayerWithID(messageBody.playerID);
		if (player) player->NetworkUpdate(messageBody, m_SimulationTime);
	}
}

void NetworkSystem::OnPlayerChangeTeam(const MessageHeader& header, sf::Packet& packet)
{
	ChangeTeamMessage messageBody;
	packet >> messageBody;

	if (messageBody.playerID == m_ClientID)
	{
		m_Player->SetTeam(messageBody.team);
		GoToSpawn();
	}
	else
	{
		NetworkPlayer* player = FindNetworkPlayerWithID(messageBody.playerID);
		player->SetTeam(messageBody.team);
	}
}

void NetworkSystem::OnServerTimeUpdate(const MessageHeader&, sf::Packet& packet)
{
	// measure round trip time
	float latency = m_SimulationTime - m_LatencyPingBegin;

	ServerTimeMessage messageBody;
	packet >> messageBody;

	m_SimulationTime = messageBody.serverTime - (0.5f * latency);
}

void NetworkSystem::OnShoot(const MessageHeader& header, sf::Packet& packet)
{
	// extract message body
	ShootMessage shootMessage;
	packet >> shootMessage;

	// did we shoot this projectile
	if (shootMessage.shotBy == m_ClientID)
	{
		// shoot request confirmed
		m_LocalProjectiles.front()->UpdateID(shootMessage.id);
		m_LocalProjectiles.pop();
	}
	else
	{
		// someone else shot this projectile
		// spawn it in
		Projectile* newProjectile = new Projectile(shootMessage.id, shootMessage.team, { shootMessage.x, shootMessage.y }, { shootMessage.dirX, shootMessage.dirY });
		m_Projectiles->push_back(newProjectile);
	}
}

void NetworkSystem::OnProjectilesDestroyed(const MessageHeader& header, sf::Packet& packet)
{
	ProjectilesDestroyedMessage message;
	packet >> message;

	for (auto it = m_Projectiles->begin(); it != m_Projectiles->end();)
	{
		bool projDestroyed = false;
		for (auto i = 0; i < message.count; i++)
		{
			if ((*it)->GetID() == message.ids[i])
			{
				it = m_Projectiles->erase(it);
				projDestroyed = true;
				break;
			}
		}
		if (!projDestroyed)
			it++;
	}
}

void NetworkSystem::OnShootRequestDenied(const MessageHeader&, sf::Packet&)
{
	if (m_LocalProjectiles.empty()) return;

	for (auto it = m_Projectiles->begin(); it != m_Projectiles->end(); it++)
	{
		if (*it == m_LocalProjectiles.front())
		{
			delete m_LocalProjectiles.front();
			m_Projectiles->erase(it);
			break;
		}
	}

	m_LocalProjectiles.pop();
}

void NetworkSystem::OnPlace(const MessageHeader& header, sf::Packet& packet)
{
	PlaceMessage placeMessage;
	packet >> placeMessage;

	// create block
	if (placeMessage.placedBy == m_ClientID)
	{
		m_LocalBlocks.front()->UpdateID(placeMessage.id);
		m_LocalBlocks.pop();
	}
	else
	{
		Block* newBlock = new Block(placeMessage.id, placeMessage.team, { placeMessage.x, placeMessage.y });
		m_Blocks->push_back(newBlock);
	}
}

void NetworkSystem::OnBlocksDestroyed(const MessageHeader& header, sf::Packet& packet)
{
	BlocksDestroyedMessage message;
	packet >> message;

	for (auto it = m_Blocks->begin(); it != m_Blocks->end();)
	{
		bool blockDestroyed = false;
		for (auto i = 0; i < message.count; i++)
		{
			if ((*it)->GetID() == message.ids[i])
			{
				it = m_Blocks->erase(it);
				blockDestroyed = true;
				break;
			}
		}
		if (!blockDestroyed)
			it++;
	}
}

void NetworkSystem::OnPlaceRequestDenied(const MessageHeader&, sf::Packet&)
{
	if (m_LocalBlocks.empty()) return;

	for (auto it = m_Blocks->begin(); it != m_Blocks->end(); it++)
	{
		if (*it == m_LocalBlocks.front())
		{
			delete m_LocalBlocks.front();
			m_Blocks->erase(it);
			break;
		}
	}

	m_LocalBlocks.pop();
}

void NetworkSystem::OnChangeGameState(const MessageHeader& header, sf::Packet& packet)
{
	ChangeGameStateMessage message;
	packet >> message;

	*m_GameState = message.state;
	m_RemainingGameStateDuration = message.stateDuration;

	// delete all projectiles
	for (auto projectile : *m_Projectiles)
		delete projectile;
	m_Projectiles->clear();

	if (*m_GameState == GameState::Lobby)
	{
		// returned to lobby: kill all blocks placed by players
		for (auto it = m_Blocks->begin(); it != m_Blocks->end();)
		{
			if ((*it)->GetTeam() != PlayerTeam::None)
			{
				delete (*it);
				it = m_Blocks->erase(it);
			}
			else it++;
		}
	}
}

void NetworkSystem::OnTurfLineMoved(const MessageHeader& header, sf::Packet& packet)
{
	TurfLineMoveMessage message;
	packet >> message;

	m_ChangeTurfLineFunc(message.newTurfLine);
}

void NetworkSystem::OnPlayerDeath(const MessageHeader& header, sf::Packet& packet)
{
	GoToSpawn();
}

void NetworkSystem::OnGameStart(const MessageHeader& header, sf::Packet& packet)
{
	GoToSpawn();

	*m_GameState = GameState::BuildMode;
	m_RemainingGameStateDuration = INITIAL_BUILD_MODE_DURATION;

	m_ChangeTurfLineFunc(0.5f * WORLD_WIDTH);

	m_GameStartRequested = false;
}

void NetworkSystem::SendPing()
{
	MessageHeader header{ m_ClientID, MessageCode::Ping };
	sf::Packet p;
	p << header;
	SendPacketToServerUdp(p);
}


#pragma endregion

#pragma region Utility


NetworkPlayer* NetworkSystem::FindNetworkPlayerWithID(ClientID id)
{
	for (auto player : *m_NetworkPlayers)
	{
		if (player->GetID() == id) return player;
	}
	return nullptr;
}

void NetworkSystem::GoToSpawn()
{
	if (m_Player->GetTeam() == PlayerTeam::Red)
		m_Player->setPosition({ 0.5f * SPAWN_WIDTH, 0.5f * WORLD_HEIGHT });
	else
		m_Player->setPosition({ WORLD_WIDTH - 0.5f * SPAWN_WIDTH, 0.5f * WORLD_HEIGHT });
}

#pragma endregion
