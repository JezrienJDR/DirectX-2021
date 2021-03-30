#pragma once
#include "State.h"
class PauseState :
	public State
{
public:
	PauseState(Game* game);
	virtual void ButtonPress(char k);
};

