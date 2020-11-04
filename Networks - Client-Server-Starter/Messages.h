#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Message,
	Command
};

enum class ServerMessage
{
	Welcome,
	UnWelcome,
	Message,
	Kick,
	ChangeName,
	Ban,
	ChangeColor
};

struct Color
{
	Color() {};

	Color(unsigned int r, unsigned int g, unsigned int b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}

	unsigned int r = 255;
	unsigned int g = 255;
	unsigned int b = 255;
};


