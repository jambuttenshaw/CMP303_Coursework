#include "NetworkTypes.h"

#include "MathUtils.h"
#include "Log.h"

#define CHECK_PACKET_ERROR(v) CHECK_ERROR(v, "Packet operation failed!")


sf::Packet& operator <<(sf::Packet& packet, const MessageCode& mc)
{
	CHECK_PACKET_ERROR(packet << static_cast<sf::Uint8>(mc));
	return packet;
}

sf::Packet& operator >>(sf::Packet& packet, MessageCode& mc)
{
	sf::Uint8 mc_int;
	CHECK_PACKET_ERROR(packet >> mc_int);

	mc = static_cast<MessageCode>(mc_int);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const MessageHeader& header)
{
	CHECK_PACKET_ERROR(packet << header.clientID << header.messageCode);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, MessageHeader& header)
{
	CHECK_PACKET_ERROR(packet >> header.clientID >> header.messageCode);
	return packet;
}


/*
MESSAGE BODIES
*/


sf::Packet& operator<<(sf::Packet& packet, const IntroductionMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.udpPort);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, IntroductionMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.udpPort);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const ConnectMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.playerNumber << message.team << message.numPlayers);

	for (unsigned int i = 0; i < message.numPlayers; i++)
		CHECK_PACKET_ERROR(packet << message.playerIDs[i] << message.playerTeams[i]);

	CHECK_PACKET_ERROR(packet << message.numBlocks);

	for (unsigned int i = 0; i < message.numBlocks; i++)
		CHECK_PACKET_ERROR(packet << message.blockIDs[i] << message.blockTeams[i] << message.blockXs[i] << message.blockYs[i]);

	CHECK_PACKET_ERROR(packet << message.gameState << message.remainingStateDuration << message.turfLine);

	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ConnectMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.playerNumber >> message.team >>  message.numPlayers);

	for (unsigned int i = 0; i < message.numPlayers; i++)
		CHECK_PACKET_ERROR(packet >> message.playerIDs[i] >> message.playerTeams[i]);

	CHECK_PACKET_ERROR(packet >> message.numBlocks);

	for (unsigned int i = 0; i < message.numBlocks; i++)
		CHECK_PACKET_ERROR(packet >> message.blockIDs[i] >> message.blockTeams[i] >> message.blockXs[i] >> message.blockYs[i]);

	CHECK_PACKET_ERROR(packet >> message.gameState >> message.remainingStateDuration >> message.turfLine);

	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const PlayerConnectedMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.playerID << message.team);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerConnectedMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.playerID >> message.team);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const PlayerDisconnectedMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.playerID);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerDisconnectedMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.playerID);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const UpdateMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.playerID << message.x << message.y << message.rotation << message.dt);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, UpdateMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.playerID >> message.x >> message.y >> message.rotation >> message.dt);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const ChangeTeamMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.playerID << message.team);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ChangeTeamMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.playerID >> message.team);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const ServerTimeMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.serverTime);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ServerTimeMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.serverTime);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const ShootMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.id << message.shotBy << message.team << message.x << message.y << message.dirX << message.dirY << message.shootTime);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ShootMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.id >> message.shotBy >> message.team >> message.x >> message.y >> message.dirX >> message.dirY >> message.shootTime);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const ProjectilesDestroyedMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.count);
	for (auto i = 0; i < message.count; i++) CHECK_PACKET_ERROR(packet << message.ids[i]);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ProjectilesDestroyedMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.count);
	for (auto i = 0; i < message.count; i++) CHECK_PACKET_ERROR(packet >> message.ids[i]);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const PlaceMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.id << message.placedBy << message.team << message.x << message.y);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, PlaceMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.id >> message.placedBy >> message.team >> message.x >> message.y);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const BlocksDestroyedMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.count);
	for (auto i = 0; i < message.count; i++) CHECK_PACKET_ERROR(packet << message.ids[i]);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, BlocksDestroyedMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.count);
	for (auto i = 0; i < message.count; i++) CHECK_PACKET_ERROR(packet >> message.ids[i]);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const ChangeGameStateMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.state << message.stateDuration);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ChangeGameStateMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.state >> message.stateDuration);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const TurfLineMoveMessage& message)
{
	CHECK_PACKET_ERROR(packet << message.newTurfLine);
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, TurfLineMoveMessage& message)
{
	CHECK_PACKET_ERROR(packet >> message.newTurfLine);
	return packet;
}


bool BlockProjectileCollision(BlockState* block, ProjectileState* projectile)
{
	const float r1 = 0.5f * BLOCK_SIZE; // block inner circle radius
	const float r2 = sqrtf(0.5f * BLOCK_SIZE * BLOCK_SIZE); // block outer circle radius
	float sqrDistanceBetweenCentres = SqrLength(block->position - projectile->position);

	if (sqrDistanceBetweenCentres > (r2 + PROJECTILE_RADIUS) * (r2 + PROJECTILE_RADIUS)) return false;
	if (sqrDistanceBetweenCentres < (r1 + PROJECTILE_RADIUS) * (r1 + PROJECTILE_RADIUS)) return true;

	sf::Vector2f c1ToC2 = Normalized(block->position - projectile->position);
	sf::Vector2f p = projectile->position + PROJECTILE_RADIUS * c1ToC2;

	if (p.x < block->position.x - 0.5f * BLOCK_SIZE || p.x > block->position.x + 0.5f * BLOCK_SIZE) return false;
	if (p.y < block->position.y - 0.5f * BLOCK_SIZE || p.y > block->position.y + 0.5f * BLOCK_SIZE) return false;

	return true;
}


bool PlayerProjectileCollision(const sf::Vector2f& playerPos, const sf::Vector2f& projectilePos)
{
	float sqrDistanceBetweenCentres = SqrLength(playerPos - projectilePos);
	float playerRadius = 0.5f * PLAYER_SIZE;
	return sqrDistanceBetweenCentres < (PROJECTILE_RADIUS + playerRadius)* (PROJECTILE_RADIUS + playerRadius);
}
