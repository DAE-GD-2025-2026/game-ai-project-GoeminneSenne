// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Movement/SteeringBehaviors/PathFollow/PathFollowSteeringBehavior.h"
#include "Shared/Level_Base.h"
#include "Shared/Graph/GraphEditorComponent.h"

#include "Level_MST.generated.h"

namespace GameAI
{
	class EulerianPath;
}

/**
 * 
 */
UCLASS()
class GAMEAIPROG_API ALevel_MST : public ALevel_Base
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GraphEditor")
	TSubclassOf<UGraphEditorComponent> GraphEditorClass;

	// Sets default values for this actor's properties
	ALevel_MST();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

private:
	GameAI::Graph Graph{false};
	GameAI::GraphRenderer Renderer{nullptr};
	GameAI::GraphNodeFactory<GameAI::Node> NodeFactory{};
	
	
	UPROPERTY()
	UGraphEditorComponent* PlayerGraphEditor{}; // ref
	
	void CalculateMST();
};
