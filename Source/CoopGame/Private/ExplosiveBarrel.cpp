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
	//�� Ŭ������ healthChanged�Լ��� ���ε��Ѵ�.

		HealthComp->OnHealthChanged.AddDynamic(this, &AExplosiveBarrel::OnHealthChanged);
	

		MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
		//�� ������ ���ϸ� ���߽� �ٸ� �跲���� ������ �ȹ޴´�.
		MeshComp->SetSimulatePhysics(true);
		//radial component�� ������ ��ĥ�� �ְ� physicsBody����
		MeshComp->SetCollisionObjectType(ECC_PhysicsBody);
		//MeshComp->SetCollisionResponseToChannels(ECC_GameTraceChannel1);//(ECC_GameTraceChannel1);
		RootComponent = MeshComp;

		//set collision

		RadialForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
		RadialForceComp->SetupAttachment(RootComponent);
		RadialForceComp->Radius = 250.0f;
		RadialForceComp->bImpulseVelChange = true;
		//���� Ʈ���� �ɶ��� Fireimpusle �߻� . �⺻�����δ� true�� ���õǾ��ִ�.
		RadialForceComp->bAutoActivate = false;
		//�� �ڽ��� �����ϰ� �����Ѵ�.
		RadialForceComp->bIgnoreOwningActor = true;
		ExplosionImpulse = 400;;

		SetReplicates(true);
		//���ŷ �Ҽ� ���� ���̰�, �Ѿ��� ���� �����ִ�.
		SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

void AExplosiveBarrel::OnHealthChanged(USHealthComponent * OwningHealthComp, float Health, float HelthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	// health ��ȭ ���� ��  ����
	
	if (bExploded)
	{
		return;
	}
	if(Health <= 0.0f)
	{
			FVector TotalForce = FVector::UpVector*ExplosionImpulse;
			//Die
			bExploded = true;
			//�������� ����ǰ� bExploded�ٲ� Ŭ�� ����
			OnRep_Exploded();

			//�ǹ��ִ� �����÷��� ��Ҵ�
			//boost barrel upwards , setmovementreplicate�� ���� �����϶� �����ȴ�.
			MeshComp->AddImpulse(TotalForce, NAME_None, true);

			//�� �ڵ� ������ replicatemovement StaticMesh�� false�� �����ϰ� onRep_Exploded�� �־ ���⿡�� �۵��ϴ� ��ó�� �۵��Ѵ�.
			//blast away nearby physics actors
			RadialForceComp->FireImpulse();

		}
	

}

void AExplosiveBarrel::OnRep_Exploded()
{
	//�����÷��� �ǹ��ִ°͵��� �ƴϴ� ->������� ��Ҵ�
		//play FX and self material to black
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosiveEffect, GetActorLocation());

		MeshComp->SetMaterial(0, ExplodedMaterial);

	
}


// Called every frame
void AExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


//���� ������ ����Ѵ�
void AExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//���Ǿ��� Ŭ�� ����
	DOREPLIFETIME(AExplosiveBarrel, bExploded);
}