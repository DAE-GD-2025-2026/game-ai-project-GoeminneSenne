#include "NavGraphPathfinding.h"

#include "AStar.h"
#include "PathSmoothing.h"
#include "VectorTypes.h"
#include "Shared/Graph/NavGraph/NavGraph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

using namespace GameAI;

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos,
	NavGraph* const pNavGraph, std::vector<FVector2D>& debugNodePositions, std::vector<NavLine>& debugPortals) 
{
	//Create the path to return
	std::vector<FVector2D> finalPath{};

	//Get the start and endTriangle
	auto NavMesh = pNavGraph->GetNavPolygon();
	
	auto StartTriangle = NavMesh->GetTriangleAtPosition(startPos, true);
	auto EndTriangle = NavMesh->GetTriangleAtPosition(endPos, true);

	if (not StartTriangle or not EndTriangle) return finalPath;
	
	if (StartTriangle == EndTriangle)
	{
		finalPath.push_back(startPos);
		finalPath.push_back(endPos);
		return finalPath;
	}
	
	//We have valid start/end triangles and they are not the same
	//=> Start looking for a path
	//Copy the graph
	auto GraphCopy = pNavGraph->Clone();
	
	//Create Extra node for the Start Node (Agent's position
	int startIndex = GraphCopy->AddNode(std::make_unique<NavGraphNode>(startPos, -1));
	
	for (const auto& edge : StartTriangle->GetEdges())
	{
		auto edgeIndex = NavMesh->FindEdgeIndex(edge);
		if (edgeIndex.has_value())
		{
			auto edgeNodeId = GraphCopy->GetNodeIdFromEdgeIndex(edgeIndex.value());
			if (edgeNodeId == Graphs::InvalidNodeId) continue;
			
			auto connection = std::make_unique<Connection>(startIndex, edgeNodeId);
			float distance = FVector2D::Distance(GraphCopy->GetNode(startIndex)->GetPosition(), GraphCopy->GetNode(edgeNodeId)->GetPosition());
			connection->SetWeight(distance);
			
			GraphCopy->AddConnection(std::move(connection));
		}
	}
	
	//Create extra node for the endNode
	int endIndex = GraphCopy->AddNode(std::make_unique<NavGraphNode>(endPos, -1));
	
	for (const auto& edge : EndTriangle->GetEdges())
	{
		auto edgeIndex = NavMesh->FindEdgeIndex(edge);
		if (edgeIndex.has_value())
		{
			int edgeNodeId = GraphCopy->GetNodeIdFromEdgeIndex(edgeIndex.value());
			if (edgeNodeId == Graphs::InvalidNodeId) continue;
			
			auto connection = std::make_unique<Connection>(endIndex, edgeNodeId);
			float distance = FVector2D::Distance(GraphCopy->GetNode(endIndex)->GetPosition(), GraphCopy->GetNode(edgeNodeId)->GetPosition());
			connection->SetWeight(distance);
			
			GraphCopy->AddConnection(std::move(connection));
		}
	}
	
	//Run A star on new graph
	auto pathfinder = std::make_unique<AStar>(GraphCopy.get(), HeuristicFunctions::Chebyshev);
	auto foundPath = pathfinder->FindPath(GraphCopy->GetNode(startIndex).get(), GraphCopy->GetNode(endIndex).get());
	
	
	for (const auto& node : foundPath)
	{
		finalPath.push_back(node->GetPosition());
	}
	
	//Debug Visualisation
	debugNodePositions = finalPath;
	
	// Extra: Run optimiser on new graph (First check if everything works without SSFA!)
	debugPortals = SSFA::FindPortals(foundPath, *pNavGraph->GetNavPolygon());
	// finalPath = SSFA::OptimizePortals(debugPortals, *pNavGraph->GetNavPolygon());
	
	
	return finalPath;
}

std::vector<FVector2D> NavMeshPathfinding::FindPath(const FVector2D& startPos, const FVector2D& endPos, NavGraph* const pNavGraph)
{
	std::vector<FVector2D> debugNodePositions{};
	std::vector<NavLine> debugPortals{};

	return FindPath(startPos, endPos, pNavGraph, debugNodePositions, debugPortals);
}