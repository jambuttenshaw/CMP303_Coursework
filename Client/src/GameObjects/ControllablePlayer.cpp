#include "ControllablePlayer.h"

#include "Projectile.h"
#include "Block.h"

#include "MathUtils.h"


ControllablePlayer::ControllablePlayer(sf::RenderWindow& window)
	: m_Window(window)
{
}

ControllablePlayer::~ControllablePlayer()
{
}

sf::Vector2f ControllablePlayer::CalculateMovement(float dt)
{
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

	return velocity * dt;
}

void ControllablePlayer::UpdateRotation()
{
	// calculate angle to mouse
	auto toMouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_Window)) - getPosition();
	float angle = RadToDeg(atan2f(toMouse.y, toMouse.x));

	setRotation(angle);
}
