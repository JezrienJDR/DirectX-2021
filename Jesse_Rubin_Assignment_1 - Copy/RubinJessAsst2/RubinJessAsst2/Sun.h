#pragma once
#include "Sprite.h"
class Sun :
	public Sprite
{
public:
	Sun(Game* game, string material, float width, float height, bool AlphaTest);
	virtual void UpdateSelf(const GameTimer& gt);
};

