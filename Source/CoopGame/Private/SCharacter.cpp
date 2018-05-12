// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "CoopGame.h"
#include "SHealthComponent.h"
#include "SWeapon.h"
#include "Animation/AnimInstance.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;// ī�޶� ȸ��
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));//�����Ϸ����� ī�޶����� ��� Ÿ�����˷��ش�.
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	//crouch ����Ʈ : �������� �츮�� ũ���ġ�� �㰡�Ѱ��� üũ�Ѵ�. �Ʒ� �Լ��� ai�Լ�����
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;
	//ĸ�����Ͽ��� weapon trace�� ������� ���ϰ� �����ϰ� �Ѵ�.
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);


	ZoomedFOV = 65.0f;
	DefaultFOV = 90.0f;
	ZoomInterpSpeed = 20;

	WeaponAttachSocketName = "WeaponSocket";
	
	GetCapsuleComponent()->InitCapsuleSize(48.0f, 96.0f);
	CrouchedCapsuleHalfHeight = 48.0f;
	DefaultCapsuleHalfHeight = 96.0f;



}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
	
	//���� �������� , �������� ����, ���������� currentweapon ������ ���������� Ŭ���̾�Ʈ���� ���� ���������ʴ´�.
	if (Role == ROLE_Authority) { 

		//����Ʈ ���� ���� - �������Ʈ ��ȯ�ڵ� , ������ Ŭ������ �ʿ��ϴ�
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);//�� ������ ������
			CurrentWeapon->AttachToComponent(Cast<USceneComponent>(GetMesh()), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
		}
	}
}


void ASCharacter::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector()* Value);
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector()* Value);
}

void ASCharacter::BeginCrouch()
{
	Crouch();// �׳� �̰� ���̴�..
	bWantsToCrouch = true;
}

void ASCharacter::EndCrouch()
{
	UnCrouch();// �̰͵� �׳� �̰� �� ,�������� �����Ǿ������ϱ�
	bWantsToCrouch = false;
}


void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;
}

FVector ASCharacter::GetPawnViewLocation() const{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void ASCharacter::StartFire()
{
	if (CurrentWeapon)
		CurrentWeapon->StartFire();
}
void ASCharacter::StopFire()
{
	if (CurrentWeapon)
		CurrentWeapon->StopFire();
}

void ASCharacter::OnReload()
{
	if (CurrentWeapon) {
		CurrentWeapon->StartReload();
	

	}
}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HelthDelta, 
	const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	//Health�� �ٲ������ �Ͼ �ൿ��
	
	if (Health <= 0.0f && !bDied)
	{
		//Die
		bDied = true;
		//no longer movement
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Pawn�� �ı��Ѵ�
		DetachFromControllerPendingDestroy();
		//after 10 sec pawn will destory
		SetLifeSpan(10.0f);
	}

}
//AmmoCrate pickup ���� �� �����ϰ�
void ASCharacter::AddBullets()
{
	if (CurrentWeapon)
		CurrentWeapon->AddBullets();
}


void ASCharacter::ApplyRecoilToCharacter(float Value)
{
	float RangeValue=FMath::FRandRange(-Value, Value);
	//���� �̵�
	AddControllerPitchInput(-Value);
	//�� ��� �̵�
	AddControllerYawInput(RangeValue);
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//set zoom Interp
	float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;
	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);
	CameraComp->SetFieldOfView(NewFOV);


}



// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//Movement
	PlayerInputComponent->BindAxis("MoveForward",this,&ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);// ���콺 ���Ʒ� �̵�
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump); //�ϳ��� �����̴ϱ� �׼����� ���ε����ش�.

	//Weapon
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);
	
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("OnReload", IE_Pressed, this, &ASCharacter::OnReload); //�ϳ��� �����̴ϱ� �׼����� ���ε����ش�.
}


//���� ������ ����Ѵ�
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//���Ǿ��� Ŭ�� ����
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}