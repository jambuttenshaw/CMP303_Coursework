#include "CommonTypes.h"


sf::Packet& operator <<(sf::Packet& packet, const PlayerTeam& team)
{
	return packet << static_cast<sf::Uint8>(team);
}

sf::Packet& operator >>(sf::Packet& packet, PlayerTeam& team)
{
	sf::Uint8 team_int;
	packet >> team_int;

	team = static_cast<PlayerTeam>(team_int);
	return packet;
}

sf::Packet& operator <<(sf::Packet& packet, const GameState& state)
{
	return packet << static_cast<sf::Uint8>(state);
}

sf::Packet& operator >>(sf::Packet& packet, GameState& state)
{
	sf::Uint8 state_int;
	packet >> state_int;

	state = static_cast<GameState>(state_int);
	return packet;
}

const char* GameStateToStr(GameState s)
{
	switch (s)
	{
	case GameState::Invalid:
		return "Invalid";
		break;
	case GameState::FightMode:
		return "Fight Mode";
		break;
	case GameState::BuildMode:
		return "Build Mode";
		break;
	default:
		return "Unknown";
		break;
	}
}
