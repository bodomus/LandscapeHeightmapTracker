#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LandscapeProxy.h"
#include "LHTContourBoxesActor.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS(NotBlueprintable, NotPlaceable)
class LANDSCAPEHEIGHTMAPTRACKER_API ALHTContourBoxesActor : public AActor
{
	GENERATED_BODY()

public:
	ALHTContourBoxesActor();

	virtual bool IsEditorOnly() const override { return true; }
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

	bool Matches(const ALandscapeProxy* Landscape, const FGuid& InContourId) const;
	bool IsEquivalent(
		const ALandscapeProxy* Landscape,
		const FGuid& InContourId,
		double InBoundaryHeightMeters,
		const FLinearColor& InColor,
		const TArray<FTransform>& WorldTransforms) const;
	void ConfigureIdentity(
		ALandscapeProxy* Landscape,
		const FGuid& InContourId,
		double InBoundaryHeightMeters,
		const FLinearColor& InColor);
	void RebuildInstances(const TArray<FTransform>& WorldTransforms);

	const FGuid& GetContourId() const { return ContourId; }
	ALandscapeProxy* GetSourceLandscape() const { return SourceLandscape.Get(); }
	int32 GetInstanceCount() const;

private:
	void ApplyPreviewMaterial();

	UPROPERTY(VisibleAnywhere, Category="Landscape Heightmap Tracker")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> Instances;

	UPROPERTY(VisibleAnywhere, Category="Landscape Heightmap Tracker")
	TSoftObjectPtr<ALandscapeProxy> SourceLandscape;

	UPROPERTY(VisibleAnywhere, Category="Landscape Heightmap Tracker")
	FGuid ContourId;

	UPROPERTY(VisibleAnywhere, Category="Landscape Heightmap Tracker")
	double BoundaryHeightMeters = 0.0;

	UPROPERTY(VisibleAnywhere, Category="Landscape Heightmap Tracker")
	FLinearColor ContourColor = FLinearColor::White;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;
};
