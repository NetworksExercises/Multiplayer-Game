#include "ModuleNetworkingClient.h"
#include <stdio.h>
#include <stdlib.h>


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
	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_port = htons(serverPort); // Port
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);

	// - Connect to the remote address
	// --- Establish connection to server ---
	connect(sk, (const struct sockaddr*)&serverAddress, sizeof(serverAddress));

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
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (sendPacket(packet, sk))
			state = ClientState::Logging;
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		if (ImGui::Begin("Client Window"))
		{
			Texture* tex = App->modResources->client;
			ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
			ImGui::Image(tex->shaderResource, texSize);

			ImGui::Text("%s connected to the server...", playerName.c_str());

			ImGuiWindowFlags consoleFlags = 0;
			consoleFlags |= ImGuiWindowFlags_NoFocusOnAppearing;

			bool enabled = true;

			if (ImGui::BeginChild("Chat", ImVec2(0, 0), false, consoleFlags))
			{
				if (ImGui::SmallButton("Clear"))
				{
					messages.clear();
				}

				ImGui::SameLine();

				filter.Draw("Filter", 200.0f);

				ImGui::Separator();

				ImGuiWindowFlags scrollFlags = 0;
				scrollFlags |= ImGuiWindowFlags_HorizontalScrollbar;
				scrollFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;

				ImGui::SetKeyboardFocusHere();

				if (ImGui::BeginChild("Chat Body", ImVec2(0, ImGui::GetWindowHeight() * 0.9f), false, scrollFlags))
				{
					// --- Print logs to console ---

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 1)); // Tighten spacing
					
					static std::string server_key = "|";

					std::string key;
					std::string item;

					for (unsigned int i = 0; i < messages.size(); ++i)
					{
						item = messages[i];
						key = item.substr(0, 1);

						// --- Display error messages in red color ---
						if (key == server_key)
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75, 0.75, 0, 255));
						else
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 255, 255, 255));

						// --- If text does not match the filter don't print it ---
						if (!filter.PassFilter(item.c_str()))
						{
							ImGui::PopStyleColor();
							continue;
						}

						ImGui::TextUnformatted(item.c_str());
						ImGui::PopStyleColor();

					}

					ImGui::PopStyleVar();
				}

				if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
					ImGui::SetScrollHereY(1.0f);

				ImGui::EndChild();

				static char msg[1000];

				if (ImGui::InputText("", msg, IM_ARRAYSIZE(msg), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
				{
					if (strcmp(msg, "/clear") == 0)
					{
						messages.clear();
					}
					else
					{
						std::string final_msg = playerName + ": " + msg;
						OutputMemoryStream packet;
						packet << ClientMessage::Message;
						packet << final_msg;

						if (!sendPacket(packet, sk))
						{
							ELOG("Message could not be sent");
						}
					}

					strcpy_s(msg, 1000, "");
				}
			}

			ImGui::End();		
		}

		ImGui::End();
	}

	return true;
}

bool ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet >> serverMessage;

	std::string msg;
	packet >> msg;

	if (serverMessage == ServerMessage::Welcome || serverMessage == ServerMessage::Message)
	{
		messages.push_back(msg);
	}
	// --- Server disconnects client since the playername is already picked ---
	else if (serverMessage == ServerMessage::UnWelcome)
	{
		WLOG("ModuleNetworkingClient::onSocketReceivedData() - %s", msg.c_str());
		return false;
	}
	else if (serverMessage == ServerMessage::Kick)
	{
		WLOG("Kicked by: %s", msg.c_str());
		return false;
	}
	else if (serverMessage == ServerMessage::ChangeName)
	{
		int senderIndex = msg.find(":");
		if (msg.length() > senderIndex + 2)
		{
			std::string senderName = msg.substr(senderIndex + 2, std::string::npos);
			playerName = senderName;
		}
			
		messages.push_back(msg);
	}
	/*else if ()
	{
		messages.push_back(msg);
	}*/
	//}
	// --- Another client has sent a message ---
	//else if (serverMessage == ServerMessage::Message)
	//{
	//	messages.push_back(msg);
	

	return true;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
	messages.clear();
}

