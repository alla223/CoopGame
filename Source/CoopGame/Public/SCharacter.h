// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class SWeapon;
class USHealthComponent;

UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	//UCapsuleComponent* CapsuleComp = GetCapsuleComponent();

	//add a crouch
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	UCharacterMovementComponent* CharacterMovment = GetCharacterMovement();
	void BeginCrouch();
	void EndCrouch();
	bool bWantsToCrouch;
	float CrouchedCapsuleHalfHeight;
	float DefaultCapsuleHalfHeight;
	void CrouchImpl(float DeltaTime);

	//Aiming down sight
	bool bWantsToZoom;
	

	//fov의 초기값
	float DefaultFOV;
	//줌 상태의 fov값,블루프린트에서 수정가능하도록 왜냐하면 디자이너가 수정할 수 있게 하기 위해서
	UPROPERTY(EditDefaultsOnly,Category="Player")
	float ZoomedFOV;

	void BeginZoom();
	void EndZoom();

	UPROPERTY(EditDefaultsOnly, Category = "Player",meta=(ClampMin=0.1 , ClampMax=100))
	float ZoomInterpSpeed;

	virtual FVector GetPawnViewLocation() const override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USpringArmComponent* SpringArmComp;

	//Health 이벤트 생성하기 위한 컴포넌트 또한 블루프린트에서 생성한 이벤트를 대체가능하다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	//블루프린트에 노출시키면 플레이어캐릭터 이벤트그래프에서 뷰포트에 추가하여 현재총알개수를 보이게 할 수 있다.
	//클라이언트는 currentweapon 변수값을 설정할수 없기 때문에 (beginplay가 role autority 로 서버에서만 실행해서 replicate하기때문에) 여기서 변수를 복제
	
	UPROPERTY(Replicated,EditAnywhere, BlueprintReadOnly,Category = "Player")
	ASWeapon* CurrentWeapon;
	
	UPROPERTY(EditDefaultsOnly,Category="Player")
	TSubclassOf<ASWeapon>StarterWeaponClass; 

	void StartFire();

	void StopFire();
	UPROPERTY(VisibleDefaultsOnly,Category="Player")

	FName WeaponAttachSocketName;

	UFUNCTION(BlueprintCallable, Category = "Reload")
	void OnReload();

	//바인드 itself
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HelthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/*pawn이 이미 죽었나?*/
	UPROPERTY(Replicated,BlueprintReadOnly,Category="Player")
	bool bDied;
	


	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ReloadAnim;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//ammo crate 획득 시 총알 증가
	void AddBullets();
	///////////////////////////////////////////
	//Recoil
	///////////////////////////////////////////
	UFUNCTION()
		void ApplyRecoilToCharacter(float Value);

};
