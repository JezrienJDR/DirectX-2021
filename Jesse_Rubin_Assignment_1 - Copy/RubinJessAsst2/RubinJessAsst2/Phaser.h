#pragma once
#include "Sprite.h"

class PhaserPool;

class Phaser :
	public Sprite
{
public:
	Phaser(Game* game, string material, float width, float height, bool AlphaTest, PhaserPool* pp);
	virtual void UpdateSelf(const GameTimer& gt);

	float speed = 2000.0f;

	XMFLOAT3 delta = XMFLOAT3(0.0f, 0.0f, 1.0f);

	PhaserPool* phaserPool;

	void Fire(float x, float y);
	void Stop();

};

