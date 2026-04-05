#include "NavGraph.h"

#include "NavGraphNode.h"

GameAI::NavGraph::NavGraph(std::unique_ptr<TriPolygon> && NavPoly)
	: Graph{false}
	, pNavPoly{std::move(NavPoly)}
{
	CreateNavigationGraph();
}

GameAI::NavGraph::NavGraph(const NavGraph& Other)
	: Graph(false)
{
	Nodes.reserve(Other.Nodes.size());
	for (std::unique_ptr<Node> const & OtherNode : Other.Nodes)
	{
		Nodes.push_back(std::make_unique<NavGraphNode>(*dynamic_cast<NavGraphNode*>(OtherNode.get())));
	}
        
	Connections.reserve(Other.Connections.size());
	for (std::unique_ptr<Connection> const & OtherConnection : Other.Connections)
	{
		Connections.push_back(std::make_unique<Connection>(*OtherConnection.get()));
	}
}

std::unique_ptr<GameAI::NavGraph> GameAI::NavGraph::Clone() const
{
	return std::make_unique<NavGraph>(*this);
}

int GameAI::NavGraph::GetNodeIdFromEdgeIndex(int EdgeIdx) const
{
	if (EdgeIdx >= 0)
	{
		for (auto const & pNode : Nodes)
		{
			if (reinterpret_cast<NavGraphNode*>(pNode.get())->GetEdgeIdx() == EdgeIdx)
			{
				return pNode->GetId();
			}
		}
	}
	
	return Graphs::InvalidNodeId;
}

void GameAI::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigation mesh and create nodes
	for (const auto& edge : pNavPoly->GetEdges())
	{
		int triangleConnections = 0;
		
		for (const auto& triangle : pNavPoly->GetTriangles())
		{
			if (triangle.HasEdge(edge))
			{
				triangleConnections++;
				if (triangleConnections > 1)
				{
					//Add NavGraphNode
					FVector edgeMiddle = edge.GetP1(*pNavPoly.get()) + edge.GetP2(*pNavPoly.get());
					edgeMiddle /= 2.f;
					
					int index = pNavPoly->FindEdgeIndex(edge).value();
					
					AddNode(std::make_unique<NavGraphNode>(FVector2D(edgeMiddle.X, edgeMiddle.Y), index));
					break;
				}
			}
		}
	}
	
	//2. Create connections now that every node is created	
		//2 valid nodes -> 1 connection
		//3 valid nodes -> 3 connections
	std::vector<int> nodeIds(3);
	
	for (const auto& triangle : pNavPoly->GetTriangles())
	{
		nodeIds.clear();
		
		//Loop over edge indexes
		for (const auto& edge : triangle.GetEdges())
		{
			auto edgeIndex = pNavPoly->FindEdgeIndex(edge);
			
			if (edgeIndex.has_value())
			{
				auto nodeId = GetNodeIdFromEdgeIndex(edgeIndex.value());
				if (nodeId == Graphs::InvalidNodeId) continue;
				
				nodeIds.push_back(nodeId);
			}
		}
		
		if (nodeIds.size() == 2)
		{
			AddConnection(nodeIds[0], nodeIds[1]);
		}
		else if (nodeIds.size() == 3)
		{
			AddConnection(nodeIds[0], nodeIds[1]);
			AddConnection(nodeIds[1], nodeIds[2]);
			AddConnection(nodeIds[0], nodeIds[2]);
		}
	}
	
	//3. Set the connections cost to the actual distance
	for (auto& connection : Connections)
	{
		const Node* fromNode = GetNode(connection->GetFromId()).get();
		const Node* toNode = GetNode(connection->GetToId()).get();
		const float distance = FVector2D::Distance(fromNode->GetPosition(), toNode->GetPosition());
		
		connection->SetWeight(distance);
	}
}
