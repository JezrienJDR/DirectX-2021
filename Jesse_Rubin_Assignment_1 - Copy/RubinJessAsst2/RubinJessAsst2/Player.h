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
	void WASD();
	void ArrowKeys();

	enum input
	{
		LEFT,
		RIGHT,
		UP,
		DOWN,
		FIRE,
		A,
		B,
		C,
		ESCAPE,
		ACTIONCOUNT
	};

	std::map<input, char> keyBindings;
	std::map<input, Command> actionBindings;
	std::map<input, Command> eventBindings;
	std::map<input, bool> keyDown;

};

