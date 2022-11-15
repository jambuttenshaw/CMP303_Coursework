#include "NetworkTypes.h"

#include "MathUtils.h"


sf::Packet& operator <<(sf::Packet& packet, const MessageCode& mc)
{
	return packet << static_cast<sf::Uint8>(mc);
}

sf::Packet& operator >>(sf::Packet& packet, MessageCode& mc)
{
	sf::Uint8 mc_int;
	packet >> mc_int;

	mc = static_cast<MessageCode>(mc_int);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const MessageHeader& header)
{
	return packet << header.clientID << header.messageCode;
}

sf::Packet& operator>>(sf::Packet& packet, MessageHeader& header)
{
	return packet >> header.clientID >> header.messageCode;
}


/*
MESSAGE BODIES
*/


sf::Packet& operator<<(sf::Packet& packet, const IntroductionMessage& message)
{
	return packet << message.udpPort;
}

sf::Packet& operator>>(sf::Packet& packet, IntroductionMessage& message)
{
	return packet >> message.udpPort;
}


sf::Packet& operator<<(sf::Packet& packet, const ConnectMessage& message)
{
	packet << message.team << message.numPlayers;
	for (unsigned int i = 0; i < message.numPlayers; i++)
		packet << message.playerIDs[i] << message.playerTeams[i];
	packet << message.numBlocks;
	for (unsigned int i = 0; i < message.numBlocks; i++)
		packet << message.blockIDs[i] << message.blockTeams[i] << message.blockXs[i] << message.blockYs[i];
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ConnectMessage& message)
{
	packet >> message.team >>  message.numPlayers;
	for (unsigned int i = 0; i < message.numPlayers; i++)
		packet >> message.playerIDs[i] >> message.playerTeams[i];
	packet >> message.numBlocks;
	for (unsigned int i = 0; i < message.numBlocks; i++)
		packet >> message.blockIDs[i] >> message.blockTeams[i] >> message.blockXs[i] >> message.blockYs[i];
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const PlayerConnectedMessage& message)
{
	return packet << message.playerID << message.team;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerConnectedMessage& message)
{
	return packet >> message.playerID >> message.team;
}


sf::Packet& operator<<(sf::Packet& packet, const PlayerDisconnectedMessage& message)
{
	return packet << message.playerID;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerDisconnectedMessage& message)
{
	return packet >> message.playerID;
}


sf::Packet& operator<<(sf::Packet& packet, const UpdateMessage& message)
{
	return packet << message.playerID << message.x << message.y << message.rotation;
}

sf::Packet& operator>>(sf::Packet& packet, UpdateMessage& message)
{
	return packet >> message.playerID >> message.x >> message.y >> message.rotation;
}


sf::Packet& operator<<(sf::Packet& packet, const ChangeTeamMessage& message)
{
	return packet << message.playerID << message.team;
}

sf::Packet& operator>>(sf::Packet& packet, ChangeTeamMessage& message)
{
	return packet >> message.playerID >> message.team;
}


sf::Packet& operator<<(sf::Packet& packet, const ServerTimeMessage& message)
{
	return packet << message.serverTime;
}

sf::Packet& operator>>(sf::Packet& packet, ServerTimeMessage& message)
{
	return packet >> message.serverTime;
}


sf::Packet& operator<<(sf::Packet& packet, const ShootMessage& message)
{
	return packet << message.id << message.team << message.x << message.y << message.dirX << message.dirY << message.shootTime;
}

sf::Packet& operator>>(sf::Packet& packet, ShootMessage& message)
{
	return packet >> message.id >> message.team >> message.x >> message.y >> message.dirX >> message.dirY >> message.shootTime;
}


sf::Packet& operator<<(sf::Packet& packet, const ProjectilesDestroyedMessage& message)
{
	packet << message.count;
	for (auto i = 0; i < message.count; i++) packet << message.ids[i];
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ProjectilesDestroyedMessage& message)
{
	packet >> message.count;
	for (auto i = 0; i < message.count; i++) packet >> message.ids[i];
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const PlaceMessage& message)
{
	return packet << message.id << message.team << message.x << message.y;
}

sf::Packet& operator>>(sf::Packet& packet, PlaceMessage& message)
{
	return packet >> message.id >> message.team >> message.x >> message.y;
}


sf::Packet& operator<<(sf::Packet& packet, const BlocksDestroyedMessage& message)
{
	packet << message.count;
	for (auto i = 0; i < message.count; i++) packet << message.ids[i];
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, BlocksDestroyedMessage& message)
{
	packet >> message.count;
	for (auto i = 0; i < message.count; i++) packet >> message.ids[i];
	return packet;
}


bool BlockProjectileCollision(BlockState* block, ProjectileState* projectile)
{
	const float r1 = 0.5f * BLOCK_SIZE;
	const float r2 = sqrtf(0.5f * BLOCK_SIZE * BLOCK_SIZE);
	float sqrDistanceBetweenCentres = SqrLength(block->position - projectile->position);
	if (sqrDistanceBetweenCentres > (r1 + PROJECTILE_RADIUS) * (r1 + PROJECTILE_RADIUS)) return false;
	if (sqrDistanceBetweenCentres < (r2 + PROJECTILE_RADIUS) * (r2 + PROJECTILE_RADIUS)) return true;

	sf::Vector2f c1ToC2 = Normalized(block->position - projectile->position);
	sf::Vector2f p = projectile->position + PROJECTILE_RADIUS * c1ToC2;

	if (p.x < block->position.x - 0.5f * BLOCK_SIZE || p.x > block->position.x + 0.5f * BLOCK_SIZE) return false;
	if (p.y < block->position.y - 0.5f * BLOCK_SIZE || p.y > block->position.y + 0.5f * BLOCK_SIZE) return false;

	return true;
}
