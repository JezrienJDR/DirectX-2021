#include "World.h"

World::World(Game* window)
	: SceneGraph(new Node(game))
	, game(window)
	, player(nullptr)
	, Background1(nullptr)
	, Background2(nullptr)
	, enemy(nullptr)
	, worldBounds(0.f, 0.f, 600.0f, 200.0f)
	, spawnPosition(0.f, 0.f)
	, scrollSpeed(-0.f)
	//, Layers({new Node(game)})
{

}

void World::Update(const GameTimer& gt)
{

	SceneGraph->Update(gt);
}

void World::Draw()
{

	SceneGraph->Draw();
}

void World::BuildScene()
{
	unique_ptr<Sprite> playerShip(new Sprite(game, "Defiant", 8.1f, 10.0f, true));
	player = playerShip.get();
	player->SetLocalPosition(0.0f, 0.1f, 0.0f);
	player->SetLocalScale(1.0f, 1.0f, 1.0f);
	//player->SetVelocity(0.0f, 1.0f);
	SceneGraph->AddChild(std::move(playerShip));

	unique_ptr<Sprite> bg1(new Sprite(game, "space", 120.0f, 85.0f, true));
	Background1 = bg1.get();
	Background1->SetLocalPosition(0.0f, 0.0f, 0.0f);
	Background1->SetLocalScale(1.0f, 1.0f, 1.0f);
	Background1->SetVelocity(0.0f, -50.0f);
	SceneGraph->AddChild(std::move(bg1));

	unique_ptr<Sprite> bg2(new Sprite(game, "space", 120.0f, 85.0f, true));
	Background2 = bg2.get();
	Background2->SetWorldPosition(0.0f, 0.0f, 850.0f);
	Background2->SetLocalScale(1.0f, 1.0f, 1.0f);
	Background2->SetVelocity(0.0f, -50.0f);
	SceneGraph->AddChild(std::move(bg2));


	unique_ptr<Sprite> e(new Sprite(game, "sun", 2.0f, 2.0f, true));
	enemy = e.get();
	enemy->SetLocalPosition(-9.0f, 0.1f, 0.0f);
	enemy->SetLocalScale(1.0f, 1.0f, 1.0f);
	SceneGraph->AddChild(std::move(e));

	SceneGraph->Build();
}
