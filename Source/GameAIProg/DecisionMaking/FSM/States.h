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
		
		void OnEnter(UBlackboardComponent* Blackboard) override
		{
			auto selfActor = Blackboard->GetValueAsObject("SelfActor");
			if (ASteeringAgent* SteeringAgent = Cast<ASteeringAgent>(selfActor))
			{
				SteeringAgent->SetSteeringBehavior(m_pPathFollow);
			}
		}
		
		virtual void Tick(float DeltaTime, UBlackboardComponent* Blackboard) override {}
		
	private:
		PathFollow* m_pPathFollow{nullptr};
	};
	
	class Chase : public State
	{
	public:
		Chase(Pursuit* pPursuit) : State(), m_pPursuit(pPursuit) {}
		
		void OnEnter(UBlackboardComponent* Blackboard) override
		{
			auto selfActor = Blackboard->GetValueAsObject("SelfActor");
			if (ASteeringAgent* SteeringAgent = Cast<ASteeringAgent>(selfActor))
			{
				SteeringAgent->SetSteeringBehavior(m_pPursuit);
			}
		}
		
		virtual void Tick(float DeltaTime, UBlackboardComponent* Blackboard) override
		{
 			auto targetActor = Blackboard->GetValueAsObject("ChaseTarget");
			if (ASteeringAgent* TargetAgent = Cast<ASteeringAgent>(targetActor))
			{
				m_pPursuit->SetTarget(FTargetData{TargetAgent->GetPosition()});
			}
		}
		
	private:
		Pursuit* m_pPursuit{nullptr};
	};
}
