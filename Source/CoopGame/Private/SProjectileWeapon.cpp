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
	//���������� �ִ� ����, ���� ���⸦ �����ִ���

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
		//������Ÿ���� ���忡 ���͸� �����ϴ� ����
		GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams); //������Ÿ�� Ŭ������ �����ϰ�,


	}
}


