#include "TitleState.h"
#include "Game.hpp"

TitleState::TitleState(Game* game)
	: State(game)
{
}

void TitleState::ButtonPress(char k)
{
	if (k == 'C')
	{
		mGame->MainMenu();
	}
}
