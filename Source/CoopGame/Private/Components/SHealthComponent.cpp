// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{

	DefaultHealth = 100.0f;

	SetIsReplicated(true);

}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			//이 오브젝트의 멤버함수인 handleTakeAnyDamge함수와 바인드한다, 델리게이트는 참조로만 사용하는것이 좋다 , 동적할당이기때문에, 
			//이곳에서 호출을 하지만 오브젝트의 유형을 알지 못한다.
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}
	Health = DefaultHealth;
}

//Delegate
void USHealthComponent::HandleTakeAnyDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
	if (Damage <= 0.0f)
	{
		return;
	}
	//update health clamp
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	UE_LOG(LogTemp, Log, TEXT("Health Changed %s"), *FString::SanitizeFloat(Health));
}

//변수 복제를 명시한다
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//조건없이 클라에 복제
	DOREPLIFETIME(USHealthComponent, Health);
}