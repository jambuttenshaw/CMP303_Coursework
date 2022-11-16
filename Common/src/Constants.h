#pragma once

#include <SFML/System.hpp>
#include "CommonTypes.h"


// server properties
extern const char* const SERVER_ADDRESS;
extern const unsigned short SERVER_PORT;

// max game objects (these are used as stack-allocated array dimensions, so cannot be marked as extern and compiled in a different compilation unit)
const sf::Uint8 MAX_NUM_PLAYERS = 16;
const sf::Uint8 MAX_NUM_BLOCKS = 255;
const sf::Uint8 MAX_NUM_PROJECTILES = 255;

// update ticks every 100ms
extern const float UPDATE_FREQUENCY;

// world bounds
extern const float WORLD_MIN_X;
extern const float WORLD_MAX_X;
extern const float WORLD_MIN_Y;
extern const float WORLD_MAX_Y;

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
extern const GameState INITIAL_GAME_STATE;

extern const float INITIAL_BUILD_MODE_DURATION;
extern const float INITIAL_FIGHT_MODE_DURATION;
extern const float MIN_BUILD_MODE_DURATION;
extern const float MIN_FIGHT_MODE_DURATION;
