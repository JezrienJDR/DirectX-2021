#pragma once
#include "CommandQueue.h"
#include <map>


class Player
{
public:
	Player();
	void HandleEvents(CommandQueue& cq);
	void HandleRealtimeInput(CommandQueue& cq);

	void Setup();
	

	enum input
	{
		LEFT,
		RIGHT,
		UP,
		DOWN,
		FIRE,
		ACTIONCOUNT
	};

	std::map<input, char> keyBindings;
	std::map<input, Command> actionBindings;
	std::map<input, Command> eventBindings;
	std::map<input, bool> keyDown;

};

