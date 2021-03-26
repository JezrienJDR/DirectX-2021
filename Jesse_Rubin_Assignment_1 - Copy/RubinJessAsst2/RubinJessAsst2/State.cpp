#include "State.h"

void State::Draw()
{
	mWorld.Draw();
}

bool State::Update(const GameTimer& gt)
{
	mWorld.Update(gt);
}

bool State::ProcessInput()
{
	return false;
}
