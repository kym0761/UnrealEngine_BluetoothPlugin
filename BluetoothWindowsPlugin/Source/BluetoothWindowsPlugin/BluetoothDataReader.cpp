// Fill out your copyright notice in the Description page of Project Settings.


#include "BluetoothDataReader.h"
#include "BluetoothDataReceiver.h"

// Sets default values
ABluetoothDataReader::ABluetoothDataReader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DataReadInterval = 0.01f;
	TimerSet = 2.0f;
}

// Called when the game starts or when spawned
void ABluetoothDataReader::BeginPlay()
{
	Super::BeginPlay();

	DataReceiver = MakeShareable(new BluetoothDataReceiver());

	GetWorldTimerManager().SetTimer(
		BluetoothReadTimer, 
		this, 
		&ABluetoothDataReader::ReadData,
		DataReadInterval,
		true, 
		1.0f //First Delay
	);

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
			//CurrentRPM = 0.0f;
			if ((current_CT - Prev_CrankEventTimeStamp) != 0) //���� ����ð�-�����ð� ���� 0�� �Ǹ� ������ ���� ����
			{
				current = ((float)(current_CR - Prev_CrankRevolutions) / ((current_CT - Prev_CrankEventTimeStamp) / 1024.0f)) * 60.0f;
			}

			//����� �Ϸ�ƴٸ� ���� ���� prev���� �־���
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;

			//���� ���� 0 �̻��̸� �ٰ� �ִ� �Ǵ��Ͽ� runtimer�� Set��.
			if (current > 0.0f)
			{
				RunTimer = TimerSet;
			}


			if (RunTimer > 0.0f)
			{
				//Runtimer�� Ȱ��ȭ ���ε���, current�� 0�� �Ǿ��ٸ� CurrentRPM ���� �������Ȱ� ������.
				if (FMath::IsNearlyEqual(current, 0.0f))
				{
					//UE_LOG(LogTemp, Warning, TEXT("ignore"));

				}
				else if (current > 0.0f)//���� ���� ����� CurrentRPM�� �ٲ���.
				{
					CurrentRPM = current;
				}
				else//�ð��� uint16 ���� �ʰ��ϸ� 0���� ���ư��鼭 �������� ���̳ʽ��� �Ǵ� �ʱ�ȭ
				{
					CurrentRPM = 0.0f;

					Prev_WheelRevolutions = 0;
					Prev_WheelEventTimeStamp = 0;
					Prev_CrankRevolutions = 0;
					Prev_CrankEventTimeStamp = 0;

				}
			}
			else if (RunTimer <= 0.0f) //runtimer�� ������ �ƴٸ� currentRPM�� 0���� �����ϰ� interpolate�ϵ��� ��.
			{
				//UE_LOG(LogTemp, Warning, TEXT("runner is not ok"));

				CurrentRPM = 0.0f;
			}

			//RPM�� CurrentRPM�� interpolate��.
			//UE_LOG(LogTemp, Warning, TEXT("interpolate"));

			if (current > 0.0f && RunTimer > 0.0f) // �⺻ interpolate
			{
				RPM = FMath::FInterpTo(RPM, CurrentRPM, DataReadInterval, 2.0f);
			}
			else if (FMath::IsNearlyEqual(current, 0.0f) && RunTimer > 0.0f) // runtimer�� reset�� ���� �ʾ����� currentRPM�� 0�� �ƴٸ� ���� ��¦�� 0���� ���δ�.
			{
				RPM = FMath::FInterpTo(RPM, CurrentRPM, DataReadInterval, 0.1f);
			}
			else //runtimer�� reset�Ǹ� ������ 0���� ���δ�.
			{
				RPM = FMath::FInterpTo(RPM, CurrentRPM, DataReadInterval, 0.5f);
			}
			

		}

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DataReceiver is not Valid.."));
	}

}

