#pragma once
#include "State.h"
class MenuState :
	public State
{
public:
	MenuState(Game* game);
	virtual void ButtonPress(char k);
};

