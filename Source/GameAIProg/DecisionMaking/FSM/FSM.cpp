#include "FSM.h"

//Transition
///////////////////////////////
GameAI::FSM::Transition::Transition(State* From, State* To, std::function<bool(UBlackboardComponent*)> EvalFunc)
	: From(From), To(To), EvalFunc(EvalFunc)
{
}

bool GameAI::FSM::Transition::Evaluate(UBlackboardComponent* blackboard) const
{
	return EvalFunc(blackboard);
}

GameAI::FSM::State* GameAI::FSM::Transition::GetFromState() const
{
	return From;
}

GameAI::FSM::State* GameAI::FSM::Transition::GetToState() const
{
	return To;
}


//FSM
////////////////////
void GameAI::FSM::FSM::SetCurrentState(State* newState)
{
	CurrentState = newState;
	CurrentState->OnEnter(Blackboard);
}

void GameAI::FSM::FSM::Tick(float DeltaTime)
{
	if (!CurrentState) return;
	
	CurrentState->Tick(DeltaTime, Blackboard);
	
	for (const auto& Transition : Transitions)
	{
		if (Transition->GetFromState() == CurrentState)
		{
			if (Transition->Evaluate(Blackboard))
			{
				SetCurrentState(Transition->GetToState());
				break;
			}
		}
	}
	
}

void GameAI::FSM::FSM::SetBlackboard(UBlackboardComponent* pBlackboard)
{
	Blackboard = pBlackboard;
}

void GameAI::FSM::FSM::AddState(std::unique_ptr<State>&& NewState)
{
	States.emplace_back(std::move(NewState));
}

void GameAI::FSM::FSM::AddTransition(State* From, State* To, std::function<bool(UBlackboardComponent*)> EvalFunc)
{
	Transitions.emplace_back(std::make_unique<Transition>(From, To, EvalFunc));
}
