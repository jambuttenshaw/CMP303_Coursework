#pragma once

#include <SFML/Network.hpp>
#include "CommonTypes.h"

const char* const ServerAddress = "127.0.0.1";
const unsigned short ServerPort = 4444;

const sf::Uint8 MAX_NUM_PLAYERS = 16;

// update ticks every 100ms
const float UpdateTickSpeed = 0.1f;
// the amount of time to wait before resending a message
const float ResendTimeout = 0.5f;


using ClientID = sf::Uint8;
const ClientID INVALID_CLIENT_ID = (ClientID)(-1);
const ClientID MAX_CLIENT_ID = INVALID_CLIENT_ID - 1;


enum class MessageCode : sf::Uint8
{
	Connect,			// Confirm connection to server (S->C)
	Introduction,		// Introduce the server to the client (C->S)
	Disconnect,			// Request to disconnect from server/Confirm disconnection from server (C<->S)
	PlayerConnected,	// Announce a new player has connected (S->C)
	PlayerDisconnected, // Announce a player has disconnected (S->C)
	
	Update,				// Send player position/rotation update (C<->S)
	
	ShootRequest,		// Request to shoot a projectile (C->S)
	PlaceRequest,		// Request to place a block (C->S)
	ShootRequestDenied,	// Announce a clients request to shoot a projectile has been denied (S->C)
	PlaceRequestDenied,	// Announce a clients request to place a block has been denied (S->C)
	
	Shoot,				// Announce a projectile has been shot (S->C)
	Place,				// Announce a block has been placed (S->C)
	PlayerDeath,		// Announce a player has died (S->C)
	
	LatencyPing,		// Calculate latency between client and server (C->S)
};
sf::Packet& operator <<(sf::Packet& packet, const MessageCode& mc);
sf::Packet& operator >>(sf::Packet& packet, MessageCode& mc);


// MESSAGE TYPES
struct MessageHeader
{
	ClientID clientID;
	MessageCode messageCode;
	float time;
};
sf::Packet& operator <<(sf::Packet& packet, const MessageHeader& header);
sf::Packet& operator >>(sf::Packet& packet, MessageHeader& header);


struct ConnectMessage
{
	// info to be given to the newly joining player
	PlayerTeam team;
	// info about the players already in the game
	sf::Uint8 numPlayers;
	ClientID playerIDs[MAX_NUM_PLAYERS];
	PlayerTeam playerTeams[MAX_NUM_PLAYERS];
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
};
sf::Packet& operator <<(sf::Packet& packet, const UpdateMessage& message);
sf::Packet& operator >>(sf::Packet& packet, UpdateMessage& message);


// NETWORK REPRESENTATIONS OF GAME OBJECTS
struct PlayerState
{
	PlayerTeam team;

	float x;
	float y;
	float rotation;

	void Update(const UpdateMessage&);
};
