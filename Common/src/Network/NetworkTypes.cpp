#include "NetworkTypes.h"



sf::Packet MessageHeader::Create()
{
	sf::Packet packet;
	packet << clientID << messageCode << time << sequence;
	return packet;
}

void MessageHeader::Extract(sf::Packet& packet)
{
	packet >> clientID >> messageCode >> time >> sequence;
}




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
