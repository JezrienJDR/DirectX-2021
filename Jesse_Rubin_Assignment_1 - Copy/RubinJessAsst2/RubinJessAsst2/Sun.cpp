#include "Sun.h"
Sun::Sun(Game* game, string material, float width, float height, bool AlphaTest)
	: Sprite(game, material, width, height, AlphaTest)
{

}


void Sun::UpdateSelf(const GameTimer& gt)
{

	SetWorldPosition(GetLocalPosition().x + GetParent()->GetWorldPosition().x,
		GetLocalPosition().y + GetParent()->GetWorldPosition().y,
		GetLocalPosition().z + GetParent()->GetWorldPosition().z);

	velocity.x += acceleration.x * gt.DeltaTime();
	velocity.y += acceleration.y * gt.DeltaTime();

	//SetVelocity(0.0f, -50.0f);
	//velocity.y = 1000.0f;
	XMFLOAT2 d;
	d.x = velocity.x * gt.DeltaTime();
	d.y = velocity.y * gt.DeltaTime();

	//Move(0, 0, -10);

	renderItem->NumFramesDirty++;
	Move(d.x, 0, d.y);
	renderItem->NumFramesDirty++;


}