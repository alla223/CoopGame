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
	UStaticMeshComponent* MeshComp;//�ð��� ȿ���� �ڽ� ������Ʈ�� �ֱ� ����

	//Health �̺�Ʈ �����ϱ� ���� ������Ʈ�̸�, ���� �������Ʈ���� ������ �̺�Ʈ�� ��ü�����ϴ�
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
		URadialForceComponent* RadialForceComp;

	//���ε� itself
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HelthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);


	/*pawn�� �̹� �׾���?*/
	UPROPERTY(ReplicatedUsing="OnRep_Exploded",BlueprintReadOnly, Category = "ExplosiveBarrel")
		bool bExploded;

	//bExpolded�� �ٲ𶧸��� Ŭ���̾�Ʈ���� �����Ѵ�.
	UFUNCTION()
		void OnRep_Exploded();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosiveBarrel")
		UParticleSystem* ExplosiveEffect;

	UPROPERTY(EditDefaultsOnly, Category = "ExplosiveBarrel")
		float ExplosionImpulse;

	//�������� �޽��� �����Ҷ� ��ü�ȴ�
	UPROPERTY(EditDefaultsOnly, Category = "ExplosiveBarrel")
		UMaterialInterface* ExplodedMaterial;
	UPROPERTY(EditDefaultsOnly, Category = "ExplosiveBarrel")
		float	UpSpeed;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
