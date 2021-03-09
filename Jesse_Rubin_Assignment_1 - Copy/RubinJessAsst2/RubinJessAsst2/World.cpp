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
	, phaserPool(game, SceneGraph)
	//, Layers({new Node(game)})
{

}

CommandQueue& World::GetCQ() 
{
	return mCQ;
}

void World::Update(const GameTimer& gt)
{

	while (!mCQ.isEmpty())
	{
		std::cout << "Executing command";
		SceneGraph->OnCommand(mCQ.Pop(), gt);
	}

	SceneGraph->Update(gt);

	vec3 planetPos = vec3(planet1->GetLocalPosition().x, planet1->GetLocalPosition().y, planet1->GetLocalPosition().z );
	planetPos = planetPos.Yrotate(gt.DeltaTime() * 100.0f);
	planet1->SetLocalPosition(planetPos.x, planetPos.y, planetPos.z);

	vec3 planetPos2 = vec3(planet2->GetLocalPosition().x, planet2->GetLocalPosition().y, planet2->GetLocalPosition().z);
	planetPos2 = planetPos2.Yrotate(gt.DeltaTime() * 200.0f);
	planet2->SetLocalPosition(planetPos2.x, planetPos2.y, planetPos2.z);


	//if (GetAsyncKeyState(0x41))
	//{
	//	//player->Move(-0.1f, 0.0f, 0.0f);
	//	SceneGraph->OnCommand(moveLeft, gt);
	//}
	//else if (GetAsyncKeyState(0x44))
	//{
	//	//player->Move(0.1f, 0.0f, 0.0f);
	//	SceneGraph->OnCommand(moveRight, gt);
	//}


}

void World::Draw()
{

	SceneGraph->Draw();
}

//void MoveLeft(Node& n, const GameTimer& gt)
//{
//	//n.Move(-10.0f * gt.DeltaTime, 0.0f, 0.0f);
//	n.Move(-0.1f, 0.0f, 0.0f);
//}

//void MoveRight(Node& n, const GameTimer& gt)
//{
//	//n.Move(10.0f * gt.DeltaTime, 0.0f, 0.0f);
//	n.Move(0.1f, 0.0f, 0.0f);
//}

void World::BuildScene()
{
	unique_ptr<Ship> playerShip(new Ship(game, "Defiant", 8.1f, 10.0f, true, &phaserPool));
	player = playerShip.get();
	player->SetWorldPosition(0.0f, 1.1f, 0.0f);
	player->SetLocalScale(1.0f, 1.0f, 1.0f);
	//player->SetVelocity(0.0f, 1.0f);
	SceneGraph->AddChild(std::move(playerShip));

	unique_ptr<Background> bg1(new Background(game, "space", 120.0f, 85.0f, true));
	Background1 = bg1.get();
	Background1->SetLocalPosition(0.0f, 0.0f, 0.0f);
	Background1->SetLocalScale(1.0f, 1.0f, 1.0f);
	Background1->SetVelocity(0.0f, -50.0f);
	SceneGraph->AddChild(std::move(bg1));

	unique_ptr<Background> bg2(new Background(game, "space", 120.0f, 85.0f, true));
	Background2 = bg2.get();
	Background2->SetLocalPosition(0.0f, 0.0f, 850.0f);
	Background2->SetLocalScale(1.0f, 1.0f, 1.0f);
	Background2->SetVelocity(0.0f, -50.0f);
	SceneGraph->AddChild(std::move(bg2));


	unique_ptr<Sun> e(new Sun(game, "sun", 20.0f, 20.0f, true));
	enemy = e.get();
	//enemy->SetWorldPosition(-250.0f, 0.05f, -200.0f);
	enemy->SetLocalPosition(-250.0f, 0.05f, 350.0f);
	enemy->SetLocalScale(1.0f, 1.0f, 1.0f);
	enemy->SetVelocity(0.0f, -35.0f);

	unique_ptr<Sprite> p1(new Sprite(game, "planet", 5.0f, 5.0f, true));
	planet1 = p1.get();
	planet1->SetLocalPosition(80.0f, -0.01f, 0.0f);
	//planet1->SetLocalPosition(0.0f, 0.15f, 0.0f);
	planet1->SetLocalScale(1.0f, 1.0f, 1.0f);

	unique_ptr<Sprite> p2(new Sprite(game, "planet", 3.0f, 3.0f, true));
	planet2 = p2.get();
	planet2->SetLocalPosition(-120.0f, 0.15f, 0.0f);
	//planet1->SetLocalPosition(0.0f, 0.15f, 0.0f);
	planet2->SetLocalScale(1.0f, 1.0f, 1.0f);


	SceneGraph->AddChild(std::move(e));
	enemy->AddChild(std::move(p1));
	enemy->AddChild(std::move(p2));

	SceneGraph->Build();

	//moveLeft.action = &MoveLeft;
	/*moveRight.action = &MoveRight;*/

	moveLeft.category = Category::Player;
	moveRight.category = Category::Player;
}



