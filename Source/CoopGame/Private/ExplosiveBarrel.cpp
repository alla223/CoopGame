// Fill out your copyright notice in the Description page of Project Settings.

#include "ExplosiveBarrel.h"
#include "SHealthComponent.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CoopGame.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"
// Sets default values
AExplosiveBarrel::AExplosiveBarrel()
{
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	//이 클래스의 healthChanged함수와 바인드한다.

		HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged);
	

		MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
		//이 설정을 안하면 폭발시 다른 배럴들은 영향을 안받는다.
		MeshComp->SetSimulatePhysics(true);
		//radial component가 영향을 끼칠수 있게 physicsBody설정
		MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
		//MeshComp->SetCollisionResponseToChannels(ECC_GameTraceChannel1);//(ECC_GameTraceChannel1);
		RootComponent = MeshComp;

		//set collision

		RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
		RadialForceComp->SetupAttachment(RootComponent);
		RadialForceComp->Radius = 250.0f;
		RadialForceComp->bImpulseVelChange = true;
		//오직 트리거 될때만 Fireimpusle 발생 . 기본적으로는 true로 셋팅되어있다.
		RadialForceComp->bAutoActivate = false;
		//나 자신을 제외하고 적용한다.
		RadialForceComp->bIgnoreOwningActor = true;
		ExplosionImpulse = 400;;

		SetReplicates(true);
		//블락킹 할수 있을 것이고, 총알을 막을 수도있다.
		SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

void AExplosiveBarrel::OnHealthChanged(USHealthComponent * OwningHealthComp, float Health, float HelthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	// health 변화 했을 때  정의
	
	if (bExploded)
	{
		return;
	}
	if(Health <= 0.0f)
	{
			FVector TotalForce = FVector::UpVector*ExplosionImpulse;
			//Die
			bExploded = true;
			//서버에서 실행되고 bExploded바뀔때 클라에 복제
			OnRep_Exploded();

			//의미있는 게임플레이 요소다
			//boost barrel upwards , setmovementreplicate에 의해 움직일때 복제된다.
			MeshComp->AddImpulse(TotalForce, NAME_None, true);

			//이 코드 라인을 replicatemovement StaticMesh를 false로 설정하고 onRep_Exploded에 넣어도 여기에서 작동하는 것처럼 작동한다.
			//blast away nearby physics actors
			RadialForceComp->FireImpulse();

		}
	

}

void AExplosiveBarrel::OnRep_Exploded()
{
	//게임플레이 의미있는것들이 아니다 ->비쥬얼적 요소다
		//play FX and self material to black
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosiveEffect, GetActorLocation());

		MeshComp->SetMaterial(0, ExplodedMaterial);

	
}


// Called every frame
void AExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


//변수 복제를 명시한다
void AExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//조건없이 클라에 복제
	DOREPLIFETIME(AExplosiveBarrel, bExploded);
}