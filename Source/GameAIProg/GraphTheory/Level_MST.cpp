// Fill out your copyright notice in the Description page of Project Settings.


#include "GraphTheory/Level_MST.h"

#include "Algorithms/EulerianPath.h"
#include "Algorithms/PrimMST.h"
#include "Shared/GameAISpectator.h"

using namespace GameAI;

// Sets default values
ALevel_MST::ALevel_MST()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_MST::BeginPlay()
{
	Super::BeginPlay();
	
	// Add the graph editor to our player
	if (PlayerController = Cast<APlayerController>(GetWorld()->GetFirstLocalPlayerFromController()->PlayerController); 
		GraphEditorClass && PlayerController)
	{
		PlayerGraphEditor = NewObject<UGraphEditorComponent>(PlayerController->GetPawn(), GraphEditorClass);
		PlayerGraphEditor->RegisterComponent();
		PlayerGraphEditor->SetEditedGraph(&Graph);
		PlayerGraphEditor->SetNodeFactory(&NodeFactory);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to get PlayerController from LocalPlayer or GraphEditorClass is null"))
		return;
	}
	
	// Make the view orthogonal for less perspective issues
	if (AGameAISpectator* Player = Cast<AGameAISpectator>(PlayerController->GetPawnOrSpectator()); Player)
	{
		Player->SetCameraProjection(ECameraProjectionMode::Orthographic);
	}
	
	Renderer = GraphRenderer{GetWorld()};
	
	auto NodeId1 = Graph.AddNode(std::make_unique<Node>(FVector2D{0.f, 0.f}));
	auto NodeId2 = Graph.AddNode(std::make_unique<Node>(FVector2D{100.f, 100.f}));
	auto Conn = std::make_unique<Connection>(NodeId1, NodeId2);
	Conn->SetWeight(FVector2D::Distance(FVector2D{0.f, 0.f}, FVector2D{100.f, 100.f}));
	Graph.AddConnection(std::move(Conn));
	
	/*
	auto NodeId2 = Graph.AddNode(std::make_unique<Node>(FVector2D{100.f, 100.f}));
	auto NodeId3 = Graph.AddNode(std::make_unique<Node>(FVector2D{50.f, -200.f}));
	auto NodeId4 = Graph.AddNode(std::make_unique<Node>(FVector2D{250.f, 300.f}));
	Graph.AddConnection(NodeId1, NodeId2);
	Graph.AddConnection(NodeId2, NodeId3);
	Graph.AddConnection(NodeId1, NodeId3);
	Graph.AddConnection(NodeId2, NodeId4);
	*/

}

void ALevel_MST::BeginDestroy()
{
	Super::BeginDestroy();
}

void ALevel_MST::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Graph Theory");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Spacing();
		if (ImGui::Button("Calculate MST"))
		{
			CalculateMST();
		}
		
		//End
		ImGui::End();
	}
#pragma endregion UI
	
	Renderer.RenderGraph(Graph);
}



void ALevel_MST::CalculateMST()
{
	for (const auto& conn : Graph.GetConnections())
	{
		UE_LOG(LogTemp, Log, TEXT("Connection from %d to %d"), conn->GetFromId(), conn->GetToId());
	}
	
	PrimMST mst{};
	mst.CalculateMST(Graph);
}