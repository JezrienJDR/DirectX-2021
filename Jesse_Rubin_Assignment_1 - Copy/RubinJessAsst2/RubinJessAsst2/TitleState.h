#pragma once
#include "State.h"
class TitleState :
	public State
{
public:
	TitleState(Game* game);
	virtual void ButtonPress(char k);
};

