#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	bool start(const char *serverAddress, int serverPort, const char *playerName);

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

	bool onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging
	};



	struct Message
	{
		Message(std::string message, Color color)
		{
			this->message = message;
			this->color = color;
		}

		std::string message;
		Color color;
	};

	Color myColor = Color(255, 255, 255);

	ClientState state = ClientState::Stopped;

	sockaddr_in serverAddress = {};
	SOCKET sk = INVALID_SOCKET; 	// NOTE: Had to change name of .h var from socket to sk due to overlapping function name


	std::vector<Message> messages;

	std::string playerName;

	ImGuiTextFilter filter;
};

