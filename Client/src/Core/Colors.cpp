#include "Colors.h"


const sf::Color& ColourFromTeam(const PlayerTeam team)
{
	switch (team)
	{
	case PlayerTeam::None:	return NoTeamColor;
	case PlayerTeam::Red:	return RedTeamColor;
	case PlayerTeam::Blue:	return BlueTeamColor;
	}

	return NoTeamColor;
}

const sf::Color& DarkColourFromTeam(const PlayerTeam team)
{
	switch (team)
	{
	case PlayerTeam::None:	return DarkNoTeamColor;
	case PlayerTeam::Red:	return DarkRedTeamColor;
	case PlayerTeam::Blue:	return DarkBlueTeamColor;
	}

	return NoTeamColor;
}

const sf::Color& LightColourFromTeam(const PlayerTeam team)
{
	switch (team)
	{
	case PlayerTeam::None:	return LightNoTeamColor;
	case PlayerTeam::Red:	return LightRedTeamColor;
	case PlayerTeam::Blue:	return LightBlueTeamColor;
	}

	return NoTeamColor;
}

sf::Color GetGhostBlockColour(const PlayerTeam team, bool disabled)
{
	auto c = DarkColourFromTeam(disabled ? PlayerTeam::None : team);
	return sf::Color(c.r, c.g, c.b, GhostBlockAlpha);
}
