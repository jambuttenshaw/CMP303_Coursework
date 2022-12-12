#pragma once

#include "Constants.h"
#include "CommonTypes.h"


// a unique identifier assigned to a client
using ClientID = sf::Uint8;
const ClientID INVALID_CLIENT_ID = (ClientID)(-1);
// different from the max number of players that can connect: this is the max possible representable ID
const ClientID MAX_CLIENT_ID = INVALID_CLIENT_ID - 1;


// a message code is an 8 bit unsigned integer that specifies the purpose of the following message
// it is present in every message between client and server so they can identify what to do with the data they receive
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

// a message header preceeds the data in every message
// its important to identify who send the message, and what they want done with the data inside the message
struct MessageHeader
{
	ClientID clientID;
	MessageCode messageCode;
};
sf::Packet& operator <<(sf::Packet& packet, const MessageHeader& header);
sf::Packet& operator >>(sf::Packet& packet, MessageHeader& header);


// sent to a client after they connect,
// and it describes the current state of the game when they join
// so they can synchronize with the server
struct ConnectMessage
{
	// info to be given to the newly joining player
	sf::Uint8 playerNumber;
	PlayerTeam team;
	// info about the players already in the game
	sf::Uint8 numPlayers;
	ClientID playerIDs[MAX_NUM_PLAYERS];
	PlayerTeam playerTeams[MAX_NUM_PLAYERS];
	
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

// used for a client to tell the server how to contact them via udp
struct IntroductionMessage
{
	sf::Uint16 udpPort;
};
sf::Packet& operator <<(sf::Packet& packet, const IntroductionMessage& message);
sf::Packet& operator >>(sf::Packet& packet, IntroductionMessage& message);

// informs aready connected players that a new player has connected
struct PlayerConnectedMessage
{
	ClientID playerID;
	PlayerTeam team;
};
sf::Packet& operator <<(sf::Packet& packet, const PlayerConnectedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, PlayerConnectedMessage& message);

// informs all connected players that a player has disconnected
struct PlayerDisconnectedMessage
{
	ClientID playerID;
};
sf::Packet& operator <<(sf::Packet& packet, const PlayerDisconnectedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, PlayerDisconnectedMessage& message);

// contains all data about the current state of the player
// dt and sendTime are used for interpolating, predicting, and rewinding
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

// requests/confirms that a player has changed team
struct ChangeTeamMessage
{
	ClientID playerID; // the client that changed team
	PlayerTeam team; // their new team
};
sf::Packet& operator <<(sf::Packet& packet, const ChangeTeamMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ChangeTeamMessage& message);

// the clients can ask the server for the current time so they can sync their clocks with the servers
struct ServerTimeMessage
{
	float serverTime;
};
sf::Packet& operator <<(sf::Packet& packet, const ServerTimeMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ServerTimeMessage& message);

// request/confirmation that a projectile has been shot
// contains all the data about the projectile being shot
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

// tells clients that a number of projectiles have been shot
struct ProjectilesDestroyedMessage
{
	sf::Uint8 count;
	ProjectileID ids[MAX_NUM_PROJECTILES]; // ids of the destroyed projectiles
};
sf::Packet& operator <<(sf::Packet& packet, const ProjectilesDestroyedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ProjectilesDestroyedMessage& message);

// request/confirm that a block has been placed
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

// inform clients that one or more blocks have been destroyed
struct BlocksDestroyedMessage
{
	sf::Uint8 count;
	BlockID ids[MAX_NUM_BLOCKS]; // the ids of the destroyed blocks
};
sf::Packet& operator <<(sf::Packet& packet, const BlocksDestroyedMessage& message);
sf::Packet& operator >>(sf::Packet& packet, BlocksDestroyedMessage& message);

// inform clients the game state has changed
struct ChangeGameStateMessage
{
	GameState state;
	float stateDuration;
};
sf::Packet& operator <<(sf::Packet& packet, const ChangeGameStateMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ChangeGameStateMessage& message);

// inform clients the turf line has moved
struct TurfLineMoveMessage
{
	float newTurfLine;
};
sf::Packet& operator <<(sf::Packet& packet, const TurfLineMoveMessage& message);
sf::Packet& operator >>(sf::Packet& packet, TurfLineMoveMessage& message);


// a record of the players state at a certain moment in time
// by keepnig hold of the previous player state frames the players state can be calculated at any moment in the past
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
