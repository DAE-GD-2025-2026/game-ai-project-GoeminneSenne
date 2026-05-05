// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_FSM.h"

#include "FSMComponent.h"
#include "States.h"
#include "BehaviorTree/BlackboardAssetProvider.h"
#include "DecisionMaking/GameAIController.h"


// Sets default values
ALevel_FSM::ALevel_FSM()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_FSM::BeginPlay()
{
	Super::BeginPlay();
	
	//Thief Agent & Steering Behavior
	pSeek = std::make_unique<Seek>();
	MouseTarget.Position = FVector2D{800, 650};
	
	Thief = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass,
		FVector{800, 650, 90}, FRotator::ZeroRotator);
	Thief->SetDebugRenderingEnabled(false);
	Thief->SetSteeringBehavior(pSeek.get());
	
	
	//Patrol Agent
	Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{0,0,90}, FRotator::ZeroRotator);
	Agent->SetDebugRenderingEnabled(false);
	
	// Steering Behaviors
	std::vector<FVector2D> path{FVector2D(-200,0), FVector2D(200, 0)};
	pPathFollow = std::make_unique<PathFollow>();
	pPathFollow->SetPath(path);
	pPathFollow->SetRepeating(true);
	
	pPursuit = std::make_unique<Pursuit>();
	
	//std::function<bool()> evalFunc = [&](){return true;};
	std::function<bool(UBlackboardComponent*)> evalFunc = [&](UBlackboardComponent* blackboard)
	{
		FVector2D agentPos{Agent->GetPosition()};
		FVector2D thiefPos{Thief->GetPosition()};
		
		return FVector2D::Distance(agentPos, thiefPos) < 100;
	};
	
	if (AGameAIController* AIController = Cast<AGameAIController>(Agent->GetController()))
	{
		if (UFSMComponent* FSM = Cast<UFSMComponent>(AIController->GetBrainComponent()))
		{
			std::unique_ptr<GameAI::FSM::Patrol> patrol = std::make_unique<GameAI::FSM::Patrol>(pPathFollow.get());
			std::unique_ptr<GameAI::FSM::Chase> chase = std::make_unique<GameAI::FSM::Chase>(pPursuit.get());
			
			GameAI::FSM::Patrol* patrolRaw = patrol.get();
			GameAI::FSM::Chase* chaseRaw = chase.get();
			
			//STATES
			FSM->AddState(std::move(patrol));
			FSM->AddState(std::move(chase));
			
			//TRANSITIONS
			FSM->AddTransition(patrolRaw, chaseRaw, 
				evalFunc);
			
			//BLACKBOARD
			auto Blackboard = AIController->GetBlackboardComponent();
			Blackboard->SetValueAsObject("ChaseTarget", Thief);
			
			AIController->RunFiniteStateMachine();
			
			//Needs to be called after Run so the Blackboard is initialized.
			FSM->SetCurrentState(patrolRaw);
		}
	}
	
}

// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	pSeek->SetTarget(MouseTarget);
}

