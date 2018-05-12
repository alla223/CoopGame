// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopGame.h"
#include "SCharacter.h"
#include "Public/TimerManager.h"

#include "Net/UnrealNetwork.h"
// Sets default values



static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing( //�ֺܼ����� �̸� ����
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);//�ֿܼ� ���������� ������ ���� ���� �ٲ����ʴ´�

ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TargetName = "Target";
	BaseDamage = 20.0f;
	RateOfFire = 600;
	BulletsPerMag = 30;
	AmmoPool = 30;
	CurrentBullets = 30;
	AmmoCrateBulletsCount = 30;
	MaxRecoilAmount = 0.2f;
	MaxAmmo = 30;

	//SCharacter.cpp�� beginplay���� ȣ���Ҷ� ������ Ŭ���̾�Ʈ�� ������ �㰡�ϹǷ� �����Ҷ� pawn�� �� actor(weapon)�� ������ �� �ִ�.
	SetReplicates(true);
	
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60/ RateOfFire ;
}



void ASWeapon::Fire()
{
	//world�� �����Ѵ�. ���� eye���� ũ�ν������ġ�κ���

	//���������� �ִ� ����, ���� ���⸦ �����ִ���

	//Ŭ�󿡼� ȣ���ϰ� �������� Fire() �����Ѵ�
	if (Role < ROLE_Authority)
	{
		ServerFire();
		//return�� �ּ�ó���ؼ� �Ʒ��ڵ带 Ŭ�󿡼��� �����Ѵ�

		//return;
	}

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		FVector EyeLocation;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000);


		FCollisionQueryParams QueryParams; //���� Ʈ���̽��� ���Ǵ� �ݸ���ä���� �Ķ����
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;//ĳ������ ĸ��������� ��Ȯ�� ���ߴ°Ÿ� �������ִ� ��
		QueryParams.bReturnPhysicalMaterial = true; //Hit surface�� � ���͸����� �������� Ȯ���ϱ����� �߰��� �ڵ�
		FVector TraceEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		if (CurrentBullets <= 0) { return; }
		CurrentBullets = CurrentBullets - 1;
		
		//����ü
		FHitResult Hit;
		//�� ���� ��Ʈ���� �ϰ� �츮�� Ŀ�����ݸ��� ä���� �����ϱ� ���� ECC_GameTraceChannel1 �� ���
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams)) { // hit ����ü, ������,������, �ݸ���ä��
		//Blocking hit! Process Damage
			AActor* HitActor = Hit.GetActor();//Ÿ�� ���
			//Hit ������ ǥ�������� ��´�. ��弦 �߰� �������� �����ϱ����ؼ� �� �ڵ� �Ʒ��� ApplyPointDamage �ڵ尡 ����ȴ�.
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			//�����ϱ����ؼ� ���ο� ���� �߰�, ��ü����������� �� ������ �ǰ�
			float ActualDamage = BaseDamage;
			//�������ּ� �߿��� �Ӹ��¾��� �� �߰� ������ ����
			if (SurfaceType == SURFACE_FLESHVURNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
		

			//���� �������� hit�� �������̴�
			TraceEndPoint = Hit.ImpactPoint;
			
			
		}
			AddRecoil();

			if (Role == ROLE_Authority)
			{
				HitScanTrace.TraceTo = TraceEndPoint;
				HitScanTrace.SurfaceType = SurfaceType;
			}

			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
			}

		

			PlayFireEffects(TraceEndPoint);//���������� ���ڷ� �Ѱ��ش�
			


	}
	//������ �߻� ������ ������ �ð��� ��´� 
	LastFireTime = GetWorld()->TimeSeconds;

}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

void ASWeapon::StartFire()
{	//TimeBetweenShots:0.1
	float FirstDelay =FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f);
	//Ÿ�̸� ����
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots,this,&ASWeapon::Fire, TimeBetweenShots, true, FirstDelay); //�� 1�ʸ��� Fire�Լ��� ȣ��

}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::StartReload()
{

	//Do we have ammo in the ammoPool? ����
	if(AmmoPool<=0 || CurrentBullets>=MaxAmmo)
	{ return ; }
	
	PlayWeaponAnimation(ReloadAnim);

	//AmmoPool�� 30�� �̸� �� ��
	if (AmmoPool < (MaxAmmo - CurrentBullets))
	{
		//10 15
		CurrentBullets = CurrentBullets + AmmoPool;
		AmmoPool = 0;
	}

	else
	{
		//10�� ���� źâ�� ������ 20�߸� �߰��ϸ�ȴ�.
		AmmoPool = AmmoPool - (MaxAmmo - CurrentBullets);
		CurrentBullets = MaxAmmo;
	}
	
}

float ASWeapon::PlayWeaponAnimation(UAnimMontage* Pawn3P)
{
	float Duration = 0.0f;
	
	if (MyOwner)
	{
		UAnimMontage* UseAnim = Pawn3P;
		if (UseAnim)
		{
			Duration = MyOwner->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}


void ASWeapon::AddBullets()
{
	

	AmmoPool = AmmoPool + AmmoCrateBulletsCount;
}

void ASWeapon::PlayFireEffects(FVector TraceEnd)
{
	//���� ����Ʈ�� �����޽� ���Ͽ� ���δ� FName : ���� ���ϳ���
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
	//��������Ʈ (����ȿ��)
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		//���� ���ش�.
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter("Target", TraceEnd);
		}
	}
	//ī�޶� ��鸲
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {//��ȿ���˻�
		APlayerController* PC=Cast<APlayerController>(MyOwner->GetController());//��Ʈ�ѷ� Ÿ������ ���� �÷��̾� ��Ʈ�ѷ��� �ƴϴ� . ���� APlayerController�� ĳ�������ش�
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}

	}

}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	//ǥ��Hit�� �������� null�� �켱�ΰ� ���Ŀ� FleshEffect,DefaultImpact�߿� �����Ѵ�
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
		//CoopGame.h���� ������ ��� ���, surfaceType1, surfaceType2 �� �Ȱ��� �����Ѵ�
	case SURFACE_FLESHDEFAULT://�Ӹ� �̿��� ��� ����
	case SURFACE_FLESHVURNERABLE://�Ӹ� 
								 //�� 2������ ���� ����Ʈ ����
		SelectedEffect = FleshImpactEffect;
		break;
		//�������� �Ϲ����� ����Ʈ ����
	default:
		SelectedEffect = DefaultImpactEffect;
		break;

	}
	if (SelectedEffect) {
		//�� ����
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());
	}
}


void ASWeapon::AddRecoil()
{
	ASCharacter* MyOwner = Cast<ASCharacter>(GetOwner());
	if (MyOwner == NULL) { return; }
	MyOwner->ApplyRecoilToCharacter(MaxRecoilAmount);
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceTo);//���������� ���ڷ� �Ѱ��ش�

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


//���� ������ ����Ѵ�
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//���Ǿ��� Ŭ�� ����
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace,COND_SkipOwner);
}