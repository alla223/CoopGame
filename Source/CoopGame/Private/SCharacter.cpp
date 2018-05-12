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
	SpringArmComp->bUsePawnControlRotation = true;// 카메라 회전
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));//컴파일러에게 카메라컴포 라는 타입을알려준다.
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	//crouch 서포트 : 엔진에서 우리가 크라우치를 허가한건지 체크한다. 아래 함수는 ai함수지만
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;
	//캡슐로하여금 weapon trace를 블락하지 못하게 무시하게 한다.
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
	
	//무기 스폰위해 , 서버에서 실행, 서버에서는 currentweapon 변수가 설정되지만 클라이언트에서 절대 설정되지않는다.
	if (Role == ROLE_Authority) { 

		//디폴트 무기 생성 - 블루프린트 변환코드 , 스폰할 클래스가 필요하다
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);//이 무기의 소유자
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
	Crouch();// 그냥 이게 끝이다..
	bWantsToCrouch = true;
}

void ASCharacter::EndCrouch()
{
	UnCrouch();// 이것도 그냥 이게 끝 ,엔진에서 구현되어있으니까
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
	//Health가 바뀌었을때 일어날 행동들
	
	if (Health <= 0.0f && !bDied)
	{
		//Die
		bDied = true;
		//no longer movement
		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//Pawn을 파괴한다
		DetachFromControllerPendingDestroy();
		//after 10 sec pawn will destory
		SetLifeSpan(10.0f);
	}

}
//AmmoCrate pickup 했을 때 증가하게
void ASCharacter::AddBullets()
{
	if (CurrentWeapon)
		CurrentWeapon->AddBullets();
}


void ASCharacter::ApplyRecoilToCharacter(float Value)
{
	float RangeValue=FMath::FRandRange(-Value, Value);
	//위로 이동
	AddControllerPitchInput(-Value);
	//좌 우로 이동
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

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);// 마우스 위아래 이동
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump); //하나의 동작이니까 액션으로 바인드해준다.

	//Weapon
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);
	
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("OnReload", IE_Pressed, this, &ASCharacter::OnReload); //하나의 동작이니까 액션으로 바인드해준다.
}


//변수 복제를 명시한다
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//조건없이 클라에 복제
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
}