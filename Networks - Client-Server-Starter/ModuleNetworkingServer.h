#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingServer : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingServer public methods
	//////////////////////////////////////////////////////////////////////

	bool start(int port);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isListenSocket(SOCKET socket) const override;

	void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) override;

	bool onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;


	bool HandleHelloPacket(std::string & msg, Color & color, SOCKET socket);

	void HandleHelpCommand(Color& color, SOCKET socket);
	void HandleKickCommand(std::string& msg, Color& color, SOCKET socket);
	void HandleChangeNameCommand(std::string& msg, Color& color, SOCKET socket);
	void HandleBanCommand(std::string& msg, Color& color, SOCKET socket);
	void HandleChangeColorCommand(std::string& msg, Color& color, SOCKET socket);
	void HandleListCommand(std::string& msg, Color& color, SOCKET socket);
	void HandleWhisperCommand(std::string& msg, Color& color, SOCKET socket);


	//////////////////////////////////////////////////////////////////////
	// State
	//////////////////////////////////////////////////////////////////////

	enum class ServerState
	{
		Stopped,
		Listening
	};

	ServerState state = ServerState::Stopped;

	SOCKET listenSocket;

	struct ConnectedSocket
	{
		sockaddr_in address;
		SOCKET socket;
		std::string playerName;
		Color color = Color(255,255,255);
	};

	std::vector<ConnectedSocket> connectedSockets;
	std::vector<std::string> banned_players;
};

