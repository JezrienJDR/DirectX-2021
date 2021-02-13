#pragma once
#include "Sprite.h"
class Background :
	public Sprite
{
public:
	Background(Game* game, string material, float width, float height, bool AlphaTest);
	virtual void UpdateSelf(const GameTimer& gt);
};

