// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOPGAME_API ASProjectileWeapon : public ASWeapon
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	ASProjectileWeapon();
protected:
			virtual void Fire() override;
		
			UPROPERTY(EditDefaultsOnly,Category="Projectile")
			TSubclassOf <AActor> ProjectileClass;
	
			// ������� ���͵� �������ִ� ����Ŭ����	
};
