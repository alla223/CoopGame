// Fill out your copyright notice in the Description page of Project Settings.

#include "SProjectileWeapon.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"

ASProjectileWeapon::ASProjectileWeapon(){
	AmmoPool = 10;
	CurrentBullets = 1;
	MaxAmmo = 1;
	BaseDamage = 2000;
}

void ASProjectileWeapon::Fire()
{
	//웨폰을갖고 있는 오너, 누가 무기를 갖고있는지

	AActor* MyOwner = GetOwner();
	if (MyOwner&&ProjectileClass)
	{
		if (CurrentBullets <= 0) { return; }
		CurrentBullets = CurrentBullets - 1;

		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector MuzzleLocation=MeshComp->GetSocketLocation(MuzzleSocketName);
	

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		//프로젝타일은 월드에 액터를 스폰하는 역할
		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams); //프로젝타일 클래스를 스폰하고,


	}
}


