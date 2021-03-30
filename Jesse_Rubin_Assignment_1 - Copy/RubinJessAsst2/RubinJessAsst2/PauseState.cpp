#include "PauseState.h"
#include "Game.hpp"

PauseState::PauseState(Game* game)
	: State(game)
{
}

void PauseState::ButtonPress(char k)
{
	if (k == 0x1B)
	{
		mGame->UnPause();
	}
}
