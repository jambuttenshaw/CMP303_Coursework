#include "Constants.h"


// server properties
const unsigned short SERVER_PORT = 4444;

const float IDLE_TIMEOUT = 10.0f; // 10 second idle timout
const float UPDATE_FREQUENCY = 1.0f / 20.0f; // update ticks 20 times a second
const float MAX_MOVE_DISTANCE = 100.0f;
const float STATE_HISTORY_DURATION = 3.0f; // 3 seconds

// world bounds
const float WORLD_WIDTH = 900.0f;
const float WORLD_HEIGHT = 600.0f;

const float SPAWN_WIDTH = 170.0f;

// player properties
const float PLAYER_SIZE = 15.0f;
const float PLAYER_MOVE_SPEED = 150.0f;

// projectile properties
const float PROJECTILE_RADIUS = 3.0f;
const float PROJECTILE_MOVE_SPEED = 1500.0f;

// block properties
const float BLOCK_SIZE = 20.0f;
const float BLOCK_PLACE_RADIUS = 50.0f;

// game state changes
const float INITIAL_BUILD_MODE_DURATION = 3.0f;
const float INITIAL_FIGHT_MODE_DURATION = 800.0f;
const float MIN_BUILD_MODE_DURATION = 10.0f;
const float MIN_FIGHT_MODE_DURATION = 30.0f;
