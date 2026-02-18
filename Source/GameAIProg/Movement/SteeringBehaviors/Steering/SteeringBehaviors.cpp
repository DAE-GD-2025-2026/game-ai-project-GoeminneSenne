#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
//TODO: Normalize outputs
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
	TargetDir.Normalize();
	FVector Forward = Agent.GetActorForwardVector();
	
	float Angle = FMath::FindDeltaAngleDegrees(
		Forward.Rotation().Yaw,
		FVector(TargetDir, 0.f).Rotation().Yaw
		);
	
	constexpr float RotationSpeed = 2.f;
	Steering.AngularVelocity = Angle * RotationSpeed * DeltaT;
	return Steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
	SteeringOutput SeekSteering = Seek::CalculateSteering(deltaT, Agent);
	
	const float Distance = SeekSteering.LinearVelocity.Length();
	const float MaxAgentSpeed = Agent.GetMaxLinearSpeed();
		
	float ReachTime = Distance / MaxAgentSpeed;
	constexpr float MaxReachTime = 4.f;
	ReachTime = FMath::Clamp(ReachTime, 0.f, MaxReachTime);
	
	const FVector2D PredictedPosition = Target.Position + Target.LinearVelocity * ReachTime;
	
	DrawDebugPoint(Agent.GetWorld(), FVector(PredictedPosition, 0.f), 10, FColor::Red);
	DrawDebugDirectionalArrow(Agent.GetWorld(), FVector(Agent.GetPosition(), 0.f), FVector(PredictedPosition, 0.f), 100, FColor::Red);
	SteeringOutput Steering{};
	Steering.LinearVelocity = PredictedPosition - Agent.GetPosition();
	
	return Steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = Pursuit::CalculateSteering(deltaT, Agent);
	Steering.LinearVelocity *= -1;
	
	return Steering;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	FVector2D CircleCenter = Agent.GetPosition() + FVector2D(Agent.GetActorForwardVector().GetSafeNormal2D()) * m_OffsetDistance;

	DrawDebugPoint(Agent.GetWorld(), FVector(CircleCenter, 0.f), 10, FColor::Red);
	DrawDebugCircle(Agent.GetWorld(), FVector(CircleCenter, 0.f), m_Radius, 16, FColor::Blue,
	false, -1, 0, 0, FVector(0,1, 0), FVector(1,0,0));

	//TODO; MaxAngleChange implementen
	m_WanderAngle = FMath::RandRange(0.f, 2*3.14f);
	FVector2D AnglePosition = CircleCenter;
	AnglePosition.X += m_Radius * FMath::Cos(m_WanderAngle);
	AnglePosition.Y += m_Radius * FMath::Sin(m_WanderAngle);
	
	DrawDebugPoint(Agent.GetWorld(), FVector(AnglePosition, 0.f), 15, FColor::Green);
	
	SetTarget(FTargetData(AnglePosition));
	return Seek::CalculateSteering(DeltaT, Agent);
}
