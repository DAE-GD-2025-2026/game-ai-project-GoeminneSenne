// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_FSM.h"

#include "FSMComponent.h"
#include "States.h"
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
	
	Agent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, 
	FVector{0,0,90}, FRotator::ZeroRotator);
	Agent->SetDebugRenderingEnabled(false);
	
	//Steering Behaviors
	std::vector<FVector2D> path{FVector2D(-200,0), FVector2D(200, 0)};
	pPathFollow = std::make_unique<PathFollow>();
	pPathFollow->SetPath(path);
	pPathFollow->SetRepeating(true);
	
	
	if (AGameAIController* AIController = Cast<AGameAIController>(Agent->GetController()))
	{
		if (UFSMComponent* FSM = Cast<UFSMComponent>(AIController->GetBrainComponent()))
		{
			std::unique_ptr<GameAI::FSM::Patrol> patrol = std::make_unique<GameAI::FSM::Patrol>(pPathFollow.get());
			
			FSM->SetCurrentState(patrol.get());
			FSM->AddState(std::move(patrol));
			
			AIController->RunFiniteStateMachine();
			
			//BLACKBOARD
			auto Blackboard = AIController->GetBlackboardComponent();
			
			Blackboard->SetValueAsString("Test", "Hello");
			//Blackboard->SetValueAsObject("PathFollowBehavior", pPathFollow.get());
		}
	}
	
}

// Called every frame
void ALevel_FSM::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

