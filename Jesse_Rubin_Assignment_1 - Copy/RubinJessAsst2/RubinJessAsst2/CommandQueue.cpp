#include "CommandQueue.h"


void CommandQueue::Push(const Command& com)
{
	cq.push(com);
}

Command CommandQueue::Pop()
{
	Command front = cq.front();
	cq.pop();
	return front;
}

bool CommandQueue::isEmpty() const
{
	//return cq.empty();
	if (cq.size() == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}
