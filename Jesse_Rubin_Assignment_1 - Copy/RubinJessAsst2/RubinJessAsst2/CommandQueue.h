#pragma once
#include "Command.h"
#include <queue>

class CommandQueue
{
public:
	void Push(const Command& com);
	Command Pop();
	bool isEmpty() const;
	void Clear();

public:
	std::queue<Command> cq;
	
};

