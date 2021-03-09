#include "PhaserPool.h"

PhaserPool::PhaserPool(Game* g, Node* n)
	: game(g), SceneGraph(n)
{
	Initialize();
}

void PhaserPool::Initialize()
{
	for (int i = 0; i < 16; i++)
	{
		Phaser* phaser;

		unique_ptr<Phaser> p(new Phaser(game, "Phaser", 3.0f, 6.0f, true, this));
		phaser = p.get();
		phaser->SetLocalScale(1.0f, 1.0f, 1.0f);
		phaser->Stop();
		SceneGraph->AddChild(std::move(p));

		shots.push(phaser);
	}

}

Phaser* PhaserPool::Fire(float x, float y)
{
	Phaser* p = shots.front();
	p->Fire(x, y);
	shots.pop();

	return p;
}

void PhaserPool::Reload(Phaser* p)
{
	p->Stop();
	shots.push(p);
}
