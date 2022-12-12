#include "ControllablePlayer.h"

#include "Projectile.h"
#include "Block.h"

#include "MathUtils.h"
#include "Constants.h"

#include "imgui.h"


bool ControllablePlayer::s_EnableAutomove = false;

ControllablePlayer::ControllablePlayer(sf::RenderWindow& window)
	: m_Window(window)
{
}

ControllablePlayer::~ControllablePlayer()
{
}

sf::Vector2f ControllablePlayer::CalculateMovement(float dt)
{
	// check if player should automove
	if (s_EnableAutomove) return Automove(dt);

	// move based off of keyboard input
	sf::Vector2f velocity{ 0, 0 };
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		velocity.x += 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		velocity.x -= 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		velocity.y += 1;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		velocity.y -= 1;

	// scale movement vector to be of length PLAYER_MOVE_SPEED
	Normalize(velocity);
	velocity *= PLAYER_MOVE_SPEED;

	return velocity * dt;
}

void ControllablePlayer::UpdateRotation()
{
	// calculate angle to mouse
	auto toMouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_Window)) - getPosition();
	float angle = RadToDeg(atan2f(toMouse.y, toMouse.x));

	setRotation(angle);
}

void ControllablePlayer::SettingsGUI()
{
	ImGui::Checkbox("Automove", &s_EnableAutomove);
}

sf::Vector2f ControllablePlayer::Automove(float dt)
{
	// quick function to move the player in a semi random direction and to switch direction every so often
	static float t = 0;
	static float directionX = 0.0f;
	static float directionY = 1.0f;
	static float timerX = 2.0f;
	static float timerY = 10.0f;

	sf::Vector2f velocity{ dt * directionX * PLAYER_MOVE_SPEED, dt * directionY * PLAYER_MOVE_SPEED };

	t += dt;
	if (t > timerX)
	{
		timerX += 2.31f;
		directionX = -directionX;
	}
	if (t > timerY)
	{
		timerY += 3.58f;
		directionY = -directionY;
	}

	return velocity;
}
