#include "PrimMST.h"

void GameAI::PrimMST::CalculateMST(GameAI::Graph& Graph)
{
	Visited.clear();
	Visited.insert(Graph.GetNode(0).get());
	
	// A graph with 1 node is always an MST
	if (Graph.GetNodeCount() == 1)
	{
		return;
	}
	
	auto conn = FindCheapestConnection(Graph);
	UE_LOG(LogTemp, Log, TEXT("Cheapest connection has weight %f"), conn->GetWeight());
	
}

GameAI::Connection* GameAI::PrimMST::FindCheapestConnection(const GameAI::Graph& Graph)
{
	std::vector<GameAI::Connection*> Candidates;
	
	//Gather all candidate edges (ones that connect a unvisited node to a visited node)
	for (Node* VisitedNode : Visited)
	{
		auto ConnectionsFrom = Graph.FindConnectionsFrom(VisitedNode->GetId());
		
		for (Connection* Conn : ConnectionsFrom)
		{
			Node* TargetNode = Graph.GetNode(Conn->GetToId()).get();
			
			//Only keep the edge if it expands the current MST
			if (!Visited.contains(TargetNode))
			{
				Candidates.push_back(Conn);
			}
		}
	}
	
	if (Candidates.empty())
		return nullptr;
	
	//Select the cheapest edge from all candidates
	auto CheapestIt = 
		std::ranges::min_element(Candidates,
	    	[](const Connection* a, const Connection* b)
	    	{
			    return a->GetWeight() < b->GetWeight();
	    	});
	
	return *CheapestIt;
}
