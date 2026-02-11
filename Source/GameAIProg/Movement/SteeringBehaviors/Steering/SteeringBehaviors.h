#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// Your own SteeringBehaviors should follow here...
//SEEK
//*********
class Seek : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
};

//FLEE
//*********
class Flee : public Seek
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
};

//ARRIVE
//*********
class Arrive : public Seek
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
	
	void SetSlowRadius(float slowRadius){SlowRadius = slowRadius;}
	void SetTargetRadius(float targetRadius){TargetRadius = targetRadius;}
	
protected:
	float OgMaxSpeed = 0.f; //TODO; Constructor which gets agent's speed
	float SlowRadius = 500.f;
	float TargetRadius = 100.f;
};

//FACE
//*********
class Face : public ISteeringBehavior
{
public:
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) override;
};