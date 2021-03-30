#pragma once


#include "d3dApp.h"
#include "MathHelper.h"
#include "UploadBuffer.h"
#include "GeometryGenerator.h"
#include "myGeometry.h"
#include "Camera.h"
#include "FrameResource.h"
#include "Category.h"
#include "Command.h"
//#include "State.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;
using namespace std;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

class State;

struct RenderItem
{
public:
	RenderItem() = default;

	string name = "missingNo";

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;


	vector<RenderItem*> children;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	vec3 position = vec3(0, 0, 0);
	vec3 localPos = vec3(0, 0, 0);
	vec3 scale = vec3(1, 1, 1);

	vec3 velocity = vec3(0, 0, 0);
	vec3 acceleration = vec3(0, 0, 0);

	float width = 1;
	float height = 1;


};

class Game;
class Command;

class Node {
public:
	typedef std::unique_ptr<Node> ptr;

public:
	Node(Game* game);

public:
	void AddChild(ptr child);
	ptr RemoveChild(const Node& node);
	void Update(const GameTimer& gt);
	void Draw() const;
	void Build();

	void OnCommand(const Command& comm, const GameTimer& gt);

	XMFLOAT3 GetWorldPosition() const;
	void SetWorldPosition(float x, float y, float z);
	XMFLOAT3 GetLocalPosition() const;
	void SetLocalPosition(float x, float y, float z);

	XMFLOAT3 GetWorldRotation() const;
	void SetWorldRotation(float x, float y, float z);
	XMFLOAT3 GetWorldScale() const;
	XMFLOAT3 GetLocalScale() const;
	void SetWorldScale(float x, float y, float z);
	void SetLocalScale(float x, float y, float z);

	XMFLOAT4X4 GetWorldTransform() const;
	XMFLOAT4X4 GetLocalTransform() const;

	void Move(float x, float y, float z);

	Node* GetParent();

	virtual void ButtonPress(char k);

	virtual void UpdateSelf(const GameTimer& gt);

	State* state;
	bool isRoot;

private:
	void UpdateChildren(const GameTimer& gt);

	virtual void DrawSelf() const;
	void DrawChildren() const;
	virtual void BuildSelf();
	void BuildChildren();

public:
	Game* game;
	RenderItem* renderItem;

	Category::Type category;

private:
	XMFLOAT3 worldPosition;
	XMFLOAT3 worldRotation;
	XMFLOAT3 worldScale;

	XMFLOAT3 localPosition;
	XMFLOAT3 localRotation;
	XMFLOAT3 localScale;

	vector<ptr> children;
	Node* parent;

};