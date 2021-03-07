#pragma once
#include <functional>
#include "Node.h"
#include "Category.h"
//#include "Ship.h"

class Node;
class Ship;

struct Command
{
	Command();

	std::function<void(Node&, const GameTimer&)> action;
	unsigned int category;
};

template <typename GameObject, typename Function>
std::function<void(Node&, const GameTimer&)> derivedAction(Function fn)
{
	return [=](Node& node, const GameTimer& gt)
	{
		// Check if cast is safe
		assert(dynamic_cast<GameObject*>(&node) != nullptr);

		// Downcast node and invoke function on it
		fn(static_cast<GameObject&>(node), gt);
	};
}
