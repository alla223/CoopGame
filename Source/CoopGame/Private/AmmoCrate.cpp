// Fill out your copyright notice in the Description page of Project Settings.

#include "AmmoCrate.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SCharacter.h"
#include "CoopGame.h"
#include "GameFramework/Actor.h"
// Sets default values
AAmmoCrate::AAmmoCrate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	//set collision
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = MeshComp;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	
	//set collision
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore); //플레이어 채널
	SphereComp->SetupAttachment(MeshComp);



}

// Called when the game starts or when spawned
void AAmmoCrate::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAmmoCrate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AAmmoCrate::PlayEffect()
{

	UGameplayStatics::SpawnEmitterAtLocation(this, PickupFX, GetActorLocation());
}
//액터가오버랩됨
void AAmmoCrate::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);
	//overlap되었다면 이펙트재생
	PlayEffect();

		ASCharacter* MyCharacter = Cast<ASCharacter>(OtherActor);
		if (MyCharacter)
		{
			MyCharacter->AddBullets();
			this->Destroy();
		}

	

}
