#pragma once
#include "Node.h"


class Entity : public Node
{
public:
	Entity(Game* game);
	void SetVelocity(XMFLOAT2 vel);
	void SetVelocity(float x, float y);
	XMFLOAT2 GetVelocity() const;

	void SetAcceleration(XMFLOAT2 vel);
	void SetAcceleration(float x, float y);
	XMFLOAT2 GetAcceleration() const;
	
	virtual void UpdateSelf(const GameTimer& gt);

	XMFLOAT2 velocity;
	XMFLOAT2 acceleration;

};