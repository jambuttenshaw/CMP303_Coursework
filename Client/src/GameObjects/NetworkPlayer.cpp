#include "NetworkPlayer.h"

NetworkPlayer::NetworkPlayer(ClientID clientID)
	: m_ClientID(clientID)
{
}

NetworkPlayer::~NetworkPlayer()
{
}

void NetworkPlayer::Update(float dt)
{
}

void NetworkPlayer::NetworkUpdate(const UpdateMessage& data)
{
	setPosition(data.x, data.y);
	setRotation(data.rotation);
}
