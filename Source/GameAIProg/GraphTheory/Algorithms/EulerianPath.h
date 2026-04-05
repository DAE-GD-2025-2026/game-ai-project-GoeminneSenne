#pragma once
#include <stack>

#include "DynamicMesh/DynamicMesh3.h"
#include "Shared/Graph/Graph.h"

namespace GameAI
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	class EulerianPath final
	{
	public:
		EulerianPath(Graph* const pGraph);

		Eulerianity IsEulerian() const;
		std::vector<Node*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(const std::vector<Node*>& pNodes, std::vector<bool>& visited, int startIndex) const;
		bool IsConnected() const;

		bool HasOddDegree(Node* const pNode) const;
		
		Graph* m_pGraph;
	};

	inline EulerianPath::EulerianPath(Graph* const pGraph)
		: m_pGraph(pGraph)
	{
	}

	inline Eulerianity EulerianPath::IsEulerian() const
	{
		if (not IsConnected())
			return Eulerianity::notEulerian;
		
		int NrOfOddDegrees{0};
		for (auto node : m_pGraph->GetActiveNodes())
		{
			if (HasOddDegree(node))
				++NrOfOddDegrees;
		}
		
		if (NrOfOddDegrees > 2)
			return Eulerianity::notEulerian;
		
		if (NrOfOddDegrees == 2)
			return Eulerianity::semiEulerian;
		
		if (NrOfOddDegrees == 0)
			return Eulerianity::eulerian;
		
		return Eulerianity::eulerian;
	}

	inline std::vector<Node*> EulerianPath::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		Graph graphCopy = m_pGraph->Clone();
		std::vector<Node*> Path = {};
		std::vector<Node*> Nodes = graphCopy.GetActiveNodes();
		int startNodeId{ Graphs::InvalidNodeId };
		
		if (eulerianity == Eulerianity::notEulerian)
			return Path;
		
		//Choose Starting Node
		if (eulerianity == Eulerianity::eulerian)
			startNodeId = Nodes.front()->GetId();
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			for (int idx = 0; idx < Nodes.size(); ++idx)
			{
				if (HasOddDegree(Nodes[idx]))
				{
					startNodeId = idx;
					break;
				}
			}
		}
		
		std::stack<int> nodeStack;
		nodeStack.push(startNodeId);
		
		//Add node to stack
		auto currentConnections = graphCopy.FindConnectionsTo(startNodeId);
		
		while (nodeStack.size() >  0)
		{
			int currentNodeId = nodeStack.top();
			currentConnections = graphCopy.FindConnectionsFrom(currentNodeId);
			
			if (currentConnections.size() > 0)
			{
				int nextNodeId = currentConnections.front()->GetToId();
				graphCopy.RemoveConnection(currentConnections.front());
				nodeStack.push(nextNodeId);
			}
			else
			{
				Path.push_back(m_pGraph->GetNode(currentNodeId).get());
				nodeStack.pop();
			}
		}
				
		std::reverse(Path.begin(), Path.end());
				
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex ) const
	{
		//Validate the 
		visited[startIndex] = true;
		
		//Ask the graph for the connections from that node
		auto Connections = m_pGraph->FindConnectionsFrom(startIndex);
		
		for (auto C : Connections)
		{
			const int connectedId = C->GetToId();
			if (Nodes[connectedId]->GetId() >= 0 and not visited[connectedId])
			{
				VisitAllNodesDFS(Nodes, visited, connectedId);
			}
		}
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		constexpr int StartIdx{0};
		
		std::vector<bool> Visited(Nodes.size(), false);
		VisitAllNodesDFS(Nodes, Visited, StartIdx);
		
		for (auto b : Visited)
		{
			if (not b) return false;
		}
		return true;
	}
	
	inline bool EulerianPath::HasOddDegree(Node* const pNode) const
	{
		int InDegree = m_pGraph->FindConnectionsTo(pNode->GetId()).size();
		int OutDegree = m_pGraph->FindConnectionsFrom(pNode->GetId()).size();
			
		int Degree = InDegree + OutDegree;
		if (not m_pGraph->GetIsDirectional())
			Degree /= 2;
		
		return (Degree & 1);
	}
}