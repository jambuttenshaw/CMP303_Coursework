#include "NetworkSystem.h"

#include <imgui.h>

#include "NetworkPlayer.h"
#include "GameObjects\ControllablePlayer.h"
#include "GameObjects\Projectile.h"
#include "GameObjects\Block.h"


NetworkSystem::NetworkSystem()
{
	// setup sockets
	m_TcpSocket.setBlocking(false);
	m_UdpSocket.setBlocking(false);

	// bind a udp socket immediately because it is possible we'll need to recieve before sending
	m_UdpSocket.bind(sf::Socket::AnyPort);
}

NetworkSystem::~NetworkSystem()
{
	// disconnect when the client application is destroyed
	if (Connected()) Disconnect();
}

void NetworkSystem::Init(ControllablePlayer* player,
	std::vector<NetworkPlayer*>* networkPlayers,
	std::vector<Projectile*>* projectiles,
	std::vector<Block*>* blocks,
	GameState* gameState,
	std::function<void(float)> changeTurfLineFunc,
	unsigned int* buildModeBlocks,
	unsigned int* ammo)
{
	m_Player = player;
	m_NetworkPlayers = networkPlayers;
	m_Projectiles = projectiles;
	m_Blocks = blocks;
	m_GameState = gameState;
	m_ChangeTurfLineFunc = changeTurfLineFunc;
	m_BuildModeBlocks = buildModeBlocks;
	m_Ammo = ammo;
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
		
		if (ImGui::Button("Freeze Client"))
		{
			// to test client time out
			while (true);
		}

		ImGui::Separator();
		ImGui::Text("Game State: %s", GameStateToStr(*m_GameState));
		ImGui::Text("Remaining State Duration: %0.1f", m_RemainingGameStateDuration);

		ImGui::Separator();
		if (ImGui::Button("Change Team")) RequestChangeTeam();
		
		if ((*m_GameState) == GameState::Lobby)
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
	// update simulation time
	m_SimulationTime += dt;

	// handle tcp traffic
	if (m_ConnectionState == ConnectionState::Connected)
	{
		// handle network messages
		ProcessIncomingUdp();
		ProcessOutgoingUdp(dt);
		ProcessIncomingTcp();

		// update game state duration
		if (m_RemainingGameStateDuration > 0.0f)
			m_RemainingGameStateDuration -= dt;
		else
			m_RemainingGameStateDuration = 0.0f;

		// update all network players
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
		else if (status == sf::Socket::Error)
		{
			LOG_ERROR("Error occurred while trying to receive data from server");
		}
		else if (status == sf::Socket::Disconnected)
		{
			LOG_WARN("Connection unexpectedly disconnected while connecting to server. Cleaning up...");
			OnDisconnect();
		}
	}
}


#pragma region Messages To Server

void NetworkSystem::Connect()
{
	// catch errors
	if (Connected())
	{
		LOG_WARN("Attempting to connect while already connected!");
		return;
	}

	// attempt to connect to server
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
	if (!Connected()) return;

	// request to disconnect from the server
	MessageHeader header = CreateHeader(MessageCode::Disconnect);
	sf::Packet packet;
	packet << header;
	SendPacketToServerTcp(packet);
}

void NetworkSystem::RequestGameStart()
{
	if (!Connected()) return;

	// request for the game to start
	MessageHeader header = CreateHeader(MessageCode::GameStart);
	sf::Packet p;
	p << header;
	SendPacketToServerTcp(p);

	m_GameStartRequested = true;
}

void NetworkSystem::RequestChangeTeam()
{
	if (!Connected()) return;

	// request to change team
	MessageHeader header = CreateHeader(MessageCode::ChangeTeam);
	sf::Packet packet;
	packet << header;
	SendPacketToServerTcp(packet);
}

void NetworkSystem::RequestShoot(const sf::Vector2f& position, const sf::Vector2f& direction)
{
	// only request to shoot when connected
	if (!Connected()) return;

	MessageHeader header = CreateHeader(MessageCode::Shoot);

	// create shoot request
	ShootMessage shootMessage
	{
		INVALID_PROJECTILE_ID, // will be assigned by server
		m_ClientID,
		m_Player->GetTeam(), 
		position.x, position.y,
		direction.x, direction.y,
		m_SimulationTime
	};

	// send shoot request
	sf::Packet packet;
	packet << header << shootMessage;
	SendPacketToServerTcp(packet);

	// spawn the local copy of the projectile
	// this is to avoid the player feeling like there is lag behind their actions 
	Projectile* localProjectile = new Projectile(shootMessage.id, shootMessage.team, { shootMessage.x, shootMessage.y }, { shootMessage.dirX, shootMessage.dirY });
	m_Projectiles->push_back(localProjectile);
	m_LocalProjectiles.push(localProjectile);

	(*m_Ammo)--;
}

void NetworkSystem::RequestPlaceBlock(const sf::Vector2f& position)
{
	// only request to place when connected
	if (!Connected()) return;

	// create place message
	MessageHeader header = CreateHeader(MessageCode::Place);
	PlaceMessage placeMessage
	{
		INVALID_BLOCK_ID, // will be assigned by server
		m_ClientID,
		m_Player->GetTeam(),
		position.x, position.y
	};
	
	// send message to server
	sf::Packet packet;
	packet << header << placeMessage;
	SendPacketToServerTcp(packet);

	// create a local copy of the block
	// this is to avoid the player feeling the latency between them and the server
	// the server will later confirm or deny if this block is valid
	Block* localBlock = new Block(placeMessage.id, placeMessage.team, { placeMessage.x, placeMessage.y });
	m_Blocks->push_back(localBlock);
	m_LocalBlocks.push(localBlock);

	(*m_BuildModeBlocks)--;
}

void NetworkSystem::SyncSimulationTime()
{
	// request the server for the current simulation time
	// this takes into account the latency
	// it is measured and subtracted from the time the server tells us
	MessageHeader header = CreateHeader(MessageCode::GetServerTime);
	sf::Packet packet;
	packet << header;
	SendPacketToServerTcp(packet);
	m_LatencyPingBegin = m_SimulationTime;
}

#pragma endregion

#pragma region Handling Network Traffic

void NetworkSystem::ProcessIncomingUdp()
{
	// check for incoming udp data
	sf::Packet packet;
	sf::IpAddress fromAddr;
	unsigned short fromPort;
	sf::Socket::Status status = m_UdpSocket.receive(packet, fromAddr, fromPort);
	if (status == sf::Socket::Done)
	{
		// data was recieved
		// unpack
		MessageHeader header;
		packet >> header;

		// safety checks
		if (header.clientID != m_ClientID)
		{
			LOG_WARN("Received message addressed to a different client!");
			return;
		}

		// decide what to do based on the message
		switch (header.messageCode)
		{
		case MessageCode::Update:		OnRecieveUpdate(packet);						break;
		case MessageCode::Ping:			SendPing();										break;
		default:						LOG_WARN("Received unexpected message code!");	break;
		}
	}
	else if (status == sf::Socket::Error)
	{
		LOG_ERROR("Error occurred while trying to receive from server");
	}
}


void NetworkSystem::ProcessOutgoingUdp(float dt)
{
	// send periodic updates to the server
	m_UpdateTimer += dt;
	if (m_UpdateTimer > UPDATE_FREQUENCY)
	{
		m_UpdateTimer -= UPDATE_FREQUENCY;

		// calculate the time difference since the last update was sent
		float dt = std::max(0.0f, m_SimulationTime - m_LastUpdateTime);
		m_LastUpdateTime = m_SimulationTime;

		// create update message
		MessageHeader header = CreateHeader(MessageCode::Update);
		auto playerPos = m_Player->getPosition();
		UpdateMessage messageBody
		{
			m_ClientID,
			playerPos.x, playerPos.y,
			m_Player->getRotation(),
			dt,
			m_SimulationTime
		};

		// send to server
		sf::Packet packet;
		packet << header << messageBody;
		SendPacketToServerUdp(packet);
	}
}

void NetworkSystem::ProcessIncomingTcp()
{
	// check for any incoming data on the tcp socket
	sf::Packet packet;
	auto status = m_TcpSocket.receive(packet);
	if (status == sf::Socket::Done)
	{
		// received data
		// unpack
		MessageHeader header;
		packet >> header;

		// safety checks
		if (header.clientID != m_ClientID)
		{
			LOG_ERROR("Recieved message addressed to a different client!");
			return;
		}

		// call the appropriate callback depending on the message header
		switch (header.messageCode)
		{
		case MessageCode::Disconnect:			OnDisconnect			();						break;
		case MessageCode::PlayerConnected:		OnOtherPlayerConnect	(packet);				break;
		case MessageCode::PlayerDisconnected:	OnOtherPlayerDisconnect	(packet);				break;
		case MessageCode::ChangeTeam:			OnPlayerChangeTeam		(packet);				break;
		case MessageCode::GetServerTime:		OnServerTimeUpdate		(packet);				break;
		case MessageCode::Shoot:				OnShoot					(packet);				break;
		case MessageCode::ProjectilesDestroyed:	OnProjectilesDestroyed	(packet);				break;
		case MessageCode::ShootRequestDenied:	OnShootRequestDenied	();						break;
		case MessageCode::Place:				OnPlace					(packet);				break;
		case MessageCode::BlocksDestroyed:		OnBlocksDestroyed		(packet);				break;
		case MessageCode::PlaceRequestDenied:	OnPlaceRequestDenied	();						break;
		case MessageCode::ChangeGameState:		OnChangeGameState		(packet);				break;
		case MessageCode::TurfLineMoved:		OnTurfLineMoved			(packet);				break;
		case MessageCode::PlayerDeath:			OnPlayerDeath			();						break;
		case MessageCode::GameStart:			OnGameStart				();						break;
		default:								LOG_WARN("Recieved unexpected message code");	break;
		}
	}
	else if (status == sf::Socket::Error)
	{
		LOG_ERROR("Error occurred while trying to recieve from server");
	}
	else if (status == sf::Socket::Disconnected)
	{
		LOG_WARN("Connection unexpectedly disconnected while receiving from server. Cleaning up...");
		OnDisconnect();
	}
}


void NetworkSystem::SendPacketToServerTcp(sf::Packet& packet)
{
	// send a packet to the server via tcp
	sf::Socket::Status status;
	do
	{
		status = m_TcpSocket.send(packet);
		// repeat until the entire packet has been sent
	} while (status == sf::Socket::Partial);

	// check for errors
	if (status != sf::Socket::Done)
	{
		if (status == sf::Socket::Error)
			LOG_ERROR("Error sending packet to server!");
		else if (status == sf::Socket::Disconnected)
		{
			LOG_WARN("Connection unexpectedly disconnected while sending to server. Cleaning up...");
			OnDisconnect();
		}
	}
}

	
void NetworkSystem::SendPacketToServerUdp(sf::Packet& packet)
{
	// send a packet to the server via udp
	sf::Socket::Status status = m_UdpSocket.send(packet, m_ServerAddress, m_ServerPort);
	if (status != sf::Socket::Done)
		LOG_ERROR("Sending message to server failed!");
}

#pragma endregion

#pragma region Message Callbacks

void NetworkSystem::OnConnect(const MessageHeader& header, sf::Packet& packet)
{
	// update connection state
	m_ConnectionState = ConnectionState::Connected;
	m_ClientID = header.clientID;

	// unpack message
	ConnectMessage connectMessage;
	packet >> connectMessage;

	// update player object
	m_PlayerNumber = connectMessage.playerNumber;
	m_Player->SetTeam(connectMessage.team);
	GoToSpawn();

	// construct the other players
	for (auto i = 0; i < connectMessage.numPlayers; i++)
	{
		NetworkPlayer* newPlayer = new NetworkPlayer(connectMessage.playerIDs[i]);
		newPlayer->SetTeam(connectMessage.playerTeams[i]);
		m_NetworkPlayers->push_back(newPlayer);
	}

	// construct all blocks
	for (auto i = 0; i < connectMessage.numBlocks; i++)
	{
		Block* newBlock = new Block(connectMessage.blockIDs[i], connectMessage.blockTeams[i], { connectMessage.blockXs[i] , connectMessage.blockYs[i] });
		m_Blocks->push_back(newBlock);
	}

	// update game state
	(*m_GameState) = connectMessage.gameState;
	m_RemainingGameStateDuration = connectMessage.remainingStateDuration;
	m_ChangeTurfLineFunc(connectMessage.turfLine);

	// introduce client to server
	// this will tell the server how to contact it via udp
	MessageHeader replyHeader = CreateHeader(MessageCode::Introduction);
	IntroductionMessage replyBody{ static_cast<sf::Uint16>(m_UdpSocket.getLocalPort()) };
	sf::Packet reply;
	reply << replyHeader << replyBody;
	SendPacketToServerTcp(reply);

	// also request the simulation time
	SyncSimulationTime();

	// debug info
	LOG_INFO("Connected with ID {}", static_cast<int>(m_ClientID));
	LOG_INFO("There are {} other players already connected", connectMessage.numPlayers);
	LOG_INFO("I am player {}", m_PlayerNumber);
}


void NetworkSystem::OnDisconnect()
{
	// the client has been told to disconnect from the server
	m_TcpSocket.disconnect();
	m_ConnectionState = ConnectionState::Disconnected;
	m_ClientID = INVALID_CLIENT_ID;

	// reset all game state
	m_SimulationTime = 0.0f;
	m_Player->SetTeam(PlayerTeam::None);
	m_GameStartRequested = false;
	(*m_GameState) = GameState::Lobby;
	m_RemainingGameStateDuration = 0;

	// delete game objects
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

void NetworkSystem::OnOtherPlayerConnect(sf::Packet& packet)
{
	// another player has joined the game
	
	// extract message body
	PlayerConnectedMessage messageBody;
	packet >> messageBody;

	LOG_INFO("Player ID {} has joined", messageBody.playerID);

	// construct the new player
	NetworkPlayer* newPlayer = new NetworkPlayer(messageBody.playerID);
	newPlayer->SetTeam(messageBody.team);
	m_NetworkPlayers->push_back(newPlayer);
}

void NetworkSystem::OnOtherPlayerDisconnect(sf::Packet& packet)
{
	// a player has left the game

	// extract message body
	PlayerDisconnectedMessage messageBody;
	packet >> messageBody;

	LOG_INFO("Player ID {} has left", messageBody.playerID);

	// find the player to delete
	auto it = m_NetworkPlayers->begin();
	for (; it != m_NetworkPlayers->end(); it++)
	{
		if ((*it)->GetID() == messageBody.playerID) break;
	}
	// make sure we found them
	if (it != m_NetworkPlayers->end())
	{
		// delete the player
		delete (*it);
		m_NetworkPlayers->erase(it);
	}
	else
		LOG_WARN("Player {} doesn't exist!", messageBody.playerID);
}

void NetworkSystem::OnRecieveUpdate(sf::Packet& packet)
{
	// the client has recieved an update telling it about all the other players in the game

	UpdateMessage messageBody;

	// packet will potentially contain updates for multiple players
	// loop while the packet still contains data
	while (!packet.endOfPacket())
	{
		packet >> messageBody;

		// we don't need to be updated about ourselves
		if (messageBody.playerID == m_ClientID) continue;

		// update the player
		NetworkPlayer* player = FindNetworkPlayerWithID(messageBody.playerID);
		if (player) player->NetworkUpdate(messageBody, m_SimulationTime);
	}
}

void NetworkSystem::OnPlayerChangeTeam(sf::Packet& packet)
{
	// a player has changed team

	ChangeTeamMessage messageBody;
	packet >> messageBody;

	// update the player that has changed team
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

void NetworkSystem::OnServerTimeUpdate(sf::Packet& packet)
{
	// measure round trip time
	float latency = m_SimulationTime - m_LatencyPingBegin;

	// get the servers simulation time
	ServerTimeMessage messageBody;
	packet >> messageBody;
	// our simulation time is the server's time minus half the latency
	m_SimulationTime = messageBody.serverTime - (0.5f * latency);
}

void NetworkSystem::OnShoot(sf::Packet& packet)
{
	// extract message body
	ShootMessage shootMessage;
	packet >> shootMessage;

	// did we shoot this projectile
	if (shootMessage.shotBy == m_ClientID)
	{
		// this means that our last shoot request was confirmed
		// (shoot requests are always sent via tcp so the order theyre confirmed in must be the same as the order they were requested)
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

void NetworkSystem::OnProjectilesDestroyed(sf::Packet& packet)
{
	// one or more projectiles have been destroyed

	// unpack message
	ProjectilesDestroyedMessage message;
	packet >> message;

	// find out which projectile has been destroyed
	for (auto it = m_Projectiles->begin(); it != m_Projectiles->end();)
	{
		bool projDestroyed = false;
		for (auto i = 0; i < message.count; i++)
		{
			if ((*it)->GetID() == message.ids[i])
			{
				// delete projectile
				it = m_Projectiles->erase(it);
				projDestroyed = true;
				break;
			}
		}
		if (!projDestroyed)
			it++;
	}
}

void NetworkSystem::OnShootRequestDenied()
{
	// this shouldn't occur but test just in case
	if (m_LocalProjectiles.empty()) return;

	// find which local projectile needs to be deleted
	for (auto it = m_Projectiles->begin(); it != m_Projectiles->end(); it++)
	{
		if (*it == m_LocalProjectiles.front())
		{
			// remove the projectile
			m_Projectiles->erase(it);
			break;
		}
	}

	// delete the projectile
	delete m_LocalProjectiles.front();
	m_LocalProjectiles.pop();

	(*m_Ammo)++;
}

void NetworkSystem::OnPlace(sf::Packet& packet)
{
	// a block has been placed

	// unpack message
	PlaceMessage placeMessage;
	packet >> placeMessage;

	// create block
	if (placeMessage.placedBy == m_ClientID)
	{
		// confirmation of our place request
		// assign the blocks id and remove it from the locals queue
		m_LocalBlocks.front()->UpdateID(placeMessage.id);
		m_LocalBlocks.pop();
	}
	else
	{
		// this block was placed by someone else, create it
		Block* newBlock = new Block(placeMessage.id, placeMessage.team, { placeMessage.x, placeMessage.y });
		m_Blocks->push_back(newBlock);
	}
}

void NetworkSystem::OnBlocksDestroyed(sf::Packet& packet)
{
	// one or more block were destroyed

	BlocksDestroyedMessage message;
	packet >> message;

	// find which blocks were destroyed and destroy them
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

void NetworkSystem::OnPlaceRequestDenied()
{
	// our place request has been denied
	// delete the last block we placed

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
	(*m_BuildModeBlocks)++;
}

void NetworkSystem::OnChangeGameState(sf::Packet& packet)
{
	// the game state has changed

	ChangeGameStateMessage message;
	packet >> message;

	// update state and duration
	(*m_GameState) = message.state;
	m_RemainingGameStateDuration = message.stateDuration;

	// delete all projectiles
	for (auto projectile : *m_Projectiles)
		delete projectile;
	m_Projectiles->clear();

	if ((*m_GameState) == GameState::Lobby)
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
		
		(*m_BuildModeBlocks) = INITIAL_BUILD_MODE_BLOCKS;
	}
	else
	{
		// reset ammo/blocks
		(*m_BuildModeBlocks) = SUBSEQUENT_BUILD_MODE_BLOCKS;
		(*m_Ammo) = MAX_AMMO_HELD;
	}
}

void NetworkSystem::OnTurfLineMoved(sf::Packet& packet)
{
	// move turf line
	TurfLineMoveMessage message;
	packet >> message;

	m_ChangeTurfLineFunc(message.newTurfLine);
}

void NetworkSystem::OnPlayerDeath()
{
	// this player has died; return to spawn
	GoToSpawn();
}

void NetworkSystem::OnGameStart()
{
	// setup initial game state
	GoToSpawn();

	(*m_GameState) = GameState::BuildMode;
	m_RemainingGameStateDuration = INITIAL_BUILD_MODE_DURATION;
	(*m_BuildModeBlocks) = INITIAL_BUILD_MODE_BLOCKS;

	m_ChangeTurfLineFunc(0.5f * WORLD_WIDTH);

	m_GameStartRequested = false;
}

void NetworkSystem::SendPing()
{
	// the server pings the clients to measure latency
	// we don't need to give it any data, just telling the server were awake is plenty
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
