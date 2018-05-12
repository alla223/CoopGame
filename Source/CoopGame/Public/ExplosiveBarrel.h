// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/MeshComponent.h>
#include "ExplosiveBarrel.generated.h"

//foredeclar
class SHealthComponent;
class UParticleSystem;
class UBoxComponent;
class URadialForceComponent;
UCLASS()
class COOPGAME_API AExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosiveBarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* MeshComp;//시각적 효과를 박스 컴포넌트에 주기 위해

	//Health 이벤트 생성하기 위한 컴포넌트이며, 또한 블루프린트에서 생성한 이벤트를 대체가능하다
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		URadialForceComponent* RadialForceComp;

	//바인드 itself
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HelthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


	/*pawn이 이미 죽었나?*/
	UPROPERTY(ReplicatedUsing="OnRep_Exploded",BlueprintReadOnly, Category = "ExplosiveBarrel")
		bool bExploded;

	//bExpolded가 바뀔때마다 클라이언트에서 반응한다.
	UFUNCTION()
		void OnRep_Exploded();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
		UParticleSystem* ExplosiveEffect;

	UPROPERTY(EditDefaultsOnly, Category = "ExplosiveBarrel")
		float ExplosionImpulse;

	//오리지널 메쉬가 폭발할때 대체된다
	UPROPERTY(EditDefaultsOnly, Category = "ExplosiveBarrel")
		UMaterialInterface* ExplodedMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "ExplosiveBarrel")
		float	UpSpeed;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
