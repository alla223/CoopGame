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
			//�� ������Ʈ�� ����Լ��� handleTakeAnyDamge�Լ��� ���ε��Ѵ�, ��������Ʈ�� �����θ� ����ϴ°��� ���� , �����Ҵ��̱⶧����, 
			//�̰����� ȣ���� ������ ������Ʈ�� ������ ���� ���Ѵ�.
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

//���� ������ ����Ѵ�
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//���Ǿ��� Ŭ�� ����
	DOREPLIFETIME(USHealthComponent, Health);
}