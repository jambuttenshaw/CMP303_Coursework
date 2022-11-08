#include "Constants.h"


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