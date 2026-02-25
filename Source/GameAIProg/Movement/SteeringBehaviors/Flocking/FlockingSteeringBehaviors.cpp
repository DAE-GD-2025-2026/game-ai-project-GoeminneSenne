#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	FVector2D TargetPos = pFlock->GetAverageNeighborPos();
	
	FTargetData NewTarget{TargetPos};
	SetTarget(NewTarget);
	SteeringOutput Output = Seek::CalculateSteering(deltaT, pAgent);
	
	return Output;
}

SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	//Flee from every neighbor (inversely proportional)
	//The closer the neighbor is, the more impact it should have on the output
	SteeringOutput Output{};
	const auto& Neighbors = pFlock->GetNeighbors();
	
	for (int Idx = 0; Idx < pFlock->GetNrOfNeighbors(); ++Idx)
	{
		ASteeringAgent* Neighbor = Neighbors[Idx];
		
		FTargetData NewTarget{Neighbor->GetPosition(), Neighbor->GetRotation(),
			Neighbor->GetLinearVelocity(), Neighbor->GetAngularVelocity()};
		
		SetTarget(NewTarget);
		
		float invDistance = 1.f / FVector2D::Distance(Neighbor->GetPosition(), pAgent.GetPosition());
		Output += Flee::CalculateSteering(deltaT, pAgent) * invDistance;
	}
	
	return Flee::CalculateSteering(deltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
