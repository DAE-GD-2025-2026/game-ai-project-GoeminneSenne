#pragma once
#include "Shared/Graph/Graph.h"
#include <unordered_set>

namespace GameAI
{
	class PrimMST
	{
	public:
		void CalculateMST(GameAI::Graph& Graph);
	
	private:
		Connection* FindCheapestConnection(const GameAI::Graph& Graph);
		
		std::unordered_set<Node*> Visited;
	};
}
