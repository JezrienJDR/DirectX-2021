#include "KeybindState.h"
#include "Game.hpp"

KeybindState::KeybindState(Game* game)
	: State(game)
{
}

void KeybindState::ButtonPress(char k)
{
	if (k == 'A')
	{
		mGame->WASD();
		// WASD	
	}
	else if (k == 'B')
	{
		mGame->ArrowKeys();
		// ARROW KEYS
	}
	else if (k == 'C')
	{
		mGame->MainMenu();
	}
}
