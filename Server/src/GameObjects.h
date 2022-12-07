#pragma once

#include "Network/NetworkTypes.h"
#include <SFML/System.hpp>

// Descriptions of the game objects for the server
// there is purely data, no concept of graphics



struct BlockState
{
	BlockID id;
	PlayerTeam team;

	sf::Vector2f position;

	BlockState(PlaceMessage placeMessage);
	BlockState(BlockID _id, PlayerTeam _team, const sf::Vector2f& _position);
};

struct ProjectileState
{
	ProjectileID id;
	ClientID shotBy;
	PlayerTeam team;

	sf::Vector2f position;

	sf::Vector2f initPosition;
	sf::Vector2f direction;

	float serverShootTime; // the sim time when the projectile was shot (local to the client that shot it)
	float clientShootTime; // when the server recieved the request to shoot a projectile, and when the projectile was actually created

	ProjectileState(ShootMessage shootMessage);

	void SimulationStep(float dt);

	sf::Vector2f PositionAtServerTime(float t);
	sf::Vector2f PositionAtClientTime(float t);

	bool BlockCollision(BlockState* block);
	bool PlayerCollision(const sf::Vector2f& playerPos);
	// check for player timeDelta seconds in the past
	bool PlayerCollision(const sf::Vector2f& playerPos, float timeDelta);
};
