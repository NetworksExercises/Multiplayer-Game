#include "ModuleNetworkingServer.h"
#include <string>




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

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(connectedSocket.color.r / 255, connectedSocket.color.g / 255, connectedSocket.color.b / 255, 255));

			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());

			ImGui::PopStyleColor();
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
		Color col;
		packet >> playerName;
		packet >> col;
		// --- Check if the player name is in the banned list ---

		for (std::string pName : banned_players)
		{
			if (pName == playerName)
			{
				// --- If playername is banned, stop connection ---
				OutputMemoryStream UnWelcomePacket;
				UnWelcomePacket << ServerMessage::UnWelcome;
				UnWelcomePacket << "Sorry, this player is banned";
				UnWelcomePacket << Color(0, 255, 0);
				sendPacket(UnWelcomePacket, socket);
				return false;
			}
		}


		ConnectedSocket* connectedSocketRef = nullptr;

		for (ConnectedSocket& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				// --- If playername does not exist ---
				OutputMemoryStream UnWelcomePacket;
				UnWelcomePacket << ServerMessage::UnWelcome;
				UnWelcomePacket << "Sorry, this user is already connected change your name";
				UnWelcomePacket << Color(0, 255, 0);
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
		WelcomePacket << col;
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
			messagePacket << Color(0, 255, 0);

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
		Color color;
		packet >> msg;
		packet >> color;

		if (msg.find("/help") != std::string::npos)
		{
			messagePacket << ServerMessage::Message;

			std::string commandList = "Command List : \n /clear -> Clears all messages \n /kick [username] -> Kicks the user from the chat";
			messagePacket << commandList;
			messagePacket << color;
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
			std::string senderName = msg.substr(0, senderIndex - 2);

			messagePacket << ServerMessage::Kick;
			messagePacket << senderName;
			messagePacket << color;

			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				std::string player_to_kick;

				if (msg.length() > (senderIndex + kickSize + 1))
				{
					player_to_kick = msg.substr(senderIndex + kickSize + 1, std::string::npos);

					if (player_to_kick == connectedSocket.playerName)
					{
						sendPacket(messagePacket, connectedSocket.socket);
						onSocketDisconnected(connectedSocket.socket);
						break;
					}
				}
			}
		}
		else if (msg.find("/change_name") != std::string::npos)
		{
			msg.shrink_to_fit();

			//Usermame to be changed is found by getting a substring without the sender name and /kick 
			std::string change_name;
			change_name = "/change_name";
			int changeSize = strlen(change_name.c_str());

			int senderIndex = msg.find(change_name);
			std::string senderName = msg.substr(0, senderIndex - 2);

			messagePacket << ServerMessage::ChangeName;
			
			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				std::string name_to_change;

				if (msg.length() > (senderIndex + changeSize + 1))
				{
					name_to_change = msg.substr(senderIndex + changeSize + 1, std::string::npos);

					if (senderName == connectedSocket.playerName)
					{
						connectedSocket.playerName = name_to_change;

						std::string notice = (" has changed his name to: ");
						senderName.append(notice);
						senderName.append(name_to_change);
						messagePacket << senderName;
						messagePacket << color;

						sendPacket(messagePacket, socket);
						break;
					}
				}
			}
		}
		else if (msg.find("/ban") != std::string::npos)
		{
			msg.shrink_to_fit();

			//Usermame to be changed is found by getting a substring without the sender name and /kick 
			std::string ban;
			ban = "/ban";
			int changeSize = strlen(ban.c_str());

			int senderIndex = msg.find(ban);
			std::string senderName = msg.substr(0, senderIndex - 2);

			messagePacket << ServerMessage::Ban;

			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				std::string name_to_ban;

				if (msg.length() > (senderIndex + changeSize + 1))
				{
					name_to_ban = msg.substr(senderIndex + changeSize + 1, std::string::npos);

					if (name_to_ban == connectedSocket.playerName)
					{
						messagePacket << senderName;
						messagePacket << color;

						banned_players.push_back(name_to_ban);

						sendPacket(messagePacket, connectedSocket.socket);

						onSocketDisconnected(connectedSocket.socket);
						break;
					}
				}
			}
		}
		else if (msg.find("/change_color") != std::string::npos)
		{
			// --- Syntax: /change_color 255,255,255 ---

			msg.shrink_to_fit();

			//Usermame to be changed is found by getting a substring without the sender name and /kick 
			std::string col;
			col = "/change_color";
			int changeSize = strlen(col.c_str());

			int senderIndex = msg.find(col);
			std::string senderName = msg.substr(0, senderIndex - 2);

			Color new_color;

			int color_index = senderIndex + changeSize;

			std::string color_msg = msg.substr(color_index + 1, std::string::npos);


			int coma_index = color_msg.find("/");
			std::string color_value;

			if (coma_index != std::string::npos)
			{
				// --- Find red value ---
				color_value = color_msg.substr(0, coma_index);

				try
				{
					new_color.r = std::stoi(color_value);
				}
				catch (std::invalid_argument)
				{
					LOG("Couldn't read color from: %s", senderName.c_str());
					return true;
				}

				color_msg = color_msg.substr(coma_index + 1, std::string::npos);

				// --- Find green value ---
				coma_index = color_msg.find("/");

				if (coma_index != std::string::npos)
				{
					color_value = color_msg.substr(0, coma_index);

					try
					{
						new_color.g = std::stoi(color_value);
					}
					catch (std::invalid_argument)
					{
						LOG("Couldn't read color from: %s", senderName.c_str());
						return true;
					}
				}

				// --- Find blue value;

				//color_msg = color_msg.substr(coma_index + 1, std::string::npos);
				coma_index = color_msg.find("/");

				if (coma_index != std::string::npos)
				{
					color_value = color_msg.substr(coma_index + 1, std::string::npos);

					try
					{
						new_color.b = std::stoi(color_value);
					}
					catch (std::invalid_argument)
					{
						LOG("Couldn't read color from: %s", senderName.c_str());
						return true;
					}
				}

				std::string new_message = senderName;
				new_message.append(" changed color.");

				messagePacket << ServerMessage::ChangeColor;
				messagePacket << new_message;
				messagePacket << new_color;

				for (ConnectedSocket& connectedSocket : connectedSockets)
				{
					if (connectedSocket.socket == socket)
						connectedSocket.color = new_color;

					sendPacket(messagePacket, connectedSocket.socket);
				}


				//sendPacket(messagePacket, socket);		
			}

		}
		else if (msg.find("/list") != std::string::npos)
		{
			std::string tmp;
			tmp = "The connected players are: \n";

			messagePacket << ServerMessage::Message;
			
			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				std::string connectedPlayer = connectedSocket.playerName + "\n";
				tmp.append(connectedPlayer);
			}

			messagePacket << tmp;
			messagePacket << color;
			sendPacket(messagePacket, socket);
		}
		else
		{
			messagePacket << ServerMessage::Message;
			messagePacket << msg;
			messagePacket << color;

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
		if (connectedSocket.socket == socket && !connectedSocket.playerName.empty())
		{
			// --- Notify all users a player disconnected ---
			OutputMemoryStream messagePacket;
			messagePacket << ServerMessage::Message;
			std::string msg = "|[Server]: " + connectedSocket.playerName;
			msg.append(" disconnected");
			messagePacket << msg;
			messagePacket << Color(0,255,0);

			for (ConnectedSocket& connectedSocket : connectedSockets)
			{
				sendPacket(messagePacket, connectedSocket.socket);
			}

			connectedSockets.erase(it);

			break;
		}
	}
}

