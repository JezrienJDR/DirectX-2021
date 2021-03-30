#include "Player.h"
#include "Ship.h"
#include "CommandQueue.h"

struct ShipMover
{
	XMFLOAT3 delta;

	float speed = 500.0f;

	ShipMover(float x, float y, float z)
		: delta(x, y, z)
	{	}

	void operator() (Node& n, const GameTimer& gt) const
	{
		Ship& ship = static_cast<Ship&>(n);
		ship.Move(delta.x * gt.DeltaTime() * speed, delta.y, delta.z * gt.DeltaTime() * speed);
	}
};

struct ShipFire
{
	int type;

	ShipFire(int t)
		: type(t)
	{	}

	void operator() (Node& n, const GameTimer& gt) const
	{
		Ship& ship = static_cast<Ship&>(n);
		ship.Fire(type);
	}
};

struct ButtonPress
{
	char key;

	ButtonPress(char k)
		: key(k)
	{}

	void operator() (Node& n, const GameTimer& gt) const
	{
		Node& node = n;
		node.ButtonPress(key);
	}
};

Player::Player()
{
	Setup();
}



void Player::Setup()
{
	keyBindings[LEFT] = 'A';
	keyBindings[RIGHT] = 'D';
	keyBindings[UP] = 'W';
	keyBindings[DOWN] = 'S';
	keyBindings[FIRE] = 0x20; // Spacebar
	keyBindings[A] = 'A';
	keyBindings[B] = 'B';
	keyBindings[C] = 'C';
	keyBindings[ESCAPE] = 0x1B;

	actionBindings[LEFT].action = derivedAction<Ship>(ShipMover(-1.0f, 0.0f, 0.0f));
	actionBindings[RIGHT].action = derivedAction<Ship>(ShipMover(1.0f, 0.0f, 0.0f));
	actionBindings[UP].action = derivedAction<Ship>(ShipMover(0.0f, 0.0f, 1.0f));
	actionBindings[DOWN].action = derivedAction<Ship>(ShipMover(0.0f, 0.0f, -1.0f));

	eventBindings[FIRE].action = derivedAction <Ship>(ShipFire(1));
	eventBindings[A].action = derivedAction <Node>(ButtonPress('A'));
	eventBindings[B].action = derivedAction <Node>(ButtonPress('B'));
	eventBindings[C].action = derivedAction <Node>(ButtonPress('C'));
	eventBindings[ESCAPE].action = derivedAction <Node>(ButtonPress(0x1B));


	actionBindings[LEFT].category = Category::Player;
	actionBindings[RIGHT].category = Category::Player;
	actionBindings[UP].category = Category::Player;
	actionBindings[DOWN].category = Category::Player;

	eventBindings[FIRE].category = Category::Player;
	eventBindings[A].category = Category::UI;
	eventBindings[B].category = Category::UI;
	eventBindings[C].category = Category::UI;
	eventBindings[ESCAPE].category = Category::UI;

	keyDown[LEFT] = false;
	keyDown[RIGHT] = false;
	keyDown[UP] = false;
	keyDown[DOWN] = false;
	keyDown[FIRE] = false;
	keyDown[A] = false;
	keyDown[B] = false;
	keyDown[C] = false;
	keyDown[ESCAPE] = false;
	
}

void Player::WASD()
{
	keyBindings[LEFT] = 'A';
	keyBindings[RIGHT] = 'D';
	keyBindings[UP] = 'W';
	keyBindings[DOWN] = 'S';
}

void Player::ArrowKeys()
{
	keyBindings[LEFT] = 0x25;
	keyBindings[RIGHT] = 0x27;
	keyBindings[UP] = 0x26;
	keyBindings[DOWN] = 0x28;
}

void Player::HandleEvents(CommandQueue& cq)
{
	for (auto e : eventBindings)
	{
		if (GetAsyncKeyState(keyBindings[e.first]))
		{
			if (!keyDown[e.first])
			{
				cq.Push(e.second);
				keyDown[e.first] = true;
			}
		}
		else
		{
			keyDown[e.first] = false;
		}
	}
}
//void MoveLeft(Node& n, const GameTimer& gt)
//{
//	//n.Move(-10.0f * gt.DeltaTime, 0.0f, 0.0f);
//	n.Move(-0.1f, 0.0f, 0.0f);
//}
//


void Player::HandleRealtimeInput(CommandQueue& cq)
{
	for (auto a : actionBindings)
	{
		if (GetAsyncKeyState(keyBindings[a.first]))
		{
			cq.Push(a.second);
		}
	}


	//if (GetAsyncKeyState(0x41))
	//{
	//	//player->Move(-0.1f, 0.0f, 0.0f);
	//	//SceneGraph->OnCommand(moveLeft, gt);

	//	Command moveLeft;
	//	moveLeft.category = Category::Player;
	//	moveLeft.action = derivedAction<Ship>(ShipMover(-1.1f, 0.0f, 0.0f));
	//	//moveLeft.action = &MoveLeft;

	//	cq.Push(moveLeft);

	//	std::cout << cq.cq.size();
	//}
	//else if (GetAsyncKeyState(0x44))
	//{
	//	//player->Move(0.1f, 0.0f, 0.0f);
	//	//SceneGraph->OnCommand(moveRight, gt);
	//	Command moveRight;
	//	moveRight.category = Category::Player;
	//	moveRight.action = derivedAction<Ship>(ShipMover(1.1f, 0.0f, 0.0f));

	//	cq.Push(moveRight);
	//}
}
