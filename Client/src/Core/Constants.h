#pragma once

#include <SFML/Graphics.hpp>

#include "CommonTypes.h"


const sf::Color NoTeamColor{ 237, 237, 237 };

const sf::Color RedTeamColor{ 217, 85, 76 };
const sf::Color LightRedColor{ 234, 163, 158 };
const sf::Color DarkRedColor{ 156, 41, 33 };

const sf::Color BlueTeamColor{ 76, 139, 217 };
const sf::Color LightBlueColor{ 158, 193, 234 };
const sf::Color DarkBlueColor{ 33, 88, 156 };

const sf::Color& ColourFromTeam(const PlayerTeam team);
