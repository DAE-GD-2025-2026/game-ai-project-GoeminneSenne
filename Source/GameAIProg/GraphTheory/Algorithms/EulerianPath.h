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
		
		// TODO Count nodes with odd degree 
		int NrOfOddDegrees{0};
		for (auto node : m_pGraph->GetActiveNodes())
		{
			int InDegree = m_pGraph->FindConnectionsTo(node->GetId()).size();
			int OutDegree = m_pGraph->FindConnectionsFrom(node->GetId()).size();
			
			int Degree = InDegree + OutDegree;
			if (m_pGraph->GetIsDirectional())
				Degree /= 2;
			
			if (Degree & 1)
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
		int currentNodeId{ Graphs::InvalidNodeId };
		
		// TODO Check if there can be an Euler path
		// TODO If this graph is not eulerian, return the empty path
		if (eulerianity == Eulerianity::notEulerian)
			return Path;
		
		//Choose Starting Node
		if (eulerianity == Eulerianity::eulerian)
			currentNodeId = Nodes.front()->GetId();
		else if (eulerianity == Eulerianity::semiEulerian)
		{
			
		}
		
		// TODO Start algorithm loop
		std::stack<int> nodeStack;

		std::reverse(Path.begin(), Path.end());
		return Path;
	}

	inline void EulerianPath::VisitAllNodesDFS(const std::vector<Node*>& Nodes, std::vector<bool>& visited, int startIndex ) const
	{
		//Validate the 
		visited[startIndex] = true;
		
		//Ask the graph for the connections from that node
		auto Connections = m_pGraph->FindConnectionsFrom(startIndex);
		
		// TODO recursively visit any valid connected nodes that were not visited before
		for (auto C : Connections)
		{
			const int connectedId = C->GetToId();
			if (Nodes[connectedId]->GetId() >= 0 and not visited[connectedId])
			{
				VisitAllNodesDFS(Nodes, visited, connectedId);
			}
		}
		
		// TODO Tip: use an index-based for-loop to find the correct index
	}

	inline bool EulerianPath::IsConnected() const
	{
		std::vector<Node*> Nodes = m_pGraph->GetActiveNodes();
		if (Nodes.size() == 0)
			return false;

		//TODO vervangen door GetFirstInvalidNodeIdx
		const int StartIdx{0};
		
		std::vector<bool> Visited(Nodes.size(), false);
		VisitAllNodesDFS(Nodes, Visited, StartIdx);
		
		// TODO if a node was never visited, this graph is not connected
		for (auto b : Visited)
		{
			if (not b) return false;
		}
		return true;
	}
}