#pragma once
#include "World.h"
#include "Player.h"


class State
{
public:
	State(Game* game);
	virtual void Draw();
	virtual bool Update(const GameTimer& gt);
	virtual bool ProcessInput();
	virtual void ButtonPress(char k);
	World* GetWorld();
	Player* GetPlayer();
	Game* mGame;

	virtual void Enter();
	virtual void Exit();

	Node* root;

private:
	World mWorld;
	Player mPlayer;
};

