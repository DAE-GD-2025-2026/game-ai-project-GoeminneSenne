#include "AStar.h"

#include <set>

using namespace GameAI;

AStar::AStar(Graph* const pGraph, HeuristicFunctions::Heuristic hFunction)
	: pGraph(pGraph)
	, HeuristicFunction(hFunction)
{
}

std::vector<Node*>AStar::FindPath(Node* const pStartNode, Node* const pGoalNode)
{
	std::vector<Node*> path{};
	
	std::vector<NodeRecord> openList;
	std::set<NodeRecord> visited;
	
	NodeRecord startRecord{pStartNode, nullptr, 0.f, GetHeuristicCost(pStartNode, pGoalNode)};
	openList.push_back(startRecord);
	
	NodeRecord currentNodeRecord{};
	
	while (!openList.empty())
	{
		currentNodeRecord = *std::min_element(openList.begin(), openList.end());
		
		if (currentNodeRecord.pNode == pGoalNode) break;
		
		//Get All Neighbors
		auto Connections = pGraph->FindConnectionsFrom(currentNodeRecord.pNode->GetId());
		for (auto connection : Connections)
		{
			auto pNextNode = pGraph->GetNode(connection->GetToId()).get();
			
			float nextGcost = currentNodeRecord.costSoFar + connection->GetWeight();
			
			//Check if nextNode is already in visited
			NodeRecord* pRecord{nullptr};
			for (auto record : visited)
			{
				if (record.pNode == pNextNode)
					pRecord = &record;
			}
			
			if (pRecord)
			{				
				if (pRecord->costSoFar < nextGcost)
					continue;
				else
					visited.erase(*pRecord);
			}
			
			//Check if nextNode is already in openList
			pRecord = nullptr;
			for (auto record : openList)
			{
				if (record.pNode == pNextNode)
					pRecord = &record;
			}
			
			if (pRecord)
			{
				if (pRecord->costSoFar < nextGcost)
					continue;
				else
				{
					std::erase(openList, *pRecord);
				}
			}
			
			//Add new NodeRecord to openList
			float Hcost = GetHeuristicCost(pNextNode, pGoalNode);
			
			NodeRecord newRecord{pNextNode, connection, nextGcost, nextGcost + Hcost};
			openList.push_back(newRecord);
			
		}
		
		visited.insert(currentNodeRecord);
		std::erase(openList, currentNodeRecord);
	}
	
	while (currentNodeRecord != startRecord)
	{
		path.push_back(currentNodeRecord.pNode);
		
		int nextNodeId = currentNodeRecord.pConnection->GetFromId();
		
		for (auto record : visited)
		{
			if (record.pNode->GetId() == nextNodeId)
			{
				currentNodeRecord = record;
				break;
			}	
		}
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