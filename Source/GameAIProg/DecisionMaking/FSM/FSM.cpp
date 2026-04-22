#include "FSM.h"

//Transition
///////////////////////////////
GameAI::FSM::Transition::Transition(State* From, State* To, std::function<bool()> EvalFunc)
	: From(From), To(To), EvalFunc(EvalFunc)
{
}

bool GameAI::FSM::Transition::Evaluate() const
{
	return EvalFunc();
}


//FSM
////////////////////
void GameAI::FSM::FSM::SetCurrentState(State* newState)
{
	CurrentState = newState;
}

void GameAI::FSM::FSM::Tick(float DeltaTime)
{
	CurrentState->Tick(DeltaTime, Blackboard);
}

void GameAI::FSM::FSM::SetBlackboard(UBlackboardComponent* pBlackboard)
{
	Blackboard = pBlackboard;
}

void GameAI::FSM::FSM::AddState(std::unique_ptr<State>&& NewState)
{
	States.emplace_back(std::move(NewState));
}

void GameAI::FSM::FSM::AddTransition(State* From, State* To, std::function<bool()> EvalFunc)
{
	Transitions.emplace_back(std::make_unique<Transition>(From, To, EvalFunc));
}
