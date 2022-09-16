// Fill out your copyright notice in the Description page of Project Settings.


#include "BluetoothDataReader.h"
#include "BluetoothDataReceiver.h"

// Sets default values
ABluetoothDataReader::ABluetoothDataReader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DataReadInterval = 0.01f;
}

// Called when the game starts or when spawned
void ABluetoothDataReader::BeginPlay()
{
	Super::BeginPlay();

	DataReceiver = MakeShareable(new BluetoothDataReceiver());

	GetWorldTimerManager().SetTimer(BluetoothReadTimer, this, &ABluetoothDataReader::ReadData, DataReadInterval, true, 1.0f);

}

void ABluetoothDataReader::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DataReceiver.IsValid())
	{
		DataReceiver->EnsureCompletion();
	}


	Super::EndPlay(EndPlayReason);

}

// Called every frame
void ABluetoothDataReader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABluetoothDataReader::ReadData()
{
	if (DataReceiver.IsValid())
	{
		RunTimer -= DataReadInterval;

		int32 current_WR = 0;
		int32 current_WT = 0;
		int32 current_CR = 0;
		int32 current_CT = 0;

		DataReceiver->GetCadenceData(current_WR, current_WT, current_CR, current_CT);

		//ó�� ������ �� ���� �ʱ�ȭ.
		if (Prev_WheelRevolutions == 0 && Prev_WheelEventTimeStamp == 0 && Prev_CrankRevolutions == 0 && Prev_CrankEventTimeStamp == 0)
		{
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;

			return;
		}
		else
		{

			float current = 0.0f;
			if ((current_CT - Prev_CrankEventTimeStamp) != 0) //�ð� ���� 0�� �Ǹ� ������ ���� ����
			{
				current = (float)(current_CR - Prev_CrankRevolutions) / ((float)(current_CT - Prev_CrankEventTimeStamp) / 1024.0f) * 60.0f;
			}

			//���� ���� prev���� �־���
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;


			//���� ���� ����� runtimer�� 2�� ����
			if (current > 0.0f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("runner set"));
				RunTimer = 1.5f;
			}

			//�ð��� uint16 ���� �ʰ��ϸ� �������� ���̳ʽ��� �Ǵ� �ʱ�ȭ
			if (current < 0.0f)
			{
				current = 0.0f;

				Prev_WheelRevolutions = 0;
				Prev_WheelEventTimeStamp = 0;
				Prev_CrankRevolutions = 0;
				Prev_CrankEventTimeStamp = 0;

				return;
			}
			
			//���� ������ �ʾ����Ƿ� ������.
			if (RunTimer > 0.0f && FMath::IsNearlyEqual(current, 0.0f))
			{
				//UE_LOG(LogTemp, Warning, TEXT("ignore"));
				return;
			}

			//runtimer�� ������ �ƴٸ� current�� 0���� �����ϰ� 0���� interpolate�ϵ��� ��.
			if (RunTimer <= 0.0f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("runner is not ok"));
				current = 0.0f;
			}

			//RPM�� Current�� interpolate��.
			RPM = FMath::FInterpTo(RPM, current, DataReadInterval, 0.75f);
			//UE_LOG(LogTemp, Warning, TEXT("interpolate"));


		}

	}

}

