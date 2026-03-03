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
	
	pPartitionedSpace = std::make_unique<CellSpace>(pWorld, 
		WorldSize * 2, WorldSize * 2, 
		NrOfCellsX, NrOfCellsX, 
		MaxNeighbors);
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
	
	std::vector<BlendedSteering::WeightedBehavior> WeightedBehaviors;
	WeightedBehaviors.reserve(5);
	//WeightedBehaviors.emplace_back(pSeparationBehavior.get(), 0.3f);
	//WeightedBehaviors.emplace_back(pCohesionBehavior.get(), 0.3f);
	//WeightedBehaviors.emplace_back(pAlignmentBehavior.get(), 0.2f);
	//WeightedBehaviors.emplace_back(pWanderBehavior.get(), 0.2f);
	WeightedBehaviors.emplace_back(pSeekBehavior.get(), 0.1f);
	
	pBlendedSteering = std::make_unique<BlendedSteering>(WeightedBehaviors);
	
	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{pEvadeBehavior.get(), pBlendedSteering.get()});
	
	for (const auto pAgent : Agents)
	{
		//pAgent->SetSteeringBehavior(pPrioritySteering.get());
		pAgent->SetSteeringBehavior(pBlendedSteering.get());
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
	
	for (auto pAgent : Agents)
	{
		RegisterNeighbors(pAgent);
		pAgent->Tick(DeltaTime);
		
		
		
	}
	
	for (int idx{0}; idx < FlockSize; ++idx)
	{
		auto pAgent = Agents[idx];
		
		RegisterNeighbors(pAgent);
		pAgent->Tick(DeltaTime);
		
		pPartitionedSpace->UpdateAgentCell(*pAgent, OldPositions[idx]);
		OldPositions[idx] = pAgent->GetPosition();
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
	if (DebugRenderNeighborhood)
	{
		RenderNeighborhood();
	}
	
	pPartitionedSpace->RenderCells();
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

//#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;
	
	for (auto* other : Agents)
	{
		//Make sure agent is not included in its own neighborhood
		if (other == pAgent) continue;
		
		const float Distance = FVector2D::Distance(other->GetPosition(), pAgent->GetPosition());
		if (Distance < NeighborhoodRadius) //TODO SquaredLength gebruiken?
		{
			Neighbors[NrOfNeighbors] = other;
			++NrOfNeighbors;
			
			//Stop checking neighbors if memory pool is full
			if (NrOfNeighbors >= MaxNeighbors) return;
		}
	}
}
//#endif

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


