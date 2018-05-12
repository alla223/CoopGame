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
FAutoConsoleVariableRef CVARDebugWeaponDrawing( //콘솔변수의 이름 정의
	TEXT("COOP.DebugWeapons"),
	DebugWeaponDrawing,
	TEXT("Draw Debug Lines for Weapons"),
	ECVF_Cheat);//콘솔에 숨어있으며 유저에 의해 값이 바뀌지않는다

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

	//SCharacter.cpp의 beginplay에서 호출할때 서버가 클라이언트에 복제를 허가하므로 시작할때 pawn이 이 actor(weapon)을 스폰할 수 있다.
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
	//world를 추적한다. 폰의 eye에서 크로스헤어위치로부터

	//웨폰을갖고 있는 오너, 누가 무기를 갖고있는지

	//클라에서 호출하고 서버에서 Fire() 실행한다
	if (Role < ROLE_Authority)
	{
		ServerFire();
		//return을 주석처리해서 아래코드를 클라에서도 실행한다

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


		FCollisionQueryParams QueryParams; //라인 트레이스에 사용되는 콜리전채널의 파라미터
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;//캐릭터의 캡슐같은경우 정확이 맞추는거를 보정해주는 것
		QueryParams.bReturnPhysicalMaterial = true; //Hit surface가 어떤 매터리얼을 가졌는지 확인하기위해 추가한 코드
		FVector TraceEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		if (CurrentBullets <= 0) { return; }
		CurrentBullets = CurrentBullets - 1;
		
		//구조체
		FHitResult Hit;
		//더 많은 컨트롤을 하고 우리의 커스텀콜리젼 채널을 생성하기 위해 ECC_GameTraceChannel1 을 사용
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, QueryParams)) { // hit 구조체, 시작점,엔드점, 콜리젼채널
		//Blocking hit! Process Damage
			AActor* HitActor = Hit.GetActor();//타격 대상
			//Hit 지점의 표면정보를 얻는다. 헤드샷 추가 데미지를 적용하기위해서 이 코드 아래에 ApplyPointDamage 코드가 실행된다.
			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			//수정하기위해서 새로운 변수 추가, 객체지향관점에서 더 적합한 건가
			float ActualDamage = BaseDamage;
			//피직스애셋 중에서 머리맞았을 때 추가 데미지 적용
			if (SurfaceType == SURFACE_FLESHVURNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);

			PlayImpactEffects(SurfaceType, Hit.ImpactPoint);
		

			//추적 엔드점이 hit의 가격점이다
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

		

			PlayFireEffects(TraceEndPoint);//추적끝점만 인자로 넘겨준다
			


	}
	//마지막 발사 했을때 월드의 시간을 얻는다 
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
	//타이머 설정
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots,this,&ASWeapon::Fire, TimeBetweenShots, true, FirstDelay); //매 1초마다 Fire함수를 호출

}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::StartReload()
{

	//Do we have ammo in the ammoPool? 검증
	if(AmmoPool<=0 || CurrentBullets>=MaxAmmo)
	{ return ; }
	
	PlayWeaponAnimation(ReloadAnim);

	//AmmoPool이 30개 미만 일 때
	if (AmmoPool < (MaxAmmo - CurrentBullets))
	{
		//10 15
		CurrentBullets = CurrentBullets + AmmoPool;
		AmmoPool = 0;
	}

	else
	{
		//10발 현재 탄창에 있으면 20발만 추가하면된다.
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
	//머즐 이펙트를 웨폰메시 소켓에 붙인다 FName : 머즐 소켓네임
	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}
	//추적이펙트 (연기효과)
	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		//리턴 해준다.
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter("Target", TraceEnd);
		}
	}
	//카메라 흔들림
	APawn* MyOwner = Cast<APawn>(GetOwner());
	if (MyOwner) {//유효성검사
		APlayerController* PC=Cast<APlayerController>(MyOwner->GetController());//컨트롤러 타입이지 아직 플레이어 컨트롤러는 아니다 . 따라서 APlayerController로 캐스팅해준다
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}

	}

}

void ASWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	//표면Hit의 선택점을 null로 우선두고 이후에 FleshEffect,DefaultImpact중에 결정한다
	UParticleSystem* SelectedEffect = nullptr;
	switch (SurfaceType)
	{
		//CoopGame.h에서 정의한 상수 사용, surfaceType1, surfaceType2 와 똑같이 동작한다
	case SURFACE_FLESHDEFAULT://머리 이외의 모든 부위
	case SURFACE_FLESHVURNERABLE://머리 
								 //위 2가지는 블러드 임팩트 적용
		SelectedEffect = FleshImpactEffect;
		break;
		//나머지는 일반적인 이펙트 적용
	default:
		SelectedEffect = DefaultImpactEffect;
		break;

	}
	if (SelectedEffect) {
		//슛 방향
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
	PlayFireEffects(HitScanTrace.TraceTo);//추적끝점만 인자로 넘겨준다

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


//변수 복제를 명시한다
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//조건없이 클라에 복제
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace,COND_SkipOwner);
}