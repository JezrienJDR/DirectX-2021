#pragma once
#include "Phaser.h"
#include <queue>

class PhaserPool
{
private:
    std::queue<Phaser*> shots;

public:


    PhaserPool(Game* g, Node* n);

    void Initialize();
    Phaser* Fire(float x, float y);
    void Reload(Phaser* p);

    Game* game;
    Node* SceneGraph;

};

