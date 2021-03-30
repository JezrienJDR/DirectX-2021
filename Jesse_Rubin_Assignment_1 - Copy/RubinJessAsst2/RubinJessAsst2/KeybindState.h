#pragma once
#include "State.h"

class KeybindState : public State
{
public:
	KeybindState(Game* game);
	virtual void ButtonPress(char k);
};

