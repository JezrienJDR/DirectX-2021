#pragma once
#include "Sprite.h"
#include "PhaserPool.h"
class Ship :
	public Sprite
{
public:
	Ship(Game* game, string material, float width, float height, bool AlphaTest, PhaserPool* pp);
	virtual void UpdateSelf(const GameTimer& gt);

	void Fire(int t);

	PhaserPool* phaserPool;
};

