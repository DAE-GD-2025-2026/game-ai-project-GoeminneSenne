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
		
		virtual void OnEnter(UBlackboardComponent* pBlackboard) {};
		virtual void Tick(float DeltaTime, UBlackboardComponent* Blackboard) = 0;
	};
	
	class Transition final
	{
	public:
		explicit Transition(State* From, State* To, std::function<bool(UBlackboardComponent*)> EvalFunc);
		
		bool Evaluate(UBlackboardComponent* blackboard) const;
		State* GetFromState() const;
		State* GetToState() const;
		
	private:
		State* From = nullptr;
		State* To = nullptr;
		std::function<bool(UBlackboardComponent*)> EvalFunc = nullptr;
	};
	
	class FSM final
	{
	public:
		~FSM() = default;
		
		void SetCurrentState(State* newState);
		void Tick(float DeltaTime);
		
		void SetBlackboard(UBlackboardComponent* pBlackboard);
		void AddState(std::unique_ptr<State>&& NewState);
		void AddTransition(State* From, State* To, std::function<bool(UBlackboardComponent*)> EvalFunc);
		
	private:
		State* CurrentState = nullptr;
		UBlackboardComponent* Blackboard = nullptr;
		
		std::vector<std::unique_ptr<State>> States;
		std::vector<std::unique_ptr<Transition>> Transitions;
	};
}
