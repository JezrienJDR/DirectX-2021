#pragma once
#include "Sprite.h"
#include "Background.h"
#include "Ship.h"
#include "Sun.h"
#include "CommandQueue.h"

class World 
{
public:
	explicit World(Game* window);
	void Update(const GameTimer& gt);
	void Draw();
	void BuildScene();

	CommandQueue& GetCQ();
private:
	//enum Layer
	//{
	//	Background,
	//	Midground,
	//	Foreground,
	//	LayerCount
	//};

	Game* game;
	Node* SceneGraph;
	//array<Node*, LayerCount> Layers;
	XMFLOAT4 worldBounds = XMFLOAT4(0.0f, 0.0f, 600.0f, 1000.0f);
	XMFLOAT2 spawnPosition = XMFLOAT2(0.0f, 0.0f);
	float scrollSpeed = 0.0f;

	Ship* player = nullptr;
	Background* Background1 = nullptr;
	Background* Background2 = nullptr;
	Sun* enemy = nullptr;
	Sprite* planet1 = nullptr;
	Sprite* planet2 = nullptr;

public:
	Command moveLeft;
	Command moveRight;
	CommandQueue mCQ;
};