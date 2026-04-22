#pragma once
#include <functional>
#include <memory>

#include "BehaviorTree/BlackboardComponent.h"

namespace GameAI::FSM
{
	class State
	{
	public:
		virtual ~State() = default;
		
		virtual void Tick(float DeltaTime, UBlackboardComponent* Blackboard) = 0;
	};
	
	class Transition final
	{
	public:
		explicit Transition(State* From, State* To, std::function<bool()> EvalFunc);
		
		bool Evaluate() const;
		
	private:
		State* From = nullptr;
		State* To = nullptr;
		std::function<bool()> EvalFunc = nullptr;
	};
	
	class FSM final
	{
	public:
		~FSM() = default;
		
		void SetCurrentState(State* newState);
		void Tick(float DeltaTime);
		
		void SetBlackboard(UBlackboardComponent* pBlackboard);
		void AddState(std::unique_ptr<State>&& NewState);
		void AddTransition(State* From, State* To, std::function<bool()> EvalFunc);
		
	private:
		State* CurrentState = nullptr;
		UBlackboardComponent* Blackboard = nullptr;
		
		std::vector<std::unique_ptr<State>> States;
		std::vector<std::unique_ptr<Transition>> Transitions;
	};
}
