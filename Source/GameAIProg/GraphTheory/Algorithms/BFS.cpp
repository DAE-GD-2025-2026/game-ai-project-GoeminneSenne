#include "BFS.h"

#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "Shared/Graph/Graph.h"

using namespace GameAI;

BFS::BFS(Graph* const pGraph)
	: pGraph(pGraph)
{
}

// TODO Breath First Search Algorithm searches for a path from the startNode to the destinationNode
std::vector<Node*> BFS::FindPath(Node* const pStartNode, Node* const pDestinationNode) const
{
	std::vector<Node*> path;

	std::queue<Node*> queue{};
	queue.push(pStartNode);
	std::unordered_set<Node*> visited;
	std::unordered_map<Node*, Node*> parentMap;
	
	//Add StartNode to path
	while (not queue.empty())
	{
		auto node = queue.front();
		queue.pop();
		
		if (node == pDestinationNode)
		{
			while (node != pStartNode)
			{
				path.push_back(node);
				node = parentMap[node];
			}
			
			path.push_back(node);
			std::ranges::reverse(path);
			return path;
		}


		auto NeighborConnections = pGraph->FindConnectionsFrom(node->GetId());
		std::vector<Node*> neighbors;
		for (auto connection : NeighborConnections)
		{
			neighbors.push_back(pGraph->GetNode(connection->GetToId()).get());
		}
		
		//Check all neighbors and add them to queue and parentMap
		for (auto neighbor : neighbors)
		{
			if (not visited.contains(neighbor))
			{
				visited.insert(neighbor);
				parentMap[neighbor] = node;
				queue.push(neighbor);
			}
		}
		
	}
	
	return std::vector<Node*>();
}
