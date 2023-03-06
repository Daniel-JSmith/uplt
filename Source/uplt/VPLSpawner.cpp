// Fill out your copyright notice in the Description page of Project Settings.


#include "VPLSpawner.h"

#include "Components/PointLightComponent.h"

// Sets default values for this component's properties
UVPLSpawner::UVPLSpawner()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// helper functions
namespace
{
	void normalizeColor(FLinearColor& color)
	{
		auto vec = FVector(color);
		vec.Normalize();
		color = FLinearColor(vec);
	}

	void normalizeColor(FColor& color)
	{
		auto linear = color.ReinterpretAsLinear();
		normalizeColor(linear);
		color = linear.ToFColor(true);
	}
}

// Called when the game starts
void UVPLSpawner::BeginPlay()
{
	Super::BeginPlay();

	// spawn direct lights
	auto color = FColor::White;
	normalizeColor(color);
	SpawnLight(GetOwner()->GetActorLocation(), color, lightStrength);

	// set VPL strength
	lightStrength = lightStrength / (numVPLSamplesAltitudeAxis * numVPLSamplesAzimuthAxis);
}


// Called every frame
void UVPLSpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	jitterRadians = FMath::DegreesToRadians(jitterDegrees);

	// remove previous VPLs
	for (auto& light : VPLs)
	{
		light->Destroy();
	}
	VPLs.Reset();

	// spawn new VPLs
	for (int i = 0; i < numVPLSamplesAltitudeAxis; i++)
	{
		for (int j = 0; j < numVPLSamplesAzimuthAxis; j++)
		{
			auto rayDirection = GetVPLSampleDirection(i, j, true);

			auto position = FVector();
			auto color = FColor();
			auto lambert = 1.0f;
			auto result = GetRandomVisiblePoint(rayDirection, position, color, lambert);
			if (result)
			{
				VPLs.Add(SpawnLight(position, color, lightStrength * lambert));
			}
		}
	}
}

TObjectPtr<APointLight> UVPLSpawner::SpawnLight(const FVector& position, const FColor& color, float intensity)
{	
	auto light = GetWorld()->SpawnActor<APointLight>(position, GetOwner()->GetActorRotation());
	light->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	light->SetLightColor(color);
	light->SetCastShadows(false);

	auto pointLightComponents = TArray<UPointLightComponent*>();
	light->GetComponents<UPointLightComponent>(pointLightComponents);
	
	pointLightComponents[0]->bUseInverseSquaredFalloff = usePhysicalAttenuation;

	pointLightComponents[0]->SetIntensityUnits(usePhysicalAttenuation? ELightUnits::Candelas : ELightUnits::Unitless);
	pointLightComponents[0]->SetIntensity(intensity);

	pointLightComponents[0]->ShadowResolutionScale = shadowResolutionScale;
	pointLightComponents[0]->ContactShadowLengthInWS = true;
	pointLightComponents[0]->ContactShadowLength = contactShadowLength;
	light->SetCastShadows(VPLsCastShadows);

	return light;
}

FVector UVPLSpawner::GetVPLSampleDirection(int currentSampleAltitude, int currentSampleAzimuth, bool jittered) const
{
	float altitudeStep = UE_HALF_PI / numVPLSamplesAltitudeAxis;
	float azimuthStep = UE_TWO_PI / numVPLSamplesAzimuthAxis;

	float altitude = (currentSampleAltitude + 0.5) * altitudeStep + UE_PI;
	float azimuth = currentSampleAzimuth * azimuthStep;
	altitude += FMath::RandRange(-jitterRadians, jitterRadians);
	azimuth += FMath::RandRange(-jitterRadians, jitterRadians);
	return FVector(FMath::Sin(altitude) * FMath::Cos(azimuth),
		FMath::Sin(altitude) * FMath::Sin(azimuth),
		FMath::Cos(altitude));
}

bool UVPLSpawner::GetRandomVisiblePoint(const FVector& direction, FVector& position, FColor& color, float& lambert) const
{
	auto castParams = FCollisionQueryParams::DefaultQueryParam;
	castParams.bTraceComplex = true;
	castParams.bReturnFaceIndex = true;

	FVector startPosition = GetOwner()->GetActorLocation();
	FVector endPosition = GetOwner()->GetActorLocation() + direction * maxVPLDistance;

	auto resultDetails = FHitResult{};
	auto result = GetWorld()->LineTraceSingleByChannel(resultDetails, startPosition, endPosition, ECC_Visibility, castParams);

	if (result)
	{
		auto meshSection = -1;
		auto material = resultDetails.GetComponent()->GetMaterialFromCollisionFaceIndex(resultDetails.FaceIndex, meshSection);

		auto materialParamInfo = FHashedMaterialParameterInfo(TEXT("Color"));
		auto materialColor = FLinearColor();
		auto parameterQueryResult = material->GetVectorParameterValue(materialParamInfo, materialColor);

		position = resultDetails.ImpactPoint;
		normalizeColor(materialColor);
		color = (materialColor).ToFColor(true);
		lambert = FMath::Max(0, resultDetails.ImpactNormal.Dot(-direction));

		if (debugInfoVisible)
		{
			DrawDebugSphere(GetWorld(), resultDetails.ImpactPoint, 10, 8, materialColor.ToFColor(true));
			DrawDebugLine(GetWorld(), startPosition, resultDetails.ImpactPoint, FColor::Red);
		}
	}

	return result;
}

void UVPLSpawner::SpawnLights(TArray<APointLight*>& lights)
{

}