#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"
#include "Misc/OutputDeviceStdOut.h"


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

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	//Flee from every neighbor (inversely proportional)
	//The closer the neighbor is, the more impact it should have on the output
	SteeringOutput Output{};
	const auto& Neighbors = pFlock->GetNeighbors();
	
	FVector2D OutputVelocity{FVector2D::ZeroVector};
	
	for (int Idx = 0; Idx < pFlock->GetNrOfNeighbors(); ++Idx)
	{
		ASteeringAgent* Neighbor = Neighbors[Idx];

		FVector2D PushForce = pAgent.GetPosition() - Neighbors[Idx]->GetPosition();
		
		//Inverse proportional = normalized / distance -> / distance²
		PushForce /= PushForce.SquaredLength();
		OutputVelocity += PushForce;
	}
	
	OutputVelocity.Normalize();
	Output.LinearVelocity = OutputVelocity;
	return Output;
	
}

//*************************
//ALIGNMENT (FLOCKING)
SteeringOutput Alignment::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput Output{};
	Output.LinearVelocity = pFlock->GetAverageNeighborVelocity();
	
	return Output;
}




