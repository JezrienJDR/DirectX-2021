#include "GameState.h"
#include "Game.hpp"

GameState::GameState(Game* game)
	: State(game)
	, phaserPool(game, GetWorld()->GetSceneGraph())
{
}

void GameState::ButtonPress(char k)
{
	if (k == 0x1B)
	{
		mGame->Pause();
	}
}

void GameState::Enter() 
{
	root->Move(0.0f, 10.0f, 0.0f);
}

void GameState::Exit() 
{
	root->Move(0.0f, -10.0f, 0.0f);
}

