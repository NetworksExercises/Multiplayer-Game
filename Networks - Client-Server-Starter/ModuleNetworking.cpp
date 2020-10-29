#include "Networks.h"
#include "ModuleNetworking.h"
#include <iostream>


static uint8 NumModulesUsingWinsock = 0;

void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	int result = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (result == SOCKET_ERROR)
	{
		reportError("send");
		return false;
	}

	return true;
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	//const uint32 incomingDataBufferSize = Kilobytes(1);
	//byte incomingDataBuffer[incomingDataBufferSize]{"\0"};

	InputMemoryStream packet;

	int available_ops = 0;
	// TODO(jesus): select those sockets that have a read operation available
	fd_set readSet;
	FD_ZERO(&readSet);

	for (SOCKET sk : sockets)
	{
		FD_SET(sk, &readSet);
	}

	// Timeout (return immediately)
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	available_ops = select(0, &readSet, nullptr, nullptr, &timeout);

	if (available_ops == SOCKET_ERROR)
	{
		reportError("ModuleNetworking::preUpdate() - select error ");
		return false;
	}

	// Fill this array with disconnected sockets
	std::vector<SOCKET> disconnectedSockets;
	// Read selected sockets
	for (SOCKET s : sockets)
	{
		if (FD_ISSET(s, &readSet))
		{
			// TODO(jesus): for those sockets selected, check wheter or not they are
			// a listen socket or a standard socket and perform the corresponding
			// operation (accept() an incoming connection or recv() incoming data,
			// respectively).

			if (isListenSocket(s))
			{
				// It's the server socket
				// Accept stuff
				// On accept() success, communicate the new connected socket to the
				// subclass (use the callback onSocketConnected()), and add the new
				// connected socket to the managed list of sockets.
				sockaddr_in clientAddr = { 0 }; 
				int addrsize = sizeof(clientAddr);

				SOCKET connected_sk = accept(s, (struct sockaddr*)&clientAddr, &addrsize);

				if (connected_sk == INVALID_SOCKET)
				{
					reportError("ModuleNetworking::preUpdate() - error on accept");
					continue;
				}

				OutputMemoryStream packet;
				packet << ServerMessage::Welcome;
				packet << "Welcome!!!";

				sendPacket(packet, connected_sk);

				onSocketConnected(connected_sk, clientAddr);
				addSocket(connected_sk);
			}
			else
			{       
				
				// It's a client socket
				// Recv stuff
				// On recv() success, communicate the incoming data received to the
				// subclass (use the callback onSocketReceivedData()).
				int iResult = recv(s, packet.GetBufferPtr(), packet.GetCapacity(), 0);

				if (iResult == SOCKET_ERROR)
				{
					disconnectedSockets.push_back(s);
					reportError("ModuleNetworking::preUpdate() - error on recv (disconnecting socket...)");
					continue;
				}
				else if (iResult == 0)
				{
					disconnectedSockets.push_back(s);;
					continue;
				}

				packet.SetSize((uint32)iResult); // reuse iResult
				onSocketReceivedData(s, packet);
			}
		}
	}

	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().

	for (std::vector<SOCKET>::iterator it = disconnectedSockets.begin(); it != disconnectedSockets.end(); ++it)
	{
		for (std::vector<SOCKET>::iterator it2 = sockets.begin(); it2 != sockets.end(); ++it2)
		{
			if (*it == *it2)
			{
				onSocketDisconnected(*it2);
				sockets.erase(it2);
				break;
			}
		}
	}

	disconnectedSockets.clear();
	

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
