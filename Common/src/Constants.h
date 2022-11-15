#pragma once

#include <SFML/System.hpp>


const char* const SERVER_ADDRESS = "127.0.0.1";
const unsigned short SERVER_PORT = 4444;

const sf::Uint8 MAX_NUM_PLAYERS = 16;
const sf::Uint8 MAX_NUM_BLOCKS = 255;
const sf::Uint8 MAX_NUM_PROJECTILES = 255;

// update ticks every 100ms
const float UPDATE_FREQUENCY = 0.1f;
const float LATENCY_PING_FREQUENCY = 1.0f;

const float WORLD_MIN_X = 0.0f;
const float WORLD_MAX_X = 800.0f;

const float WORLD_MIN_Y = 0.0f;
const float WORLD_MAX_Y = 600.0f;

const float PLAYER_SIZE = 15.0f;
const float PLAYER_MOVE_SPEED = 150.0f;

const float PROJECTILE_RADIUS = 3.0f;
const float PROJECTILE_MOVE_SPEED = 750.0f;

const float BLOCK_SIZE = 20.0f;
