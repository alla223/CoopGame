// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"


class UParticleSystem;
class USkeletalMeshComponent;
class UDamageType;

UENUM()
enum class EWeaponState
{
	Idle,
	Firing,
	Reloading
};

//��Ƽ���� Hit FX�� ǥ�鿡 ���� �ٸ� FX ����
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
		TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY()
		FVector_NetQuantize TraceTo;
};


UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	/*While Firing ����Ʈ �� ī�޶� ��鸲*/
	void PlayFireEffects(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* MeshComp;
	/*//world�� �����Ѵ�. ���� eye ũ�ν������ġ�κ��� blueprintcallable*/
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		TSubclassOf<UDamageType> DamageType;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName MuzzleSocketName;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		FName TargetName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* MuzzleEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* DefaultImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* FleshImpactEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		UParticleSystem* TracerEffect;
	
	UPROPERTY(EditDefaultsOnly,  Category = "Weapon")
	TSubclassOf<UCameraShake> FireCamShake;
	
	/*�⺻ ������*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	FTimerHandle TimerHandle_TimeBetweenShots;
	
	/*�׷����̵� ���ĸ� ��ӽ�Ű�� ���� virtual Ű���� �߰�*/
	//UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();
	
	UFUNCTION(Server,Reliable,WithValidation) 
	void ServerFire();

	//������ �߻� �ð�
	float LastFireTime;
	
	/*rpm- �д� �߻�� �Ѿ˵�*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;
	
	/*�ð����� �߻��, RateOfFire�� �̿��Ѵ�*/
	float TimeBetweenShots;


	/*�������Ǿ��� �� ĳ����Ŭ������ ĳ���ø� �����ؼ� SWeaponŬ�������� Ammocarte�� ������ �����ϰ� �Ǿ���. ������ζ�� AmmoCrate�� ���������Ѵ�.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoCrate")
		int32 AmmoCrateBulletsCount;
	
	//recoil
	UPROPERTY(EditDefaultsOnly,Category=Weapon)
		float MaxRecoilAmount;
	//recoil
	UFUNCTION()
		void AddRecoil();

	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
		FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	/** play weapon animations */
	float PlayWeaponAnimation(UAnimMontage* Pawn3P);
	
	class ASCharacter* MyOwner;//= Cast<ASCharacter>(GetOwner());

private:
	FTimerHandle TimerHandle_ReloadWeapon;

	FTimerHandle TimerHandle_StopReload;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	/*SCharacter���� ȣ���� ���� public ����, ���� �ڵ��߻縦 ���� �߰��� �Լ�*/
	 void StartFire();

	/*���� �ڵ��߻縦 ���� �߰��� �Լ�*/
	 void StopFire();
	
	/*���� �������� ���� �Լ�*/
	void StartReload();
	/*���ε� �ִϸ��̼� ��Ÿ��*/
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ReloadAnim;


	/*��ź ���� �Ѿ��� ����*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		int32 BulletsPerMag;

	/*������ �ִ� ��ü �Ѿ��� ����*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		int32 AmmoPool;

	/*���� �Ѿ� ����*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		int32 CurrentBullets;
	/*���� �� �ִ� �Ѿ��� �ִ��*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo;

	/*ammocrate �Ծ��� �� �Ѿ�����*/
	void AddBullets();
};
