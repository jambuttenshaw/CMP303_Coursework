#pragma once

#include <SFML/Network.hpp>

/*
Common types shared between the client and server projects
*/


enum class PlayerTeam : sf::Uint8
{
	None,
	Red,
	Blue
};
sf::Packet& operator <<(sf::Packet& packet, const PlayerTeam& team);
sf::Packet& operator >>(sf::Packet& packet, PlayerTeam& team);


enum class GameState : sf::Uint8
{
	FightMode,
	BuildMode
};
sf::Packet& operator <<(sf::Packet& packet, const GameState& state);
sf::Packet& operator >>(sf::Packet& packet, GameState& state);


using ProjectileID = sf::Uint32;
const ProjectileID INVALID_PROJECTILE_ID = (ProjectileID)(-1);

using BlockID = sf::Uint32;
const BlockID INVALID_BLOCK_ID = (BlockID)(-1);
