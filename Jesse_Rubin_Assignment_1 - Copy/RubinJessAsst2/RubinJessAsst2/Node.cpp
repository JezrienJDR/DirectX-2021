#include "Node.h"
#include "Game.hpp"


Node::Node(Game* game)
	: children(), parent(nullptr), game(game) //, renderItem(new RenderItem)
{
	worldPosition = XMFLOAT3(0, 0, 0);
	worldScale = XMFLOAT3(1, 1, 1);
	worldRotation = XMFLOAT3(0, 0, 0);

	localPosition = XMFLOAT3(0, 0, 0);
	localScale = XMFLOAT3(1, 1, 1);
	localRotation = XMFLOAT3(0, 0, 0);
	
	category = Category::None;

}

void Node::AddChild(ptr child)
{
	child->parent = this;
	children.push_back(std::move(child));
}

std::unique_ptr<Node> Node::RemoveChild(const Node& node)
{
	auto found = std::find_if
	(
		children.begin(), children.end(), [&](ptr& p)
		{
			return p.get() == &node;
		}
	);

	assert(found != children.end());

	ptr result = std::move(*found);
	result->parent = nullptr;
	children.erase(found);

	return result;

}

void Node::Update(const GameTimer& gt)
{

	UpdateSelf(gt);
	UpdateChildren(gt);
}

void Node::Draw() const
{

	DrawSelf();
	DrawChildren();
}

void Node::Build()
{

	BuildSelf();
	BuildChildren();
}

void Node::OnCommand(const Command& comm, const GameTimer& gt)
{
	if (comm.category & category) 
	{
		comm.action(*this, gt);
	}

	for (ptr& child : children)
	{
		child->OnCommand(comm, gt);
	}
}

XMFLOAT3 Node::GetWorldPosition() const
{
	return worldPosition;
}

void Node::SetWorldPosition(float x, float y, float z)
{
	worldPosition = XMFLOAT3(x, y, z);
}

XMFLOAT3 Node::GetLocalPosition() const
{
	return localPosition;
}

void Node::SetLocalPosition(float x, float y, float z)
{
	localPosition = XMFLOAT3(x, y, z);
}

XMFLOAT3 Node::GetWorldRotation() const
{
	return worldRotation;
}

void Node::SetWorldRotation(float x, float y, float z)
{
	worldRotation = XMFLOAT3(x, y, z);
}

XMFLOAT3 Node::GetWorldScale() const
{
	return worldScale;
}

XMFLOAT3 Node::GetLocalScale() const
{
	return localScale;
}

void Node::SetWorldScale(float x, float y, float z)
{
	worldScale = XMFLOAT3(x, y, z);
}

void Node::SetLocalScale(float x, float y, float z)
{
	localScale = XMFLOAT3(x, y, z);
}

XMFLOAT4X4 Node::GetWorldTransform() const
{
	XMFLOAT4X4 transform = MathHelper::Identity4x4();
	XMMATRIX T = XMLoadFloat4x4(&transform);
	
	for (const Node* node = this; node != nullptr; node = node->parent)
	{
		XMMATRIX Tp = XMLoadFloat4x4(&node->GetLocalTransform());
		T = Tp * T;
	}
	XMStoreFloat4x4(&transform, T);

	return transform;
}

XMFLOAT4X4 Node::GetLocalTransform() const
{
	XMFLOAT4X4 transform;
	XMStoreFloat4x4(&transform, XMMatrixScaling(worldScale.x, worldScale.y, worldScale.z) * 
		XMMatrixRotationX(worldRotation.x) *
		XMMatrixRotationY(worldRotation.y) *
		XMMatrixRotationZ(worldRotation.z) *
		XMMatrixTranslation(worldPosition.x, worldPosition.y, worldPosition.z));

	return transform;
}

void Node::Move(float x, float y, float z)
{
	localPosition.x += x;
	localPosition.y += y;
	localPosition.z += z;

	renderItem->NumFramesDirty++;
}

Node* Node::GetParent()
{
	return parent;
}

void Node::UpdateSelf(const GameTimer& gt)
{
	// subclasses will define
}

void Node::UpdateChildren(const GameTimer& gt)
{
	for (ptr& child : children)
	{
		child->Update(gt);
	}
}

void Node::DrawSelf() const
{
	// subclasses will define
}

void Node::DrawChildren() const
{
	for (const ptr& child : children)
	{
		child->Draw();
	}
}

void Node::BuildSelf()
{
	// subclasses will define
}

void Node::BuildChildren()
{
	for (const ptr& child : children)
	{
		child->Build();
	}
}
