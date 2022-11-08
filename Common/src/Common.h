#pragma once


#pragma warning(disable: 4099)


#include <SFML/Graphics.hpp>

class Player
{
public:
	Player() = default;

	const sf::RectangleShape& GetShape() const;

private:
	sf::RectangleShape shape;
};
