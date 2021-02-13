#include "Sprite.h"
#include "Game.hpp"

Sprite::Sprite(Game* game, string material, float width, float height, bool AlphaTest)
	: Entity(game), mat(material), width(width), height(height), Alpha(AlphaTest)
{
	
	this->game = game;
}

void Sprite::DrawSelf() const
{
	int x = renderItem->IndexCount;
	renderItem->World = GetLocalTransform();
	renderItem->NumFramesDirty++;
}

void Sprite::BuildSelf()
{
	//XMStoreFloat4x4(&renderItem->World, XMMatrixScaling(width, 1.0f, height) * XMMatrixTranslation(GetLocalPosition().x, GetLocalPosition().y, GetLocalPosition().z));

	SetWorldScale(width, 1.0f, height);
	//SetWorldScale(100, 100, 100);
	SetWorldRotation(0.0f, 0.0f, 0.0f);

	auto item = std::make_unique<RenderItem>();
	renderItem = item.get();
	renderItem->World = GetLocalTransform();
	XMStoreFloat4x4(&renderItem->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	//renderItem->ObjCBIndex = game->getRenderItems().size();
	//renderItem->ObjCBIndex = game->numRenderItems();
	renderItem->ObjCBIndex = game->nextObjCBIndex;
	game->nextObjCBIndex++;
	renderItem->Mat = game->getMaterials()[mat].get();
	renderItem->Geo = game->getGeometries()["shapeGeo"].get();
	renderItem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	renderItem->IndexCount = 6;//renderItem->Geo->DrawArgs["torus"].IndexCount;
	renderItem->StartIndexLocation = renderItem->Geo->DrawArgs["plane"].StartIndexLocation;
	renderItem->StartIndexLocation = renderItem->Geo->DrawArgs["plane"].BaseVertexLocation;



	if (Alpha)
	{
		game->mRitemLayer[(int)RenderLayer::AlphaTested].push_back(item.get());
	}
	else
	{
		game->mRitemLayer[(int)RenderLayer::Opaque].push_back(item.get());
	}

	game->getRenderItems().push_back(std::move(item));
}
