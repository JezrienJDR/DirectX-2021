#include "Entity.h"

Entity::Entity(Game* game)
	: Node(game), velocity(0, 0)
{
}

void Entity::SetVelocity(XMFLOAT2 vel)
{
	velocity = vel;
}

void Entity::SetVelocity(float x, float y)
{
	velocity = XMFLOAT2(x, y);
}

XMFLOAT2 Entity::GetVelocity() const
{
	return velocity;
}

void Entity::SetAcceleration(XMFLOAT2 vel)
{
	acceleration = vel;
}

void Entity::SetAcceleration(float x, float y)
{
	acceleration = XMFLOAT2(x, y);
}

XMFLOAT2 Entity::GetAcceleration() const
{
	return acceleration;
}

void Entity::UpdateSelf(const GameTimer& gt)
{
	velocity.x += acceleration.x * gt.DeltaTime();
	velocity.y += acceleration.y * gt.DeltaTime();

	XMFLOAT2 d;
	d.x = velocity.x * gt.DeltaTime();
	d.y = velocity.y * gt.DeltaTime();

	Move(d.x, 0, d.y);
}
