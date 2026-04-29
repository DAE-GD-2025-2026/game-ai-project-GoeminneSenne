#pragma once
#include "FSM.h"

namespace GameAI::FSM
{
	class TestState : public State
	{
	public:
		virtual void Tick(float DeltaTime, UBlackboardComponent* Blackboard) override
		{
			auto selfActor = Blackboard->GetValueAsObject("SelfActor");
			
			if (ASteeringAgent* SteeringAgent = Cast<ASteeringAgent>(selfActor))
			{
				UE_LOG(LogTemp, Log, TEXT("Test State: SelfActor"));
			}
		}
	};
	
	class Patrol : public State
	{
	public:
		Patrol(PathFollow* pPathFollow) : State(), m_pPathFollow(pPathFollow) {} 
		
		virtual void Tick(float DeltaTime, UBlackboardComponent* Blackboard) override
		{
			auto selfActor = Blackboard->GetValueAsObject("SelfActor");
			if (ASteeringAgent* SteeringAgent = Cast<ASteeringAgent>(selfActor))
			{
				SteeringAgent->SetSteeringBehavior(m_pPathFollow);

			}
		}
		
	private:
		PathFollow* m_pPathFollow;
	};
}
