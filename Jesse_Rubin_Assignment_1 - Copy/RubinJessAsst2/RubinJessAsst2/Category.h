#pragma once

namespace Category 
{
	enum Type
	{
		None = 0,
		Scene = 1 << 0,
		Player = 1 << 1,
		Ally = 1 << 2,
		Enemy = 1 << 3,
		Projectile = 1 << 4,

	};

}