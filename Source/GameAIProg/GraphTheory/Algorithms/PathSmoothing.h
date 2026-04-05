#pragma once
#include <vector>

#include "NavGraphPathfinding.h"
#include "Movement/Pathfinding/Navmesh/TriPolygon.h"
#include "Shared/Graph/Graph.h"
#include "Shared/Graph/NavGraph/NavGraphNode.h"

namespace GameAI
{
	class SSFA final
{
public:
	//=== SSFA Functions ===
	//--- References ---
	//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
	//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
	static std::vector<NavLine> FindPortals(std::vector<Node*> const & Path, TriPolygon const & NavPoly)
	{
		//Container
		std::vector<NavLine> Portals{};
		Portals.emplace_back(Path.front()->GetPosition(), Path.front()->GetPosition());
		
		//For each node received, get it's corresponding line
		for (int idx{1}; idx < Path.size() - 1; ++idx)
		{
			const auto& node = Path[idx];
			const auto& previousNode = Path[idx - 1];
			FVector2D direction = node->GetPosition() - previousNode->GetPosition();
			
			int EdgeIndex = static_cast<NavGraphNode*>(node)->GetEdgeIdx();
			TriPolygon::Edge Edge = NavPoly.GetEdges()[EdgeIndex];
					
			FVector P1 = Edge.GetP1(NavPoly);
			FVector P2 = Edge.GetP2(NavPoly);
			
			NavLine Line{{P1.X, P1.Y}, {P2.X, P2.Y}};
			FVector2D LineDirection = Line.P1 - Line.P2;
					
			//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
			float cross = FVector2D::CrossProduct(direction, LineDirection);
			if (cross < 0.f)
			{
				std::swap(Line.P1, Line.P2);
			}
				
			//Store portal
			Portals.emplace_back(Line);
		}
		
		//Add degenerate portal to force end evaluation
		Portals.emplace_back(Path.back()->GetPosition(), Path.back()->GetPosition());
		
		return Portals;
	}

	static std::vector<FVector2D> OptimizePortals( std::vector<NavLine> const & Portals, TriPolygon const & NavPoly)
	{
		std::vector<FVector2D> Path{};
		
		FVector2D ApexPoint{Portals.front().P1};
		//P1 == right point of portal, P2 == left point of portal
		FVector2D LeftLeg = Portals[0].P2 - ApexPoint;
		FVector2D RightLeg = Portals[0].P1 - ApexPoint;
		int LeftLegIndex = 1;
		int RightLegIndex = 1;
		
		Path.push_back(ApexPoint);
		UE_LOG(LogTemp, Warning, TEXT("Starting OptimizePortals"));
		
		for (int PortalIndex{0}; PortalIndex < Portals.size();)
		{
			const NavLine& Portal = Portals[PortalIndex];
			
			FVector2D NewRightLeg = Portal.P1 - ApexPoint;
			
			//--- RIGHT CHECK ---
			//1. See if moving funnel inwards - RIGHT
			if (FVector2D::CrossProduct(RightLeg, NewRightLeg) <= 0.f)
			{
				//2. See if new line degenerates a line segment - RIGHT
				if (FVector2D::CrossProduct(LeftLeg, NewRightLeg) < 0.f)
				{
					//Leftleg becomes new apex point
					
					ApexPoint = Portals[LeftLegIndex].P2;
					Path.push_back(ApexPoint);
					
					PortalIndex = LeftLegIndex + 1;
					LeftLegIndex = PortalIndex;
					RightLegIndex = PortalIndex;
					
					//Calculate new legs (if not the end)
					if (PortalIndex < Portals.size())
					{
						RightLeg = Portals[RightLegIndex].P1 - ApexPoint;
						LeftLeg = Portals[LeftLegIndex].P2 - ApexPoint;
					}
						
					continue;
				}
				else
				{
					RightLeg = NewRightLeg;
					RightLegIndex = PortalIndex;
				}
			}
			
			FVector2D NewLeftLeg = Portal.P2 - ApexPoint;
			
			//--- LEFT CHECK ---
			//1. See if moving funnel inwards - LEFT
			if (FVector2D::CrossProduct(LeftLeg, NewLeftLeg) >= 0.f)
			{
				//2. See if new line degenerates a line segment - LEFT
				if (FVector2D::CrossProduct(RightLeg, NewLeftLeg) > 0.f)
				{
					//Rightleg becomes new apex point
					ApexPoint = Portals[RightLegIndex].P1;
					Path.push_back(ApexPoint);
					
					PortalIndex = RightLegIndex + 1;
					LeftLegIndex = PortalIndex;
					RightLegIndex = PortalIndex;
					
					//Calculate new legs (if not the end)
					if (PortalIndex < Portals.size())
					{
						RightLeg = Portals[RightLegIndex].P1 - ApexPoint;
						LeftLeg = Portals[LeftLegIndex].P2 - ApexPoint;
					}
					continue;
				}
				else
				{
					LeftLeg = NewLeftLeg;
					LeftLegIndex = PortalIndex;
				}
			}
			
			++PortalIndex;
		}
		
		// Add last path point
		Path.push_back(Portals.back().P1);

		return Path;
	}
private:
	SSFA() {};
	~SSFA() {};
};
}
