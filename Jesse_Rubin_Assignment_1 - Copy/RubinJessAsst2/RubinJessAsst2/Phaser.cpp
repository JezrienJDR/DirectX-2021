#include "Phaser.h"
#include "PhaserPool.h"

Phaser::Phaser(Game* game, string material, float width, float height, bool AlphaTest, PhaserPool* pp)
	: Sprite(game, material, width, height, AlphaTest), phaserPool(pp)
{
	category = Category::Projectile;
}

void Phaser::Fire(float x, float y)
{
	SetLocalPosition(x, 0.1f, y);
	SetVelocity(XMFLOAT2(0.0f, speed));
}

void Phaser::Stop()
{
	SetLocalPosition(-100, -100, -100);
	SetVelocity(XMFLOAT2(0, 0));
}

void Phaser::UpdateSelf(const GameTimer& gt)
{
	SetWorldPosition(GetLocalPosition().x + GetParent()->GetWorldPosition().x,
		GetLocalPosition().y + GetParent()->GetWorldPosition().y,
		GetLocalPosition().z + GetParent()->GetWorldPosition().z);

	XMFLOAT2 d;
	d.x = velocity.x * gt.DeltaTime();
	d.y = velocity.y * gt.DeltaTime();

	//Move(0, 0, -10);

	renderItem->NumFramesDirty++;
	Move(d.x, 0, d.y);
	renderItem->NumFramesDirty++;


	if (GetWorldPosition().z > 1000)
	{
		Stop();
		phaserPool->Reload(this);
	}
}