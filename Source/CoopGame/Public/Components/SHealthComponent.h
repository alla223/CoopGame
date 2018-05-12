// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HelthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);//시그니쳐,

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class COOPGAME_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//Health변수

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	//항상 UFUNCTION 을 마크해야한다. 이벤트나 델리게이트 사용 한다면
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
public:
	//델리게이트 선언 후 클래스 추가
	UPROPERTY(BlueprintAssignable, Category ="event")
		FOnHealthChangedSignature OnHealthChanged;
};