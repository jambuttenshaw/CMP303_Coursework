#pragma once

#include <SFML/Network.hpp>

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
	Connect,			// Request to connect to server (C->S)
	Disconnect,			// Request to disconnect from server (C->S)
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


struct MessageHeader
{
	ClientID clientID;
	MessageCode messageCode;
	float time;
	sf::Uint32 sequence;
};
sf::Packet& operator <<(sf::Packet& packet, const MessageHeader& header);
sf::Packet& operator >>(sf::Packet& packet, MessageHeader& header);


struct ConnectMessage
{
	sf::Uint8 numPlayers;
	ClientID playerIDs[MAX_NUM_PLAYERS];
};
sf::Packet& operator <<(sf::Packet& packet, const ConnectMessage& message);
sf::Packet& operator >>(sf::Packet& packet, ConnectMessage& message);


struct PlayerConnectedMessage
{
	ClientID playerID;
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
	float x;
	float y;
	float rotation;
};
sf::Packet& operator <<(sf::Packet& packet, const UpdateMessage& message);
sf::Packet& operator >>(sf::Packet& packet, UpdateMessage& message);



struct Client
{
	// network properties
	ClientID id = INVALID_CLIENT_ID;
	sf::IpAddress ip;
	unsigned short port = -1;

	// in-game player properties
	float x			= 0.0f;
	float y			= 0.0f;
	float rotation	= 0.0f;
};
