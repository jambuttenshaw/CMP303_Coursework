#include "ControllablePlayer.h"

#include "MathUtils.h"


ControllablePlayer::ControllablePlayer(const sf::Window& window)
	: m_Window(window)
{
}

void ControllablePlayer::Update(float dt)
{
	auto pos = getPosition();

	sf::Vector2f velocity{ 0, 0 };
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		velocity.x += 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		velocity.x -= 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		velocity.y += 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		velocity.y -= 1;

	Normalize(velocity);
	velocity *= m_MoveSpeed;

	pos += velocity * dt;

	setPosition(pos);

	// calculate angle to mouse
	
	auto toMouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_Window)) - pos;
	float angle = RadToDeg(atan2f(toMouse.y, toMouse.x));

	setRotation(angle);
}
