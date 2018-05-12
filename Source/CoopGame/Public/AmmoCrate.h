// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoCrate.generated.h"

class USphereComponent;

UCLASS()
class COOPGAME_API AAmmoCrate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmoCrate();


protected:
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USphereComponent* SphereComp;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
		UParticleSystem* PickupFX;


	// Called when the game starts or when spawned


	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void PlayEffect();
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
	
};
