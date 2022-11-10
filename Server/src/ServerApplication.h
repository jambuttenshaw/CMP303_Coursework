#pragma once

#include <SFML/Network.hpp>


class ServerApplication
{
public:
	ServerApplication();
	~ServerApplication();

	void Run();

private:
	sf::UdpSocket m_ServerSocket;

};
