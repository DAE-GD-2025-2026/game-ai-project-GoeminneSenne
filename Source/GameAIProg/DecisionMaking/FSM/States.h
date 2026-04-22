#pragma once
#include "FSM.h"

namespace GameAI::FSM
{
	class TestState : public State
	{
	public:
		virtual void Tick(float DeltaTime, UBlackboardComponent* Blackboard) override
		{
			UE_LOG(LogTemp, Warning, TEXT("TestState Called"));
		}
	private:
	};
}
