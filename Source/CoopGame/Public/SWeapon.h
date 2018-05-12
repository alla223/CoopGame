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

//멀티에서 Hit FX와 표면에 따른 다른 FX 적용
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



	/*While Firing 이펙트 및 카메라 흔들림*/
	void PlayFireEffects(FVector TraceEnd);

	void PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint);

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* MeshComp;
	/*//world를 추적한다. 폰의 eye 크로스헤어위치로부터 blueprintcallable*/
	
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
	
	/*기본 데미지*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float BaseDamage;

	FTimerHandle TimerHandle_TimeBetweenShots;
	
	/*그레네이드 런쳐를 상속시키기 위해 virtual 키워드 추가*/
	//UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Fire();
	
	UFUNCTION(Server,Reliable,WithValidation) 
	void ServerFire();

	//마지막 발사 시간
	float LastFireTime;
	
	/*rpm- 분당 발사된 총알들*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float RateOfFire;
	
	/*시간마다 발사수, RateOfFire를 이용한다*/
	float TimeBetweenShots;


	/*오버랩되었을 때 캐릭터클래스로 캐스팅만 가능해서 SWeapon클래스에서 Ammocarte의 수량을 관리하게 되었다. 원래대로라면 AmmoCrate의 변수여야한다.*/
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
	
	/*SCharacter에서 호출을 위해 public 선언, 무기 자동발사를 위해 추가한 함수*/
	 void StartFire();

	/*무기 자동발사를 위해 추가한 함수*/
	 void StopFire();
	
	/*무기 재장전을 위한 함수*/
	void StartReload();
	/*리로드 애니메이션 몽타주*/
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ReloadAnim;


	/*총탄 마다 총알의 개수*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		int32 BulletsPerMag;

	/*가지고 있는 전체 총알의 개수*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		int32 AmmoPool;

	/*현재 총알 개수*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		int32 CurrentBullets;
	/*가질 수 있는 총알의 최대수*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo;

	/*ammocrate 먹었을 때 총알증가*/
	void AddBullets();
};
