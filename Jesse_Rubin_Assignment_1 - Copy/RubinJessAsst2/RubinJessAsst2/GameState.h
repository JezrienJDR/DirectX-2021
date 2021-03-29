#pragma once
#include "State.h"
class GameState :
	public State
{
private:
	Ship* player = nullptr;
	Background* Background1 = nullptr;
	Background* Background2 = nullptr;
	Sun* enemy = nullptr;
	Sprite* planet1 = nullptr;
	Sprite* planet2 = nullptr;

	PhaserPool phaserPool;

public:
	Command moveLeft;
	Command moveRight;
	CommandQueue mCQ;
};

