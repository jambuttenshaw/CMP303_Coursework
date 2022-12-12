#include "GameObjects.h"

#include "MathUtils.h"


BlockState::BlockState(PlaceMessage placeMessage)
{
	id = placeMessage.id;
	team = placeMessage.team;
	position = { placeMessage.x, placeMessage.y };
}
BlockState::BlockState(BlockID _id, PlayerTeam _team, const sf::Vector2f& _position)
{
	id = _id;
	team = _team;
	position = _position;
}


ProjectileState::ProjectileState(ShootMessage shootMessage)
{
	id = shootMessage.id;
	shotBy = shootMessage.shotBy;
	team = shootMessage.team;
	position = { shootMessage.x, shootMessage.y };
	initPosition = position;
	direction = { shootMessage.dirX, shootMessage.dirY };
	serverShootTime = 0.0f;
	clientShootTime = 0.0f;
}

void ProjectileState::SimulationStep(float dt)
{
	position += direction * PROJECTILE_MOVE_SPEED * dt;
}

sf::Vector2f ProjectileState::PositionAtServerTime(float t)
{
	float dt = t - serverShootTime;
	return initPosition + direction * (PROJECTILE_MOVE_SPEED * dt);
}

sf::Vector2f ProjectileState::PositionAtClientTime(float t)
{
	float dt = t - clientShootTime;
	return initPosition + direction * (PROJECTILE_MOVE_SPEED * dt);
}


bool ProjectileState::BlockCollision(BlockState* block)
{
	const float r1 = 0.5f * BLOCK_SIZE; // block inner circle radius
	const float r2 = sqrtf(0.5f * BLOCK_SIZE * BLOCK_SIZE); // block outer circle radius
	float sqrDistanceBetweenCentres = SqrLength(block->position - position);

	if (sqrDistanceBetweenCentres > (r2 + PROJECTILE_RADIUS) * (r2 + PROJECTILE_RADIUS)) return false;
	if (sqrDistanceBetweenCentres < (r1 + PROJECTILE_RADIUS) * (r1 + PROJECTILE_RADIUS)) return true;

	sf::Vector2f c1ToC2 = Normalized(block->position - position);
	sf::Vector2f p = position + PROJECTILE_RADIUS * c1ToC2;

	if (p.x < block->position.x - 0.5f * BLOCK_SIZE || p.x > block->position.x + 0.5f * BLOCK_SIZE) return false;
	if (p.y < block->position.y - 0.5f * BLOCK_SIZE || p.y > block->position.y + 0.5f * BLOCK_SIZE) return false;

	return true;
}


bool ProjectileState::PlayerCollision(const sf::Vector2f& playerPos)
{
	float sqrDistanceBetweenCentres = SqrLength(playerPos - position);
	float playerRadius = 0.5f * PLAYER_SIZE;
	return sqrDistanceBetweenCentres < (PROJECTILE_RADIUS + playerRadius)* (PROJECTILE_RADIUS + playerRadius);
}

bool ProjectileState::PlayerCollision(const sf::Vector2f& playerPos, float timeDelta)
{
	// work out position timeDelta seconds in the past
	sf::Vector2f p = position - (timeDelta * PROJECTILE_MOVE_SPEED * direction);
	float sqrDistanceBetweenCentres = SqrLength(playerPos - p);
	float playerRadius = 0.5f * PLAYER_SIZE;
	return sqrDistanceBetweenCentres < (PROJECTILE_RADIUS + playerRadius)* (PROJECTILE_RADIUS + playerRadius);
}
