#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "BehaviorTree/BlackboardComponent.h"

namespace GAMEAI::FSM
{
	class State
	{
		virtual ~State() = default;
	};
	
	class Transition
	{
	public:
		
	private:
		State* FromState{};
		State* ToState{};
		std::function<bool()> EvalFunc{};
	};
	
	class FSM
	{
	public:
		void SetBlackboard(UBlackboardComponent* Blackboard);
		
	private:
		State* CurrentState{};
		UBlackboardComponent* Blackboard{};
		
		std::vector<std::unique_ptr<State>> States;
		std::vector<std::unique_ptr<Transition>> Transitions;
	};
}
