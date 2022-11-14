#include "NetworkTypes.h"


sf::Packet& operator <<(sf::Packet& packet, const MessageCode& mc)
{
	return packet << static_cast<sf::Uint8>(mc);
}

sf::Packet& operator >>(sf::Packet& packet, MessageCode& mc)
{
	sf::Uint8 mc_int;
	packet >> mc_int;

	mc = static_cast<MessageCode>(mc_int);
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const MessageHeader& header)
{
	return packet << header.clientID << header.messageCode << header.time;
}

sf::Packet& operator>>(sf::Packet& packet, MessageHeader& header)
{
	return packet >> header.clientID >> header.messageCode >> header.time;
}


/*
MESSAGE BODIES
*/


sf::Packet& operator<<(sf::Packet& packet, const IntroductionMessage& message)
{
	return packet << message.udpPort;
}

sf::Packet& operator>>(sf::Packet& packet, IntroductionMessage& message)
{
	return packet >> message.udpPort;
}


sf::Packet& operator<<(sf::Packet& packet, const ConnectMessage& message)
{
	packet << message.team << message.numPlayers;
	for (unsigned int i = 0; i < message.numPlayers; i++)
	{
		packet << message.playerIDs[i] << message.playerTeams[i];
	}
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ConnectMessage& message)
{
	packet >> message.team >>  message.numPlayers;
	for (unsigned int i = 0; i < message.numPlayers; i++)
	{
		packet >> message.playerIDs[i] >> message.playerTeams[i];
	}
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const PlayerConnectedMessage& message)
{
	return packet << message.playerID << message.team;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerConnectedMessage& message)
{
	return packet >> message.playerID >> message.team;
}


sf::Packet& operator<<(sf::Packet& packet, const PlayerDisconnectedMessage& message)
{
	return packet << message.playerID;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerDisconnectedMessage& message)
{
	return packet >> message.playerID;
}


sf::Packet& operator<<(sf::Packet& packet, const UpdateMessage& message)
{
	return packet << message.playerID << message.x << message.y << message.rotation;
}

sf::Packet& operator>>(sf::Packet& packet, UpdateMessage& message)
{
	return packet >> message.playerID >> message.x >> message.y >> message.rotation;
}


sf::Packet& operator<<(sf::Packet& packet, const ChangeTeamMessage& message)
{
	return packet << message.playerID << message.team;
}

sf::Packet& operator>>(sf::Packet& packet, ChangeTeamMessage& message)
{
	return packet >> message.playerID >> message.team;
}



void PlayerState::Update(const UpdateMessage& m)
{
	x = m.x;
	y = m.y;
	rotation = m.rotation;
}
