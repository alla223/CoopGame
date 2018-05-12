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
	

	//fov�� �ʱⰪ
	float DefaultFOV;
	//�� ������ fov��,�������Ʈ���� ���������ϵ��� �ֳ��ϸ� �����̳ʰ� ������ �� �ְ� �ϱ� ���ؼ�
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

	//Health �̺�Ʈ �����ϱ� ���� ������Ʈ ���� �������Ʈ���� ������ �̺�Ʈ�� ��ü�����ϴ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	//�������Ʈ�� �����Ű�� �÷��̾�ĳ���� �̺�Ʈ�׷������� ����Ʈ�� �߰��Ͽ� �����Ѿ˰����� ���̰� �� �� �ִ�.
	//Ŭ���̾�Ʈ�� currentweapon �������� �����Ҽ� ���� ������ (beginplay�� role autority �� ���������� �����ؼ� replicate�ϱ⶧����) ���⼭ ������ ����
	
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

	//���ε� itself
	UFUNCTION()
		void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HelthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	/*pawn�� �̹� �׾���?*/
	UPROPERTY(Replicated,BlueprintReadOnly,Category="Player")
	bool bDied;
	


	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ReloadAnim;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//ammo crate ȹ�� �� �Ѿ� ����
	void AddBullets();
	///////////////////////////////////////////
	//Recoil
	///////////////////////////////////////////
	UFUNCTION()
		void ApplyRecoilToCharacter(float Value);

};
