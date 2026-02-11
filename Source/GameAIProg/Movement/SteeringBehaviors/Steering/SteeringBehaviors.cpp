#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
// TODO: Do the Week01 assignment :^)
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{};
	Steering.LinearVelocity = Target.Position - Agent.GetPosition();
	
	return Steering;
}

SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering{Seek::CalculateSteering(DeltaT, Agent)};
	Steering.LinearVelocity *= -1;
	
	return Steering;
}


SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	if (!OgMaxSpeed)
		OgMaxSpeed = Agent.GetMaxLinearSpeed();
	
	SteeringOutput Steering{Seek::CalculateSteering(DeltaT, Agent)};
	
	if (Steering.LinearVelocity.Length() <= TargetRadius)
	{
		Steering.LinearVelocity = FVector2D(0.f, 0.f);
	}
	else if (Steering.LinearVelocity.Length() <= SlowRadius)
	{
		//Gradually Decrease speed
		const float ratio = Steering.LinearVelocity.Length() / SlowRadius;
		Agent.SetMaxLinearSpeed(ratio * OgMaxSpeed);
	}
	else
	{
		Agent.SetMaxLinearSpeed(OgMaxSpeed);
	}

	//DrawDebugLines
	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.f), SlowRadius, 16, FColor::Blue,
			false, -1, 0, 0, FVector(0,1, 0), FVector(1,0,0));
		DrawDebugCircle(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.f), TargetRadius, 16, FColor::Orange,
			false, -1, 0, 0, FVector(0,1, 0), FVector(1,0,0));
	}
	
	return Steering;
}

SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	Agent.SetIsAutoOrienting(false);
	
	SteeringOutput Steering{};
	
	FVector2D TargetDir = Target.Position - Agent.GetPosition();
	float Dot = FVector::DotProduct(FVector(TargetDir, 0.f).GetSafeNormal(), Agent.GetActorForwardVector());
	float Angle = FMath::RadiansToDegrees(FMath::Acos(Dot));
	
	Steering.AngularVelocity = Angle;
	return Steering;
}
