// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HelthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);//�ñ״���,

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

	//Health����

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float DefaultHealth;

	//�׻� UFUNCTION �� ��ũ�ؾ��Ѵ�. �̺�Ʈ�� ��������Ʈ ��� �Ѵٸ�
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
	
public:
	//��������Ʈ ���� �� Ŭ���� �߰�
	UPROPERTY(BlueprintAssignable, Category ="event")
		FOnHealthChangedSignature OnHealthChanged;
};