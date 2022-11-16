#include "Constants.h"


// server properties
const char* const SERVER_ADDRESS = "127.0.0.1";
const unsigned short SERVER_PORT = 4444;

// update ticks every 100ms
const float UPDATE_FREQUENCY = 0.1f;

// world bounds
const float WORLD_MIN_X = 0.0f;
const float WORLD_MAX_X = 1200.0f;
const float WORLD_MIN_Y = 0.0f;
const float WORLD_MAX_Y = 675.0f;

const float SPAWN_WIDTH = 160.0f;

// player properties
const float PLAYER_SIZE = 15.0f;
const float PLAYER_MOVE_SPEED = 150.0f;

// projectile properties
const float PROJECTILE_RADIUS = 3.0f;
const float PROJECTILE_MOVE_SPEED = 750.0f;

// block properties
const float BLOCK_SIZE = 20.0f;
const float BLOCK_PLACE_RADIUS = 40.0f;

// game state changes
const GameState INITIAL_GAME_STATE = GameState::BuildMode;

const float INITIAL_BUILD_MODE_DURATION = 60.0f;
const float INITIAL_FIGHT_MODE_DURATION = 120.0f;
const float MIN_BUILD_MODE_DURATION = 10.0f;
const float MIN_FIGHT_MODE_DURATION = 30.0f;
