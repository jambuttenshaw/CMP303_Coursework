#pragma once

#include "Network\NetworkTypes.h"

#include <deque>
#include <cassert>


class Connection
{
public:
	Connection();
	~Connection();

	inline sf::TcpSocket& GetSocket() { return m_Socket; }
	inline ClientID GetID() const { return m_ID; }
	
	inline sf::Uint8 GetPlayerNumber() const { return m_PlayerNumber; }
	inline PlayerTeam GetPlayerTeam() const { return m_PlayerTeam; }
	inline void SetPlayerTeam(PlayerTeam team) { m_PlayerTeam = team; }

	inline const sf::IpAddress& GetIP() const { return m_ClientIP; }
	inline unsigned short GetTcpPort() const { return m_TcpPort; }
	inline unsigned short GetUdpPort() const { return m_UdpPort; }

	inline bool CanSendUdp() const { return m_UdpPort != (unsigned short)(-1); }

	inline bool StateQueueEmpty() const { return m_PlayerStateHistory.empty(); }
	inline PlayerStateFrame& GetCurrentPlayerState() { assert(m_PlayerStateHistory.size() > 0 && "State history is empty!");  return m_PlayerStateHistory[0]; }
	// get the player's state t seconds ago
	sf::Vector2f GetPastPlayerPos(float t);
	void AddToStateQueue(const UpdateMessage& updateMessage);

	inline bool IsReady() const { return m_Ready; }
	inline void SetReady(bool ready) { m_Ready = ready; }

	void OnTcpConnected(ClientID id, sf::Uint8 playerNum);
	void SetUdpPort(unsigned short clientPort);

	void SendPacketTcp(sf::Packet& packet);
	void SendMessageTcp(MessageCode code);
	template<typename T>
	void SendMessageTcp(MessageCode code, T& message)
	{
		sf::Packet packet;
		MessageHeader header{ m_ID, code };
		packet << header << message;

		SendPacketTcp(packet);
	}

	inline void BeginPing(float t) { m_BeginPingTime = t; }
	inline void CalculateLatency(float t) { m_Latency = t - m_BeginPingTime; }
	inline float GetLatency() const { return m_Latency; }

private:

	float CalculateHistoryDuration();

private:
	sf::TcpSocket m_Socket;

	// network properties
	ClientID m_ID = INVALID_CLIENT_ID;
	sf::Uint8 m_PlayerNumber = -1;

	sf::IpAddress m_ClientIP = sf::IpAddress::None;
	unsigned short m_TcpPort = -1;
	unsigned short m_UdpPort = -1;

	float m_BeginPingTime = 0.0f;
	float m_Latency = 0.0f;

	// in-game player properties
	PlayerTeam m_PlayerTeam = PlayerTeam::None;
	std::deque<PlayerStateFrame> m_PlayerStateHistory;

	// ready for game to start
	bool m_Ready = false;
};
