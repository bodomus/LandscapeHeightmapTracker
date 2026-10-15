#include "LHTContourBoxesActor.h"

#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "LandscapeProxy.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ALHTContourBoxesActor::ALHTContourBoxesActor()
{
	bIsEditorOnlyActor = true;
	bReplicates = false;
	SetCanBeDamaged(false);
	SetActorEnableCollision(false);

	Instances = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("ContourBoxes"));
	RootComponent = Instances;
	Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Instances->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	Instances->SetGenerateOverlapEvents(false);
	Instances->SetCanEverAffectNavigation(false);
	Instances->SetHiddenInGame(true);
#if WITH_EDITORONLY_DATA
	Instances->SetIsVisualizationComponent(true);
#endif

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Instances->SetStaticMesh(CubeMesh.Object);
	}
}

void ALHTContourBoxesActor::PostLoad()
{
	Super::PostLoad();
	ApplyPreviewMaterial();
}

#if WITH_EDITOR
void ALHTContourBoxesActor::PostEditUndo()
{
	Super::PostEditUndo();
	ApplyPreviewMaterial();
}
#endif

bool ALHTContourBoxesActor::Matches(const ALandscapeProxy* Landscape, const FGuid& InContourId) const
{
	return Landscape != nullptr && SourceLandscape.Get() == Landscape && ContourId == InContourId;
}

bool ALHTContourBoxesActor::IsEquivalent(
	const ALandscapeProxy* Landscape,
	const FGuid& InContourId,
	double InBoundaryHeightMeters,
	const FLinearColor& InColor,
	const TArray<FTransform>& WorldTransforms) const
{
	if (!Matches(Landscape, InContourId) ||
		!FMath::IsNearlyEqual(BoundaryHeightMeters, InBoundaryHeightMeters) ||
		!ContourColor.Equals(InColor) ||
		!Instances || Instances->GetInstanceCount() != WorldTransforms.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < WorldTransforms.Num(); ++Index)
	{
		FTransform Existing;
		if (!Instances->GetInstanceTransform(Index, Existing, true) || !Existing.Equals(WorldTransforms[Index], 0.01))
		{
			return false;
		}
	}
	return true;
}

void ALHTContourBoxesActor::ConfigureIdentity(
	ALandscapeProxy* Landscape,
	const FGuid& InContourId,
	double InBoundaryHeightMeters,
	const FLinearColor& InColor)
{
	SourceLandscape = Landscape;
	ContourId = InContourId;
	BoundaryHeightMeters = InBoundaryHeightMeters;
	ContourColor = InColor;
	Tags.AddUnique(TEXT("LandscapeHeightmapTracker.GeneratedContourBoxes"));
	ApplyPreviewMaterial();
}

void ALHTContourBoxesActor::RebuildInstances(const TArray<FTransform>& WorldTransforms)
{
	if (!Instances)
	{
		return;
	}
	Instances->Modify();
	Instances->ClearInstances();
	Instances->AddInstances(WorldTransforms, false, true, false);
	Instances->MarkRenderStateDirty();
}

int32 ALHTContourBoxesActor::GetInstanceCount() const
{
	return Instances ? Instances->GetInstanceCount() : 0;
}

void ALHTContourBoxesActor::ApplyPreviewMaterial()
{
	UMaterialInterface* BaseMaterial = Instances && Instances->GetStaticMesh()
		? Instances->GetStaticMesh()->GetMaterial(0)
		: nullptr;
	if (!BaseMaterial)
	{
		return;
	}
	PreviewMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
	if (PreviewMaterial)
	{
		PreviewMaterial->SetVectorParameterValue(TEXT("Color"), ContourColor);
		PreviewMaterial->SetVectorParameterValue(TEXT("BaseColor"), ContourColor);
		Instances->SetMaterial(0, PreviewMaterial);
	}
}
