#include "ServerApplication.h"

#include "Log.h"
#include "MathUtils.h"
#include "Network/NetworkTypes.h"


ServerApplication::ServerApplication()
{
	LOG_INFO("----Server----");
	LOG_INFO("Local IP: {}", sf::IpAddress::getLocalAddress().toString());

	m_ListenSocket.setBlocking(false);
	m_UdpSocket.setBlocking(false);

	if (m_UdpSocket.bind(SERVER_PORT) != sf::Socket::Done)
	{
		LOG_ERROR("Server failed to bind to port {}", SERVER_PORT);
	}
	LOG_INFO("UDP: listening on port {}", SERVER_PORT);

	if (m_ListenSocket.listen(SERVER_PORT) != sf::Socket::Done)
	{
		LOG_ERROR("Server failed to listen on port {}", SERVER_PORT);
	}
	LOG_INFO("TCP: listening on port {}", SERVER_PORT);
	LOG_INFO("--------------");

	for (ClientID id = 0; id < INVALID_CLIENT_ID; id++)
		m_NextClientID.push(id);

	m_Clients.reserve(MAX_NUM_PLAYERS);

	m_NewConnection = new Connection;


	// create the blocks around spawn
	const int blockCount = 11;
	for (int i = 0; i < blockCount; i++)
	{
		m_Blocks.push_back(new BlockState{ NextBlockID(), PlayerTeam::None, { SPAWN_WIDTH - 0.5f * BLOCK_SIZE,				 0.5f * WORLD_HEIGHT - (BLOCK_SIZE * (blockCount / 2)) + BLOCK_SIZE * i } });
		m_Blocks.push_back(new BlockState{ NextBlockID(), PlayerTeam::None, { WORLD_WIDTH - SPAWN_WIDTH + 0.5f * BLOCK_SIZE, 0.5f * WORLD_HEIGHT - (BLOCK_SIZE * (blockCount / 2)) + BLOCK_SIZE * i } });
	}

}

ServerApplication::~ServerApplication()
{
	// delete all objects in the game world
	for (auto projectile : m_Projectiles)
		delete projectile;
	for (auto block : m_Blocks)
		delete block;

	delete m_NewConnection;
	// disconnect all clients
	for (auto client : m_Clients)
		delete client;
}

void ServerApplication::Run()
{
	m_ServerClock.restart();

	while (true)
	{
		float lastSimTime = m_SimulationTime;
		m_SimulationTime = m_ServerClock.getElapsedTime().asMilliseconds() / 1000.0f;
		float dt = m_SimulationTime - lastSimTime;

		SimulateGameObjects(dt);
		UpdateGameState(dt);

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
				// this will send invalid client id back to the new client
				m_NewConnection->SendMessageTcp(MessageCode::Connect);
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
				case MessageCode::GetServerTime:		ProcessGetServerTime(client, header, packet); break;
				case MessageCode::Shoot:				ProcessShootRequest(client, header, packet); break;
				case MessageCode::Place:				ProcessPlaceRequest(client, header, packet); break;
				case MessageCode::GameStart:			ProcessGameStartRequest(client, header, packet); break;
					// these messages are sent from the server to clients, so it would be incorrect for the server to recieve them
				case MessageCode::Connect:
				case MessageCode::PlayerConnected:
				case MessageCode::PlayerDisconnected:
				case MessageCode::ShootRequestDenied:
				case MessageCode::PlaceRequestDenied:
				case MessageCode::PlayerDeath:
				case MessageCode::ProjectilesDestroyed:
				case MessageCode::BlocksDestroyed:
				case MessageCode::ChangeGameState:
				case MessageCode::TurfLineMoved:
														LOG_WARN("Received invalid message code"); break;
				case MessageCode::Update:						  
														LOG_WARN("Received update message via TCP; updates should be sent via UDP"); break;

				default:								LOG_WARN("Unknown message code: {}", static_cast<int>(header.messageCode)); break;
				}
			}
			else if (status == sf::Socket::Error)
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


void ServerApplication::SimulateGameObjects(float dt)
{
	bool gameOver = false;

	// simulate projectiles
	for (auto proj_it = m_Projectiles.begin(); proj_it != m_Projectiles.end();)
	{
		auto projectile = *proj_it;

		projectile->Step(dt);

		// check if the projectile has hit a block
		bool hitBlock = false;
		for (auto block_it = m_Blocks.begin(); block_it != m_Blocks.end(); block_it++)
		{
			auto block = *block_it;

			if (BlockProjectileCollision(block, projectile))
			{
				hitBlock = true;

				if (block->team != projectile->team && block->team != PlayerTeam::None)
				{
					DestroyBlock(block);
					m_Blocks.erase(block_it);
				}

				break;
			}
		}

		// check if this projectile has hit a player
		bool hitPlayer = false;
		for (auto client : m_Clients)
		{
			auto player = client->GetPlayerState();
			if (PlayerProjectileCollision(&player, projectile))
			{
				if (player.team != projectile->team)
				{
					hitPlayer = true;

					// kill player
					client->SendMessageTcp(MessageCode::PlayerDeath);

					// move turf line
					m_TurfLine += BLOCK_SIZE * (projectile->team == PlayerTeam::Red ? 1 : - 1);
					// check win condition
					if (m_TurfLine <= SPAWN_WIDTH || m_TurfLine >= WORLD_WIDTH - SPAWN_WIDTH)
					{
						EndGame();
						gameOver = true;
					}

					// transmit turf move to all players
					for (auto c2 : m_Clients)
					{
						TurfLineMoveMessage message{ m_TurfLine };
						c2->SendMessageTcp(MessageCode::TurfLineMoved, message);
					}

					// moving the turf line may destroy a bunch of blocks
					CheckForBlocksAcrossTurfLine();
				}

				break;
			}
		}
		if (gameOver) break;

		if (hitBlock || hitPlayer || projectile->position.x - PROJECTILE_RADIUS < 0 || projectile->position.x + PROJECTILE_RADIUS > WORLD_WIDTH
								  || projectile->position.y - PROJECTILE_RADIUS < 0 || projectile->position.y + PROJECTILE_RADIUS > WORLD_HEIGHT)
		{
			// projectile is out of bounds
			DestroyProjectile(projectile);
			proj_it = m_Projectiles.erase(proj_it);
		}
		else
			proj_it++;
	}
}

void ServerApplication::UpdateGameState(float dt)
{
	if (m_GameState == GameState::Lobby) return;

	m_StateTimer += dt;

	if (m_StateTimer > m_StateDuration)
	{
		// switch state!
		m_StateTimer = 0.0f;

		switch (m_GameState)
		{
		case GameState::Lobby: break;
		case GameState::FightMode:
			m_GameState = GameState::BuildMode;
			m_StateDuration = m_BuildModeDuration;
			m_FightModeDuration = std::max(m_FightModeDuration - 10.0f, MIN_FIGHT_MODE_DURATION);

			break;
		case GameState::BuildMode:	
			m_GameState = GameState::FightMode;
			m_StateDuration = m_FightModeDuration;
			m_BuildModeDuration = std::max(m_BuildModeDuration - 10.0f, MIN_BUILD_MODE_DURATION);

			break;
		default:
			LOG_ERROR("Unknown game state"); break;
		}

		// kill all projectiles
		for (auto projectile : m_Projectiles)
			delete projectile;
		m_Projectiles.clear();

		// tell all clients
		for (auto client : m_Clients)
		{
			ChangeGameStateMessage message{ m_GameState, m_StateDuration };
			client->SendMessageTcp(MessageCode::ChangeGameState, message);
		}
	}

}

void ServerApplication::StartGame()
{
	m_GameState = GameState::BuildMode;
	m_StateDuration = INITIAL_BUILD_MODE_DURATION;

	m_BuildModeDuration = INITIAL_BUILD_MODE_DURATION;
	m_FightModeDuration = INITIAL_FIGHT_MODE_DURATION;

	// reset turf line
	m_TurfLine = 0.5f * WORLD_WIDTH;

	// tell all clients the game has started
	for (auto client : m_Clients)
	{
		client->SendMessageTcp(MessageCode::GameStart);
		// reset ready flag
		client->SetReady(false);
	}
}

void ServerApplication::EndGame()
{
	m_GameState = GameState::Lobby;
	m_StateDuration = 0.0f;
	m_StateTimer = 0.0f;

	for (auto client : m_Clients)
	{
		ChangeGameStateMessage message{ m_GameState, m_StateDuration };
		client->SendMessageTcp(MessageCode::ChangeGameState, message);
		// reset ready flag
		client->SetReady(false);
	}

	// kill all projectiles
	for (auto projectile : m_Projectiles)
		delete projectile;
	m_Projectiles.clear();
	// kill all blocks placed by players
	for (auto it = m_Blocks.begin(); it != m_Blocks.end();)
	{
		if ((*it)->team != PlayerTeam::None)
		{
			delete (*it);
			it = m_Blocks.erase(it);
		}
		else it++;
	}
}

void ServerApplication::DestroyProjectile(ProjectileState* projectile)
{
	// tell all clients that this projectile has been destroyed
	ProjectilesDestroyedMessage message;
	message.count = 1;
	message.ids[0] = projectile->id;

	for (auto client : m_Clients)
		client->SendMessageTcp(MessageCode::ProjectilesDestroyed, message);
}

void ServerApplication::DestroyBlock(BlockState* block)
{
	// tell all clients that this block has been destroyed
	BlocksDestroyedMessage message;
	message.count = 1;
	message.ids[0] = block->id;

	for (auto client : m_Clients)
		client->SendMessageTcp(MessageCode::BlocksDestroyed, message);
}

void ServerApplication::ProcessConnect()
{
	ClientID newClientID = NextClientID();

	// setup connection object
	m_NewConnection->OnTcpConnected(newClientID);

	// tell the client their ID
	ConnectMessage connectMessage;

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
	connectMessage.team = state.team;

	// tell them about the current world state
	connectMessage.numPlayers = static_cast<sf::Uint8>(m_Clients.size());
	for (size_t i = 0; i < m_Clients.size(); i++)
	{
		connectMessage.playerIDs[i] = m_Clients[i]->GetID();

		PlayerState& s = m_Clients[i]->GetPlayerState();
		connectMessage.playerTeams[i] = s.team;
	}
	connectMessage.numBlocks = static_cast<sf::Uint8>(m_Blocks.size());
	for (size_t i = 0; i < m_Blocks.size(); i++)
	{
		connectMessage.blockIDs[i] = m_Blocks[i]->id;
		connectMessage.blockTeams[i] = m_Blocks[i]->team;
		connectMessage.blockXs[i] = m_Blocks[i]->position.x;
		connectMessage.blockYs[i] = m_Blocks[i]->position.y;
	}
	connectMessage.gameState = m_GameState;
	connectMessage.remainingStateDuration = m_StateDuration - m_StateTimer;
	connectMessage.turfLine = m_TurfLine;

	m_NewConnection->SendMessageTcp(MessageCode::Connect, connectMessage);

	// tell all other clients a new player has connected
	for (auto& c : m_Clients)
	{
		PlayerConnectedMessage playerConnectedMessage{ newClientID, state.team };
		c->SendMessageTcp(MessageCode::PlayerConnected, playerConnectedMessage);
	}

	m_Clients.push_back(m_NewConnection);
	LOG_INFO("Player connected with ID {}", newClientID);

	m_NewConnection = new Connection;
}

void ServerApplication::ProcessIntroduction(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	IntroductionMessage introductionMessage;
	packet >> introductionMessage;
	client->SetUdpPort(introductionMessage.udpPort);
}



void ServerApplication::ProcessDisconnect(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	// acknowledge the clients requests to disconnect
	client->SendMessageTcp(MessageCode::Disconnect);

	m_NextClientID.push(client->GetID());
	if (client->GetPlayerState().team == PlayerTeam::Red)
		m_RedTeamPlayerCount--;
	else
		m_BlueTeamPlayerCount--;

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
		PlayerDisconnectedMessage playerDisconnectedMessage{ client->GetID() };
		c->SendMessageTcp(MessageCode::PlayerDisconnected, playerDisconnectedMessage);
	}

	// finally delete the client
	LOG_INFO("Player ID {} disconnected", client->GetID());
	delete client;
}

void ServerApplication::ProcessUpdate(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	// unpack packet
	UpdateMessage updateMessage;
	packet >> updateMessage;

	if (updateMessage.playerID != client->GetID())
	{
		LOG_WARN("Client sending update data with incorrect client ID");
		return;
	}

	// optimization: check for any changes here before re-sending to all other clients
	PlayerState& state = client->GetPlayerState();
	state.Update(updateMessage);

	// send out to all other clients
	for (auto& c : m_Clients)
	{
		if (c == client) continue;
		SendMessageToClientUdp(c, MessageCode::Update, updateMessage);
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

	ChangeTeamMessage changeTeamMessage{ client->GetID(), state.team };

	// transmit this change to all clients
	for (auto c : m_Clients)
		c->SendMessageTcp(MessageCode::ChangeTeam, changeTeamMessage);
}

void ServerApplication::ProcessGetServerTime(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	ServerTimeMessage response{ m_SimulationTime };
	client->SendMessageTcp(MessageCode::GetServerTime, response);
}

void ServerApplication::ProcessShootRequest(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	ShootMessage shootMessage;
	packet >> shootMessage;

	// check if the projectile can be spawned
	if (VerifyProjecitleShoot({ shootMessage.x, shootMessage.y }, client->GetPlayerState()))
	{
		// create new projectile object
		shootMessage.id = NextProjectileID();

		ProjectileState* newProjectile = new ProjectileState(shootMessage);
		m_Projectiles.push_back(newProjectile);

		// tell all clients a projectile has been shot
		for (auto c : m_Clients)
			c->SendMessageTcp(MessageCode::Shoot, shootMessage);
	}
	else
	{
		// tell the player their request has been denied
		client->SendMessageTcp(MessageCode::ShootRequestDenied);
	}
}

void ServerApplication::ProcessPlaceRequest(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	PlaceMessage placeMessage;
	packet >> placeMessage;

	// check if block can be placed
	if (VerifyBlockPlacement({ placeMessage.x, placeMessage.y }, client->GetPlayerState()))
	{
		// create new block
		placeMessage.id = NextBlockID();
		
		BlockState* newBlock = new BlockState(placeMessage);
		m_Blocks.push_back(newBlock);

		for (auto c : m_Clients)
			c->SendMessageTcp(MessageCode::Place, placeMessage);
	}
	else
	{
		// tell the player their block place request has been rejected
		client->SendMessageTcp(MessageCode::PlaceRequestDenied);
	}
}

void ServerApplication::ProcessGameStartRequest(Connection* client, const MessageHeader& header, sf::Packet& packet)
{
	if (m_GameState != GameState::Lobby)
	{
		LOG_WARN("Cannot request game to start outside of lobby state!");
		return;
	}

	client->SetReady(true);

	// if all clients are ready then start the game
	bool allReady = true;
	for (auto c : m_Clients)
		allReady &= c->IsReady();

	if (allReady)
		StartGame();
}


Connection* ServerApplication::FindClientWithID(ClientID id)
{
	for (auto& connection : m_Clients)
	{
		if (connection->GetID() == id) return connection;
	}
	return nullptr;
}

ClientID ServerApplication::NextClientID()
{
	ClientID id = m_NextClientID.front();
	m_NextClientID.pop();
	return id;
}

ProjectileID ServerApplication::NextProjectileID()
{
	return m_NextProjectileID++;
}

BlockID ServerApplication::NextBlockID()
{
	return m_NextBlockID++;
}

bool ServerApplication::VerifyProjecitleShoot(const sf::Vector2f& position, const PlayerState& player)
{
	// is the game not in fight mode
	if (m_GameState != GameState::FightMode) return false;

	return true;
}

bool ServerApplication::VerifyBlockPlacement(const sf::Vector2f& position, const PlayerState& player)
{
	// is the game not in build mode
	if (m_GameState != GameState::BuildMode) return false;
	// is the player close enough to the place position
	if (Length(position - player.position) > BLOCK_PLACE_RADIUS) return false;
	// is the block on the players own turf
	if (!OnTeamTurf(position, player.team)) return false;
	// is the block on top of any other players
	for (auto c : m_Clients)
		if (Length(position - c->GetPlayerState().position) < PLAYER_SIZE) return false;
	// is the block on top of any other blocks
	for (auto b : m_Blocks)
		if (position == b->position) return false;

	return true;
}

bool ServerApplication::OnTeamTurf(const sf::Vector2f& p, PlayerTeam team)
{
	switch (team)
	{
	case PlayerTeam::None:	return true; // always allow team-less blocks (because theyll be server spawned)
	case PlayerTeam::Red:	return p.x < m_TurfLine && p.x > SPAWN_WIDTH;
	case PlayerTeam::Blue:	return p.x > m_TurfLine && p.x < WORLD_WIDTH - SPAWN_WIDTH;
	default:				return false;
	}

	return false;
}

void ServerApplication::CheckForBlocksAcrossTurfLine()
{
	BlocksDestroyedMessage blocksDestroyedMessage;
	blocksDestroyedMessage.count = 0;

	for (auto it = m_Blocks.begin(); it != m_Blocks.end();)
	{
		auto block = *it;

		if (!OnTeamTurf(block->position, block->team))
		{
			blocksDestroyedMessage.ids[blocksDestroyedMessage.count++] = block->id;
			
			delete block;
			it = m_Blocks.erase(it);
		}
		else
			it++;
	}

	if (blocksDestroyedMessage.count > 0)
	{
		for (auto client : m_Clients)
			client->SendMessageTcp(MessageCode::BlocksDestroyed, blocksDestroyedMessage);
	}
}
