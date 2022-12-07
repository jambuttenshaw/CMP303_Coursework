#pragma once

#include <SFML/System.hpp>
#include "CommonTypes.h"


// values for these constants can be found in ConstantDefinitions.cpp
// this is so that the values of constants can be tweaked without having to recompile more than 1 translation unit

// server properties
extern const unsigned short SERVER_PORT;

// how long a client has to not send a message for to be considered disconnected
extern const float IDLE_TIMEOUT;
// how often messages are sent client->server and server->client
extern const float UPDATE_FREQUENCY;
// how often the server measures client latencey
extern const float PING_FREQUENCY;
// if a player travels a distance greater than this in a single UPDATE_FREQUENCY,
// then they are considered to have been forcibly teleported
// so do not interpolate them
extern const float MAX_MOVE_DISTANCE;
// the length of time (in seconds) of state history to retain for players, both on server and clients
extern const float STATE_HISTORY_DURATION;

// max game objects (these are used as stack-allocated array dimensions, so cannot be marked as extern and compiled in a different compilation unit)
const sf::Uint8 MAX_NUM_PLAYERS = 16;
const sf::Uint8 MAX_NUM_BLOCKS = 255;
const sf::Uint8 MAX_NUM_PROJECTILES = 255;

// world bounds
extern const float WORLD_WIDTH;
extern const float WORLD_HEIGHT;

extern const float SPAWN_WIDTH;

// player properties
extern const float PLAYER_SIZE;
extern const float PLAYER_MOVE_SPEED;

// projectile properties
extern const float PROJECTILE_RADIUS;
extern const float PROJECTILE_MOVE_SPEED;

// block properties
extern const float BLOCK_SIZE;
extern const float BLOCK_PLACE_RADIUS;

// game state changes
extern const float INITIAL_BUILD_MODE_DURATION;
extern const float INITIAL_FIGHT_MODE_DURATION;
extern const float MIN_BUILD_MODE_DURATION;
extern const float MIN_FIGHT_MODE_DURATION;

// how many blocks the player gets in the first build mode
extern const unsigned int INITIAL_BUILD_MODE_BLOCKS;
// how many blocks the player gets in every other build mode
extern const unsigned int SUBSEQUENT_BUILD_MODE_BLOCKS;

// how many projectiles a player can hold at one time
extern const unsigned int MAX_AMMO_HELD;
extern const float RELOAD_TIME;
