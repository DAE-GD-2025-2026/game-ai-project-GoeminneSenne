#include "SpacePartitioning.h"

// --- Cell ---
// ------------
Cell::Cell(float Left, float Bottom, float Width, float Height)
{
	BoundingBox.Min = { Left, Bottom };
	BoundingBox.Max = { BoundingBox.Min.X + Width, BoundingBox.Min.Y + Height };
}

std::vector<FVector2D> Cell::GetRectPoints() const
{
	const float left = BoundingBox.Min.X;
	const float bottom = BoundingBox.Min.Y;
	const float width = BoundingBox.Max.X - BoundingBox.Min.X;
	const float height = BoundingBox.Max.Y - BoundingBox.Min.Y;

	std::vector<FVector2D> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(UWorld* pWorld, float Width, float Height, int Rows, int Cols, int MaxEntities)
	: pWorld{pWorld}
	, SpaceWidth{Width}
	, SpaceHeight{Height}
	, NrOfRows{Rows}
	, NrOfCols{Cols}
	, NrOfNeighbors{0}
{
	Neighbors.SetNum(MaxEntities);
	
	SpaceBottomLeft = CellOrigin - FVector2D(Width / 2.f, Height / 2.f);
	
	//calculate bounds of a cell
	CellWidth = Width / Cols;
	CellHeight = Height / Rows;

	Cells.reserve(Rows * Cols);
	for (int Row = 0; Row < Rows; ++Row)
	{
		for (int Col = 0; Col < Cols; ++Col)
		{			
			Cells.emplace_back(
				SpaceBottomLeft.X + CellWidth * Row, 
				SpaceBottomLeft.Y + CellHeight * Col,
				CellWidth, CellHeight);
		}
	}
}

void CellSpace::AddAgent(ASteeringAgent& Agent)
{
	int CellIndex = PositionToIndex(Agent.GetPosition());
	Cells[CellIndex].Agents.push_back(&Agent);
}

void CellSpace::UpdateAgentCell(ASteeringAgent& Agent, const FVector2D& OldPos)
{
	int NewCellIndex = PositionToIndex(Agent.GetPosition());
	int OldCellIndex = PositionToIndex(OldPos);
	
	if (NewCellIndex == OldCellIndex) return;
	
	if (NewCellIndex > 0 && NewCellIndex < Cells.size())
	{
		Cells[NewCellIndex].Agents.push_back(&Agent);
	}
	if (OldCellIndex > 0 && OldCellIndex < Cells.size())
	{
		Cells[OldCellIndex].Agents.remove(&Agent);
	}
}

void CellSpace::RegisterNeighbors(ASteeringAgent& Agent, float QueryRadius)
{
	NrOfNeighbors = 0;
	
	FVector2D Min = Agent.GetPosition() - FVector2D(QueryRadius, QueryRadius);
	FVector2D Max = Agent.GetPosition() + FVector2D(QueryRadius, QueryRadius);
	
	FRect BoundingBox{Min, Max}; 
	
	for (const Cell& cell : Cells)
	{
		if (not DoRectsOverlap(BoundingBox, cell.BoundingBox)) continue;
	
		for (const auto& Other : cell.Agents)
		{
			if (Other == &Agent) continue;
			
			const float DistanceSqr = FVector2D::DistSquared(Agent.GetPosition(), Other->GetPosition());
			if (DistanceSqr <= QueryRadius * QueryRadius)
			{
				Neighbors[NrOfNeighbors] = Other;
				++NrOfNeighbors;
				
				//Stop checking neighbors if memory pool is full
				if (NrOfNeighbors >= Neighbors.Num()) return;
			}
		}
	}
	
}

void CellSpace::EmptyCells()
{
	for (Cell& c : Cells)
		c.Agents.clear();
}

void CellSpace::RenderCells(bool shouldRenderNrofAgents) const
{
	std::vector<FVector2D> RectPoints(4);
	
	for (const Cell& cell : Cells)
	{
		RectPoints = std::move(cell.GetRectPoints());
		
		FVector2D CellCenter{RectPoints[0]};
		CellCenter.X += CellWidth / 2.f;
		CellCenter.Y += CellHeight / 2.f;
		
		if (shouldRenderNrofAgents)
		{
			DrawDebugString(pWorld, FVector(CellCenter, 1.f), FString::FromInt(cell.Agents.size()), 
				nullptr, FColor::Purple, 0.f, false, 2.f);
		}
		
		for (int idx{}; idx < 4; ++idx)
		{
			DrawDebugLine(pWorld, FVector(RectPoints[idx],0), FVector(RectPoints[(idx+1) % 4], 0), FColor::Purple);
		}
	}
}

int CellSpace::PositionToIndex(FVector2D const & Pos) const
{
	int IndexX = FMath::FloorToInt((Pos.X - SpaceBottomLeft.X) / CellWidth);
	int IndexY = FMath::FloorToInt((Pos.Y - SpaceBottomLeft.Y) / CellHeight);
	
	if (IndexX < 0 || IndexX >= NrOfCols || IndexY < 0 || IndexY >= NrOfRows)
	{
		return -1;
	}
	
	return IndexX * NrOfCols + IndexY;
}

bool CellSpace::DoRectsOverlap(FRect const & RectA, FRect const & RectB)
{
	// Check if the rectangles are separated on either axis
	if (RectA.Max.X < RectB.Min.X || RectA.Min.X > RectB.Max.X) return false;
	if (RectA.Max.Y < RectB.Min.Y || RectA.Min.Y > RectB.Max.Y) return false;
    
	// If they are not separated, they must overlap
	return true;
}