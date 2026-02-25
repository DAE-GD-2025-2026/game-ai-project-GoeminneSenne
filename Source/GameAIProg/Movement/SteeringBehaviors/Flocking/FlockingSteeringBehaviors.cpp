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

//*********************
//SEPARATION (FLOCKING)

//*************************
//VELOCITY MATCH (FLOCKING)
