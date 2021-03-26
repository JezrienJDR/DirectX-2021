#pragma once
#include "World.h"
#include "Player.h"


class State
{
public:
	virtual void Draw();
	virtual bool Update(const GameTimer& gt);
	virtual bool ProcessInput();

private:
	World mWorld;
	Player mPlayer;
};

