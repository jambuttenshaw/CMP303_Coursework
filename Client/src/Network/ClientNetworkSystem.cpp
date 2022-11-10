#include "ClientNetworkSystem.h"

#include "NetworkTypes.h"


ClientNetworkSystem::ClientNetworkSystem()
{
	m_ClientSocket.setBlocking(false);
}

ClientNetworkSystem::~ClientNetworkSystem()
{
}

void ClientNetworkSystem::Update(float dt)
{
	m_Timer += dt;
	if (m_Timer > NetworkTickSpeed)
	{
		m_Timer = 0.0f;

	}
}
