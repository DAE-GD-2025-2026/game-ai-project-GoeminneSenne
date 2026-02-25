#include "Flock.h"

#include "AssetDefinitionAssetInfo.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	Agents.SetNum(FlockSize);
	Neighbors.SetNum(MaxNeighbors);
	
	FActorSpawnParameters SpawnParams{};
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	for (int idx = 0; idx < FlockSize; ++idx)
	{
		FVector SpawnPos {
			FMath::RandRange(-WorldSize, WorldSize),
			FMath::RandRange(-WorldSize, WorldSize),
			90
		};
		
		ASteeringAgent* pAgent = pWorld->SpawnActor<ASteeringAgent>(AgentClass, SpawnPos, 
			FRotator::ZeroRotator, SpawnParams);
		
		if (pAgent)
		{
			pAgent->SetActorTickEnabled(false);
			pAgent->SetDebugRenderingEnabled(false);
			Agents[idx] = pAgent;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Array size: %d"), Agents.Num());

	////////////////
	///Steering Behaviors
	
	//std::unique_ptr<Separation> pSeparationBehavior{};
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	//std::unique_ptr<VelocityMatch> pVelMatchBehavior{};
	pSeekBehavior = std::make_unique<Seek>();
	//std::unique_ptr<Wander> pWanderBehavior{};
	//std::unique_ptr<Evade> pEvadeBehavior{};
	
	//TODO: TEMP 
	for (const auto pAgent : Agents)
	{
		pAgent->SetSteeringBehavior(pCohesionBehavior.get());
	}
	
	
 // TODO: initialize the flock and the memory pool

}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
}

void Flock::Tick(float DeltaTime)
{
 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
	
	for (auto pAgent : Agents)
	{
		RegisterNeighbors(pAgent);
		pAgent->Tick(DeltaTime);
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
	if (DebugRenderNeighborhood)
	{
		RenderNeighborhood();
	}
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
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

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

 //Debugrender the neighbors for the first agent in the flock
void Flock::RenderNeighborhood()
{
	//Neighborhood Radius
	DrawDebugCircle(pWorld, FVector(Agents[0]->GetPosition(),0.f), NeighborhoodRadius, 16, FColor::Blue, false, -1, 0, 0, 
		FVector(0,1,0), FVector(1,0,0));

	//Mark all included neighbors
	//TODO kleur veranderen van agents ipv punt erop te tekenen
	RegisterNeighbors(Agents[0]);
	for (int idx = 0; idx < NrOfNeighbors; ++idx)
	{
		DrawDebugPoint(pWorld, FVector(Neighbors[idx]->GetPosition(), 20.f), 30.f, FColor::Green);
	}
	
	//Draw Average Position of Neighorhood
	FVector2D AvgPos{GetAverageNeighborPos()};
	DrawDebugPoint(pWorld, FVector(AvgPos, 20.f), 30.f, FColor::Red);
	
	//TODO: Draw average lineair velocity
	
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;
	
	for (auto* other : Agents)
	{
		//Make sure agent is not included in its own neighborhood
		if (other == pAgent) continue;
		
		const float Distance = FVector2D::Distance(other->GetPosition(), pAgent->GetPosition());
		if (Distance < NeighborhoodRadius)
		{
			Neighbors[NrOfNeighbors] = other;
			++NrOfNeighbors;
			
			//Stop checking neighbors if memory pool is full
			if (NrOfNeighbors >= MaxNeighbors) return;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D avgPosition = FVector2D::ZeroVector;

	for (int Idx = 0; Idx < NrOfNeighbors; ++Idx)
	{
		avgPosition += Neighbors[Idx]->GetPosition();
	}
	avgPosition /= NrOfNeighbors;
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

 // TODO: Check if velocity from each neighbor should be normalized before addition
	for (int Idx = 0; Idx < NrOfNeighbors; ++Idx)
	{
		avgVelocity += Neighbors[Idx]->GetLinearVelocity();
	}
	avgVelocity /= NrOfNeighbors;
	
	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
}

