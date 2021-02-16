#pragma once

#include "Entity.h"

class Sprite : public Entity
{
public:
    Sprite(Game* game, string material, float width, float height, bool AlphaTest);
    string mat;
    float width;
    float height;
    bool Alpha;

private:
    virtual void DrawSelf() const;
    virtual void BuildSelf();
    virtual void UpdateSelf(const GameTimer& gt);

};