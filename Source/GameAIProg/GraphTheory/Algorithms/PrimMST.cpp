#include "PrimMST.h"

void GameAI::PrimMST::CalculateMST(GameAI::Graph& Graph)
{
	// A graph with 1 node is always an MST
	if (Graph.GetNodeCount() == 1)
	{
		return;
	}
	
	Visited.clear();
	std::unordered_set<Connection*> MstEdges;
	
	//Start construction from the first node in the graph
	Visited.insert(Graph.GetNode(0).get());
	
	while (Visited.size() < Graph.GetNodeCount())
	{
		Connection* CheapestConn = FindCheapestConnection(Graph);
		
		//If no edge is found the graph is disconnected
		if (CheapestConn == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Graph is disconnected, cannot construct MST"));
			return;
		}
		
		//Add new node to visited
		// & new edge to MSTEdges
		MstEdges.insert(CheapestConn);
		Visited.insert(Graph.GetNode(CheapestConn->GetToId()).get());
		
		//If graph is bidirectional, also add the inverse connection to MSTEdges
		if (Graph.GetIsDirectional())
		{
			auto InverseConnection = Graph.FindConnection(CheapestConn->GetToId(), CheapestConn->GetFromId());
			MstEdges.insert(InverseConnection);
		}
	}
	
	//Remove all edges that are not in MstEdges
	//auto& Connections = Graph.GetConnections();
	auto Removed = std::ranges::remove_if( Graph.GetConnections(),
		[&](const std::unique_ptr<Connection>& Conn)
		{
			return not MstEdges.contains(Conn.get());
		});
	
	 Graph.GetConnections().erase(Removed.begin(), Removed.end());
}

GameAI::Connection* GameAI::PrimMST::FindCheapestConnection(const GameAI::Graph& Graph)
{
	std::vector<GameAI::Connection*> Candidates;
	
	//Gather all candidate edges (ones that connect an unvisited node to a visited node)
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
