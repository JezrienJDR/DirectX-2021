#include "MenuState.h"
#include "Game.hpp"

MenuState::MenuState(Game* game)
	: State(game)
{
}

void MenuState::ButtonPress(char k)
{
	if (k == 'A')
	{
		mGame->StartGame();

	}
	else if (k == 'B')
	{
		mGame->Keybind();
	}
	else if (k == 'D')
	{
		// Quit game
	}
}
