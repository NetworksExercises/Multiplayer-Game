#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff

	// - Create the listenSocket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET)
	{
		reportError("ModuleNetworkingServer::start() - error on create socket");
		return false;
	}

	// --- Set address and port ---
	sockaddr_in serverAddr; // server
	serverAddr.sin_family = AF_INET; // IPv4
	serverAddr.sin_port = htons(port); // Port
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

	// - Set address reuse

	// --- Make sure we can reuse IP and port ---
	int enable = 1;
	int iResult;
	iResult = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));

	if (iResult == SOCKET_ERROR)
	{
		reportError("ModuleNetworkingServer::start() - error on setsockopt");
		return false;
	}

	// - Bind the socket to a local interface
	iResult = bind(listenSocket, (const struct sockaddr*)&serverAddr, sizeof(serverAddr));

	if (iResult != NO_ERROR)
	{
		reportError("ModuleNetworkingServer::start() - error on bind");
		return false;
	}

	// - Enter in listen mode
	listen(listenSocket, 3);

	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			// --- Socket has not yet sent its playername ---
			if (connectedSocket.playerName.empty())
				continue;
			
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

bool ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	// Set the player name of the corresponding connected socket proxy
	if (clientMessage == ClientMessage::Hello)
	{
		std::string playerName;
		packet >> playerName;

		ConnectedSocket* connectedSocketRef = nullptr;

		for (ConnectedSocket& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				// --- If playername does not exist ---
				OutputMemoryStream UnWelcomePacket;
				UnWelcomePacket << ServerMessage::UnWelcome;
				UnWelcomePacket << "Sorry, change your name";
				sendPacket(UnWelcomePacket, socket);
				return false;
			}

			if (connectedSocket.socket == socket)
			{
				connectedSocketRef = &connectedSocket;
			}
		}

		// --- If playername does not exist, send welcome package and assign name ---
		OutputMemoryStream WelcomePacket;
		WelcomePacket << ServerMessage::Welcome;
		WelcomePacket << "Welcome!!!";
		sendPacket(WelcomePacket, socket);
 
		if (connectedSocketRef)
		{
			connectedSocketRef->playerName = playerName;

			// --- Notify all users a player joined ---
			OutputMemoryStream messagePacket;
			messagePacket << ServerMessage::Message;
			std::string msg = "|[Server]: " + playerName;
			msg.append(" joined");
			messagePacket << msg;

			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				sendPacket(messagePacket, connectedSocket.socket);
			}
		}
	}
	else if (clientMessage == ClientMessage::Message)
	{
		// --- If playername does not exist ---
		OutputMemoryStream messagePacket;
		
		std::string msg;
		packet >> msg;
		

		if (msg.find("/help") != std::string::npos)
		{
			std::string commandList = "Command List : \n /clear -> Clears all messages \n /kick [username] -> Kicks the user from the chat";
			messagePacket << commandList;
			sendPacket(messagePacket, socket);
		}
		else if (msg.find("/kick") != std::string::npos)
		{

			msg.shrink_to_fit();

			//Usermame to be kicked is found by getting a substring without the sender name and /kick 
			std::string kick;
			kick = "/kick";
			int kickSize = strlen(kick.c_str());

			int senderIndex = msg.find(kick);
			std::string senderName = msg.substr(0, senderIndex - 1);

			messagePacket << ServerMessage::Kick;
			messagePacket << senderName;
			

			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				std::string player_to_kick = msg.substr(senderIndex + kickSize + 1, std::string::npos);

				if (player_to_kick == connectedSocket.playerName)
				{
					
					sendPacket(messagePacket, connectedSocket.socket);
				}
			}
		}
		else
		{
			messagePacket << ServerMessage::Message;
			messagePacket << msg;

			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				sendPacket(messagePacket, connectedSocket.socket);
			}
		}
		
	}
	//else if (clientMessage == ClientMessage::Command)
	//{
	//	// --- If playername does not exist ---
	//	OutputMemoryStream messagePacket;
	//	messagePacket << ServerMessage::Command;
	//	std::string msg;
	//	packet >> msg;

	//	
	//}

	return true;
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			// --- Notify all users a player disconnected ---
			OutputMemoryStream messagePacket;
			messagePacket << ServerMessage::Message;
			std::string msg = "|[Server]: " + connectedSocket.playerName;
			msg.append(" disconnected");
			messagePacket << msg;

			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				sendPacket(messagePacket, connectedSocket.socket);
			}

			connectedSockets.erase(it);

			break;
		}
	}
}

