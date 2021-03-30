#include "State.h"

State::State(Game* game)
	: mWorld(game, this)
	, mGame(game)
{

}

void State::Draw()
{
	mWorld.Draw();
}

bool State::Update(const GameTimer& gt)
{
	mWorld.Update(gt);
	return true;
}

bool State::ProcessInput()
{
	return false;
}

void State::ButtonPress(char k)
{
	// subclasses will define.
}

World* State::GetWorld()
{
	return &mWorld;
}

Player* State::GetPlayer()
{
	return &mPlayer;
}

void State::Enter()
{
	//mWorld.GetSceneGraph()->SetWorldPosition(0.0f, 0.0f, 0.0f);
	//mWorld.GetSceneGraph()->Move(0.0f, 10.0f, 0.0f);

	root->Move(0.0f, 100.0f, 0.0f);


	mWorld.mCQ.Clear();
	
}

void State::Exit()
{
	//mWorld.GetSceneGraph()->SetWorldPosition(0.0f, -10.0f, 0.0f);
	//mWorld.GetSceneGraph()->Move(0.0f, -10.0f, 0.0f);

	root->Move(0.0f, -100.0f, 0.0f);
}
