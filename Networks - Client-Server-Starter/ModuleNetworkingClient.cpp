#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff
	// - Create the socket

	// NOTE: Had to change name of .h var from socket to sk due to overlapping function name
	sk = socket(AF_INET, SOCK_STREAM, 0);

	if (sk == INVALID_SOCKET)
	{
		reportError("ModuleNetworkingClient::start() - error on create socket");
		return false;
	}

	// - Create the remote address object
	// --- Set address and port ---
	sockaddr_in serverAddr; // client
	serverAddr.sin_family = AF_INET; // IPv4
	serverAddr.sin_port = htons(serverPort); // Port
	//const char* remoteAddrStr = serverAddressStr;
	inet_pton(AF_INET, serverAddressStr, &serverAddr.sin_addr);

	// - Connect to the remote address
	// --- Establish connection to server ---
	connect(sk, (const struct sockaddr*)&serverAddr, sizeof(serverAddr));

	// - Add the created socket to the managed list of sockets using addSocket()
	addSocket(sk);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		// TODO(jesus): Send the player name to the server
		int iResult = send(sk, playerName.c_str(), playerName.size(), 0);

		if (iResult == SOCKET_ERROR)
			reportError("ModuleNetworkingClient::update() - error on send");

		state = ClientState::Logging;
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, byte * data)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

