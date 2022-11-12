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
	return packet << header.clientID << header.messageCode << header.time << header.sequence;
}

sf::Packet& operator>>(sf::Packet& packet, MessageHeader& header)
{
	return packet >> header.clientID >> header.messageCode >> header.time >> header.sequence;
}


/*
MESSAGE BODIES
*/

sf::Packet& operator<<(sf::Packet& packet, const ConnectMessage& message)
{
	packet << message.numPlayers;
	for (unsigned int i = 0; i < message.numPlayers; i++)
	{
		packet << message.playerIDs[i];
	}
	return packet;
}

sf::Packet& operator>>(sf::Packet& packet, ConnectMessage& message)
{
	packet >> message.numPlayers;
	for (unsigned int i = 0; i < message.numPlayers; i++)
	{
		packet >> message.playerIDs[i];
	}
	return packet;
}


sf::Packet& operator<<(sf::Packet& packet, const PlayerConnectedMessage& message)
{
	return packet << message.playerID;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerConnectedMessage& message)
{
	return packet >> message.playerID;
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
	return packet << message.x << message.y << message.rotation;
}

sf::Packet& operator>>(sf::Packet& packet, UpdateMessage& message)
{
	return packet >> message.x >> message.y >> message.rotation;
}
