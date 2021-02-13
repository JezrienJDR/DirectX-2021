#pragma once
#include "Sprite.h"
#include "Background.h"

class World 
{
public:
	explicit World(Game* window);
	void Update(const GameTimer& gt);
	void Draw();
	void BuildScene();
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

	Sprite* player = nullptr;
	Background* Background1 = nullptr;
	Background* Background2 = nullptr;
	Sprite* enemy = nullptr;

};