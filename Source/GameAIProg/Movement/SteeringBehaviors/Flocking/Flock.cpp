#include "Flock.h"

#include "AssetDefinitionAssetInfo.h"
#include "FlockingSteeringBehaviors.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
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
	Neighbors.SetNum(FlockSize / 2);
	
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, 
		WorldSize * 2, WorldSize * 2, 
		NrOfCellsX, NrOfCellsX, 
		Neighbors.Num());
	
	OldPositions.SetNum(FlockSize);
	
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
			
			pPartitionedSpace->AddAgent(*pAgent);
			OldPositions[idx] = pAgent->GetPosition();
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Array size: %d"), Agents.Num());

	////////////////
	///Steering Behaviors
	
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pAlignmentBehavior = std::make_unique<Alignment>(this);
	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();
	pEvadeBehavior->SetEvadeRadius(250.f);
	
	std::vector<BlendedSteering::WeightedBehavior> WeightedBehaviors;
	WeightedBehaviors.reserve(5);
	WeightedBehaviors.emplace_back(pSeparationBehavior.get(), 0.3f);
	WeightedBehaviors.emplace_back(pCohesionBehavior.get(), 0.3f);
	WeightedBehaviors.emplace_back(pAlignmentBehavior.get(), 0.2f);
	WeightedBehaviors.emplace_back(pWanderBehavior.get(), 0.2f);
	WeightedBehaviors.emplace_back(pSeekBehavior.get(), 0.05f);
	
	pBlendedSteering = std::make_unique<BlendedSteering>(WeightedBehaviors);
	
	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{pEvadeBehavior.get(), pBlendedSteering.get()});
	
	for (const auto pAgent : Agents)
	{
		pAgent->SetSteeringBehavior(pPrioritySteering.get());
	}
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
	
	SetTarget_Evade();
	
	
	for (int idx{0}; idx < FlockSize; ++idx)
	{
		auto* pAgent = Agents[idx];
		RegisterNeighbors(pAgent);
		pAgent->Tick(DeltaTime);
		
		if (UseSpacePartitioning)
		{
			pPartitionedSpace->UpdateAgentCell(*(Agents[idx]), OldPositions[idx]);
			OldPositions[idx] = pAgent->GetPosition();
		}
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
	if (DebugRenderNeighborhood)
	{
		RenderNeighborhood();
	}
	
	if (UseSpacePartitioning && DebugRenderPartitions)
	{
		pPartitionedSpace->RenderCells(DebugRenderAgentsInPartition);
	}
	
	//Render Steering for first 10 agents in flock
	for (int idx{}; idx < 10; ++idx)
	{
		Agents[idx]->SetDebugRenderingEnabled(DebugRenderSteering);
	}
	
	if (DebugRenderEvadeTarget)
	{
		RenderEvadeTarget();
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
		ImGui::Checkbox("Use Space Partitioning", &UseSpacePartitioning);
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::Text("Debug Rendering");
		ImGui::Spacing();
		ImGui::Checkbox("Debug render steering", &DebugRenderSteering);
		ImGui::Spacing();
		ImGui::Checkbox("Debug render neighborhood", &DebugRenderNeighborhood);
		ImGui::Spacing();
		ImGui::Checkbox("Debug render evade target", &DebugRenderEvadeTarget);
		
		if (UseSpacePartitioning)
		{
			ImGui::Spacing();
			ImGui::Checkbox("Debug render partitions", &DebugRenderPartitions);
			
			if (DebugRenderPartitions)
			{
				ImGui::Spacing();
				ImGui::Checkbox("Debug agents in partition",&DebugRenderAgentsInPartition);
				ImGui::Spacing();
			}
		}

		
		//Behavior Weights
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();
		
		//Separation
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
			pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal){pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal;}, "%.2f");
		
		//Cohesion
        ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
        	pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
        	[this](float InVal){pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal;}, "%.2f");
		
        //Alignment
        ImGuiHelpers::ImGuiSliderFloatWithSetter("Alignment",
        	pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight, 0.f, 1.f,
        	[this](float InVal){pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = InVal;}, "%.2f");
		
		//Wander
        ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
        pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight, 0.f, 1.f,
		[this](float InVal){pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = InVal;}, "%.2f");
		
		//Seek
		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
   pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight, 0.f, 1.f,
   [this](float InVal){pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = InVal;}, "%.2f");
		
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
		//TODO: draw Circle with matrix
	
	//Bounding box
	DrawDebugBox(pWorld, FVector(Agents[0]->GetPosition(), 0.f), 
		FVector(NeighborhoodRadius, NeighborhoodRadius, 1.f), FColor::Blue);
	
	//Mark all included neighbors
	RegisterNeighbors(Agents[0]);
	for (int idx = 0; idx < GetNrOfNeighbors(); ++idx)
	{
		DrawDebugPoint(pWorld, FVector(GetNeighbors()[idx]->GetPosition(), 20.f), 30.f, FColor::Green);
	}
	
	//Draw Average Position of Neighorhood
	FVector2D AvgPos{GetAverageNeighborPos()};
	DrawDebugPoint(pWorld, FVector(AvgPos, 20.f), 30.f, FColor::Red);
	
}

void Flock::RenderEvadeTarget() const
{
	//Agent Position
	DrawDebugPoint(pWorld, FVector(pAgentToEvade->GetPosition(), 20.f), 30.f, FColor::Yellow);
	
	//Draw Evade 
	DrawDebugCircle(pWorld, FVector(pAgentToEvade->GetPosition(), 0.f), pEvadeBehavior->GetEvadeRadius(), 16, FColor::Yellow, false, -1, 0, 0,
		FVector(0,1,0), FVector(1,0,0));
}

const TArray<ASteeringAgent*>& Flock::GetNeighbors() const
{
	if (UseSpacePartitioning)
		return pPartitionedSpace->GetNeighbors();
	else
		return Neighbors;
}

int Flock::GetNrOfNeighbors() const
{
	if (UseSpacePartitioning)
		return pPartitionedSpace->GetNrOfNeighbors();
	else
		return NrOfNeighbors;
}

void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	if (UseSpacePartitioning)
	{
		pPartitionedSpace->RegisterNeighbors(*pAgent, NeighborhoodRadius);
	}
	else
	{
		NrOfNeighbors = 0;
	
		for (auto* other : Agents)
		{
			//Make sure agent is not included in its own neighborhood
			if (other == pAgent) continue;
		
			const float DistanceSqr = FVector2D::DistSquared(other->GetPosition(), pAgent->GetPosition());
			if (DistanceSqr < NeighborhoodRadius * NeighborhoodRadius)
			{
				Neighbors[NrOfNeighbors] = other;
				++NrOfNeighbors;
			
				//Stop checking neighbors if memory pool is full
				if (NrOfNeighbors >= Neighbors.Num()) return;
			}
		}
	}
}

FVector2D Flock::GetAverageNeighborPos() const
{
	FVector2D avgPosition = FVector2D::ZeroVector;

	for (int Idx = 0; Idx < GetNrOfNeighbors(); ++Idx)
	{
		avgPosition += GetNeighbors()[Idx]->GetPosition();
	}
	avgPosition /= GetNrOfNeighbors();
	
	return avgPosition;
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	FVector2D avgVelocity = FVector2D::ZeroVector;

	for (int Idx = 0; Idx < GetNrOfNeighbors(); ++Idx)
	{
		avgVelocity += GetNeighbors()[Idx]->GetLinearVelocity();
	}
	avgVelocity /= GetNrOfNeighbors();
	
	return avgVelocity;
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	pSeekBehavior->SetTarget(Target);
}

void Flock::SetTarget_Evade()
{
	FTargetData Target;
	Target.Position = pAgentToEvade->GetPosition();
	Target.Orientation = pAgentToEvade->GetRotation();
	Target.LinearVelocity = pAgentToEvade->GetLinearVelocity();
	Target.AngularVelocity = pAgentToEvade->GetAngularVelocity();
	
	pEvadeBehavior->SetTarget(Target);
}


