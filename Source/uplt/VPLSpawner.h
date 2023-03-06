// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PointLight.h"
#include "VPLSpawner.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPLT_API UVPLSpawner : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVPLSpawner();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	/* Number of VPLs to generate along the altitude polar axis. Total VPLs generated is numVPLSamplesAltitudeAxis * numVPLSapmlesAzimuthAxis. */
	UPROPERTY(EditAnywhere, Category = "VPLs|VPL Spawining");
	int32 numVPLSamplesAltitudeAxis = 3;
	/* Number of VPLs to generate along azimuth polar axis. Total VPLs generated is numVPLSamplesAltitudeAxis * numVPLSapmlesAzimuthAxis. */
	UPROPERTY(EditAnywhere, Category = "VPLs|VPL Spawining");
	int32 numVPLSamplesAzimuthAxis = 12;

	/* How far VPLs can be from the light source */
	UPROPERTY(EditAnywhere, Category = "VPLs|VPL Spawining");
	float maxVPLDistance = 500.0f;

	/* Angle to randomly jitter VPL sample direction in degrees. Setting to zero gives uniform sampling */
	UPROPERTY(EditAnywhere, Category = "VPLs|VPL Spawining");
	float jitterDegrees = 0.0f;

	float jitterRadians = 0.0f;

	/* Whether to display VPL locations */
	UPROPERTY(EditAnywhere, Category = "VPLs|VPL Spawining");
	bool debugInfoVisible = false;

	/* Strength of the direct light */
	UPROPERTY(EditAnywhere, Category = "VPLs|Light Properties");
	float lightStrength = 2.0f;

	/* Whether VPLs cast shadows */
	UPROPERTY(EditAnywhere, Category = "VPLs|Light Properties");
	bool VPLsCastShadows = true;

	/* Quality of VPL shadows */
	UPROPERTY(EditAnywhere, Category = "VPLs|Light Properties");
	float shadowResolutionScale = 1.0f;

	/* Length of VPL contact shadows in world units */
	UPROPERTY(EditAnywhere, Category = "VPLs|Light Properties");
	float contactShadowLength = 0.0f;

	/* Whether light falloff uses the inverse square distance formual */
	UPROPERTY(EditAnywhere, Category = "VPLs|Light Properties");
	bool usePhysicalAttenuation = false;

	TArray<APointLight*> VPLs;

	void SpawnLights(TArray<APointLight*>& lights);

	TObjectPtr<APointLight> SpawnLight(const FVector& position, const FColor& color, float intensity);

	// returns whether a visible point was found
	bool GetRandomVisiblePoint(const FVector& direction, FVector& position, FColor& color, float& lambert) const;

	FVector GetVPLSampleDirection(int currentSampleAltitude, int currentSampleAzimuth, bool jittered) const;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
