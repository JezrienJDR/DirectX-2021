#pragma once
#include "Sprite.h"
class Ship :
	public Sprite
{
public:
	Ship(Game* game, string material, float width, float height, bool AlphaTest);
	virtual void UpdateSelf(const GameTimer& gt);

	void Fire(int t);
};

