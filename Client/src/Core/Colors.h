#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"


const sf::Color NoTeamColor{ 186, 186, 186 };
const sf::Color DarkNoTeamColor{ 135, 135, 135 };
const sf::Color LightNoTeamColor{ 237, 237, 237 };

const sf::Color RedTeamColor{ 217, 85, 76 };
const sf::Color LightRedTeamColor{ 234, 163, 158 };
const sf::Color DarkRedTeamColor{ 156, 41, 33 };

const sf::Color BlueTeamColor{ 76, 139, 217 };
const sf::Color LightBlueTeamColor{ 158, 193, 234 };
const sf::Color DarkBlueTeamColor{ 33, 88, 156 };

const sf::Uint8 GhostBlockAlpha = 190;

const sf::Color& ColourFromTeam(const PlayerTeam team);
const sf::Color& DarkColourFromTeam(const PlayerTeam team);
const sf::Color& LightColourFromTeam(const PlayerTeam team);

const sf::Color& GetGhostBlockColour(const PlayerTeam team, bool disabled);