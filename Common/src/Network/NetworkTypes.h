#pragma once

#include "Constants.h"
#include "CommonTypes.h"


using ClientID = sf::Uint8;
const ClientID INVALID_CLIENT_ID = (ClientID)(-1);
const ClientID MAX_CLIENT_ID = INVALID_CLIENT_ID - 1;


enum class MessageCode : sf::Uint8
{
	Connect,				// Confirm connection to server (S->C)
	Introduction,			// Introduce the server to the client (C->S)
	Disconnect,				// Request to disconnect from server/Confirm disconnection from server (C<->S)
	PlayerConnected,		// Announce a new player has connected (S->C)
	PlayerDisconnected,		// Announce a player has disconnected (S->C)
	
	Update,					// Send player position/rotation update (C<->S)
	ChangeTeam,				// Announce a player wants to/has changed team (C<->S)
	ChangeGameState,		// Announce a change in game state (S->C)
	TurfLineMoved,			// Announce the turf line has moved (S->C)
	
	Shoot,					// Request to shoot/Announce a projectile has been shot (S<->C)
	ShootRequestDenied,		// Announce a clients request to shoot a projectile has been denied (S->C)
	Place,					// Request to place/Announce a block has been placed (S<->C)
	PlaceRequestDenied,		// Announce a clients request to place a block has been denied (S->C)
	
	GameStart,				// Announce player is ready to start/the game has started (S<->C)
	PlayerDeath,			// Announce a player has died (S->C)
	ProjectilesDestroyed,	// Announce a projectile has been destroyed (S->C)
	BlocksDestroyed,		// Announce a block has been destroyed (S->C)
	
	GetServerTime,			// Calculate latency between client and server to update clients simulation timer (C<->S)
	Ping					// Calculate a clients latency
};
sf::Packet& operator <<(sf::Packet& packet, const MessageCode& mc);
sf::Packet& operator >>(sf::Packet& packet, MessageCode& mc);


// MESSAGE TYPES
struct MessageHeader
{
	ClientID clientID;
	MessageCode messageCode;
};
sf::Packet& operator <<(sf::Packet& packet, const MessageHeader& header);
sf::Packet& operator >>(sf::Packet& packet, MessageHeader& header);


struct ConnectMessage
{
	// info to be given to the newly joining player
	sf::Uint8 playerNumber;
	PlayerTeam team;
	// info about the players already in the game
	sf::Uint8 numPlayers;
	ClientID playerIDs[MAX_NUM_PLAYERS];
	PlayerTeam playerTeams[MAX_NUM_PLAYERS];
	
	// info about projectiles already in game
	sf::Uint8 numProjectiles;

	// info about blocks already in game
	sf::Uint8 numBlocks;
	BlockID blockIDs[MAX_NUM_BLOCKS];
	PlayerTeam blockTeams[MAX_NUM_BLOCKS];
	float blockXs[MAX_NUM_BLOCKS];
	float blockYs[MAX_NUM_BLOCKS];

	// info about the game state
	GameState gameState;
	float remainingStateDuration;

	float turfLine;
};
sf::Packet& operator <<(sf::Packet& packet, const ConnectMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ConnectMessage& message);

struct IntroductionMessage
{
	sf::Uint16 udpPort;
};
sf::Packet& operator <<(sf::Packet& packet, const IntroductionMessage& message);
sf::Packet& operator >>(sf::Packet& packet, IntroductionMessage& message);

struct PlayerConnectedMessage
{
	ClientID playerID;
	PlayerTeam team;
};
sf::Packet& operator <<(sf::Packet& packet, const PlayerConnectedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, PlayerConnectedMessage& message);

struct PlayerDisconnectedMessage
{
	ClientID playerID;
};
sf::Packet& operator <<(sf::Packet& packet, const PlayerDisconnectedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, PlayerDisconnectedMessage& message);

struct UpdateMessage
{
	ClientID playerID; // the client that the update data pertains to
	float x;
	float y;
	float rotation;
	float dt;
	float sendTime;
};
sf::Packet& operator <<(sf::Packet& packet, const UpdateMessage& message);
sf::Packet& operator >>(sf::Packet& packet, UpdateMessage& message);

struct ChangeTeamMessage
{
	ClientID playerID; // the client that changed team
	PlayerTeam team; // their new team
};
sf::Packet& operator <<(sf::Packet& packet, const ChangeTeamMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ChangeTeamMessage& message);

struct ServerTimeMessage
{
	float serverTime;
};
sf::Packet& operator <<(sf::Packet& packet, const ServerTimeMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ServerTimeMessage& message);

struct ShootMessage
{
	ProjectileID id;
	ClientID shotBy; // the client that shot the projectile
	PlayerTeam team; // the team the projectile belongs to
	float x; // projectile origin
	float y;
	float dirX; // projectile direction
	float dirY; 
	float shootTime;
};
sf::Packet& operator <<(sf::Packet& packet, const ShootMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ShootMessage& message);

struct ProjectilesDestroyedMessage
{
	sf::Uint8 count;
	ProjectileID ids[MAX_NUM_PROJECTILES]; // id of the destroyed projectile
};
sf::Packet& operator <<(sf::Packet& packet, const ProjectilesDestroyedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ProjectilesDestroyedMessage& message);

struct PlaceMessage
{
	BlockID id;
	ClientID placedBy;
	PlayerTeam team;
	float x;
	float y;
};
sf::Packet& operator <<(sf::Packet& packet, const PlaceMessage& message);
sf::Packet& operator >>(sf::Packet& packet, PlaceMessage& message);

struct BlocksDestroyedMessage
{
	sf::Uint8 count;
	BlockID ids[MAX_NUM_BLOCKS];
};
sf::Packet& operator <<(sf::Packet& packet, const BlocksDestroyedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, BlocksDestroyedMessage& message);

struct ChangeGameStateMessage
{
	GameState state;
	float stateDuration;
};
sf::Packet& operator <<(sf::Packet& packet, const ChangeGameStateMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ChangeGameStateMessage& message);

struct TurfLineMoveMessage
{
	float newTurfLine;
};
sf::Packet& operator <<(sf::Packet& packet, const TurfLineMoveMessage& message);
sf::Packet& operator >>(sf::Packet& packet, TurfLineMoveMessage& message);


struct PlayerStateFrame
{
	sf::Vector2f position;
	float rotation;
	float dt;
	float sendTimestamp; // used for ordering player state frames

	PlayerStateFrame(const UpdateMessage& m)
	{
		position.x = m.x;
		position.y = m.y;
		rotation = m.rotation;
		dt = m.dt;
		sendTimestamp = m.sendTime;
	}
};
