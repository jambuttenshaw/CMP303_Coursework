#include "ServerApplication.h"

#include "NetworkTypes.h"

#include <iostream>


ServerApplication::ServerApplication()
{
	// set up server socket
	m_ServerSocket.bind(ServerPort);
	m_ServerSocket.setBlocking(false);
}

ServerApplication::~ServerApplication()
{
}

void ServerApplication::Run()
{

	while (true)
	{
		// try to recieve data
		sf::Packet incomingData;
		sf::IpAddress incomingAddress;
		unsigned short incomingPort;

		sf::Socket::Status status = m_ServerSocket.receive(incomingData, incomingAddress, incomingPort);
		if (status == sf::Socket::Done)
		{
			// data was recieved

		}
	}

}
