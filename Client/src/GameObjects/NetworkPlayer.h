#pragma once

#include "Player.h"
#include "Network/NetworkTypes.h"


class NetworkPlayer : public Player
{
public:
	NetworkPlayer(ClientID clientID);
	virtual ~NetworkPlayer();

	inline ClientID GetID() const { return m_ClientID; }

	void Update(float dt);
	void NetworkUpdate(const UpdateMessage& data);

private:
	const ClientID m_ClientID;
};