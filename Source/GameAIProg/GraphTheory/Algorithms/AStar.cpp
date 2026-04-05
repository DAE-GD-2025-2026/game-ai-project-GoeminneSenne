#include "AStar.h"

#include <set>
#include <unordered_map>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path;
	
	std::vector<NodeRecord> openList;
	std::unordered_map<int, NodeRecord> visited;
	
	NodeRecord startRecord{pStartNode, nullptr, 0.f, GetHeuristicCost(pStartNode, pGoalNode)};
	openList.push_back(startRecord);
	
	NodeRecord currentNodeRecord{};
	while (!openList.empty())
	{
		currentNodeRecord = *std::min_element(openList.begin(), openList.end());
		
		if (currentNodeRecord.pNode == pGoalNode) break;
		
		std::erase(openList, currentNodeRecord);
		
		int currentId = currentNodeRecord.pNode->GetId();
		visited[currentId] = currentNodeRecord;
		
		auto connections = pGraph->FindConnectionsFrom(currentId);
		
		for (auto connection : connections)
		{
			Node* pNextNode = pGraph->GetNode(connection->GetToId()).get();
			int nextId = pNextNode->GetId();
			
			float nextGcost = currentNodeRecord.costSoFar + connection->GetWeight();
			
			auto visitedIt = visited.find(nextId);
			if (visitedIt != visited.end())
			{
				if (visitedIt->second.costSoFar <= nextGcost)
					continue;

				visited.erase(visitedIt);
			}

			auto openListIt = std::ranges::find_if(openList,
			   	[nextId](const NodeRecord& record)
			   	{
				    return record.pNode->GetId() == nextId;
			   	});

			if (openListIt != openList.end())
			{
				if (openListIt->costSoFar <= nextGcost)
					continue;

				openList.erase(openListIt);
			}
			
			float hCost = GetHeuristicCost(pNextNode, pGoalNode);
			NodeRecord nextRecord{pNextNode, connection, nextGcost, nextGcost + hCost};
			openList.push_back(nextRecord);
		}
	}
	
	//Path reconstruction
	while (currentNodeRecord.pNode != pStartNode)
	{
		path.push_back(currentNodeRecord.pNode);
		int fromId = currentNodeRecord.pConnection->GetFromId();
			
		auto it = visited.find(fromId);
		currentNodeRecord = it->second;
	}
		
	path.push_back(pStartNode);
	std::ranges::reverse(path);
		
	return path;
}

float AStar::GetHeuristicCost(Node* const pStartNode, Node* const pEndNode) const
{
	FVector2D toDestination = pGraph->GetNode(pEndNode->GetId())->GetPosition() - pGraph->GetNode(pStartNode->GetId())->GetPosition();
	return HeuristicFunction(abs(toDestination.X), abs(toDestination.Y));
}