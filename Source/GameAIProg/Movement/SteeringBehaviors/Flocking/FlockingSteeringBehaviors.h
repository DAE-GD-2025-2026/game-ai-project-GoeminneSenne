#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek
{
public:
	Cohesion(Flock* const pFlock) :pFlock(pFlock) {};

	//Cohesion Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Seperation final : public Flee
{
public:
	Seperation(Flock* const pFlock) : pFlock(pFlock) {};
	
	virtual CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;
	
private:
	Flock* pFlock = nullptr;
};

//VELOCITY MATCH - FLOCKING
//************************
