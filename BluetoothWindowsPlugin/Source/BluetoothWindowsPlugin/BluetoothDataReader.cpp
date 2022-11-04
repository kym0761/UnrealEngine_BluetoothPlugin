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

	DataReceiver = MakeShareable(new FBluetoothDataReceiver());

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
	//ReadData()�� DataReadInterval�� ������ 0.01f�� ���� 0.01�ʸ��� ����˴ϴ�.

	if (DataReceiver.IsValid())
	{
		RunTimer -= DataReadInterval;

		int32 current_WR = 0;
		int32 current_WT = 0;
		int32 current_CR = 0;
		int32 current_CT = 0;

		DataReceiver->GetCadenceData(current_WR, current_WT, current_CR, current_CT);

		//ó�� ������ �� ���� �ʱ�ȭ.
		if (Prev_WheelRevolutions == 0.0f && Prev_WheelEventTimeStamp == 0.0f && Prev_CrankRevolutions == 0.0f && Prev_CrankEventTimeStamp == 0.0f)
		{

			//WheelRev, WheelTimeStamp�� ���� ������ ����� �س��� �ִ� ����.

			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;

			return;
		}
		else
		{
			//RPM ��� 
			float currentRPM = 0.0f;
			float div = current_CT - Prev_CrankEventTimeStamp;
			if (div != 0.0f) //���� ����ð�-�����ð� ���� 0�� �Ǹ� ������ ���� ����
			{
				currentRPM = ((float)(current_CR - Prev_CrankRevolutions) / (div / 1024.0f)) * 60.0f;
			}

			//speed km/h == �Ÿ� / �� * 0.036f
			float currentBikeSpeed = 0.0f;
			int rounds = current_CR - Prev_CrankRevolutions;
			float pi = 3.1415926535f;

			if (div != 0.0f)
			{
				currentBikeSpeed = rounds * Diameter * pi / (div / 1024.0f) * 0.036f;
			}

			//����� �Ϸ�ƴٸ� ���� ���� prev���� �־���
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;

			//���� ���� 0�� �Ѵ´ٸ� �ٰ� �ִ� �Ǵ��Ͽ� runtimer�� Set��. TimerSet�� 2���̹Ƿ�, 2�ʷ� Set��.
			if (currentRPM > 0.0f)
			{
				RunTimer = TimerSet;
			}

			if (RunTimer > 0.0f)
			{
				//Runtimer�� Ȱ��ȭ ���ε���, current�� 0�� �Ǿ��ٸ� TargetRPM ���� ������ �ʰ� ������.
				if (FMath::IsNearlyEqual(currentRPM, 0.0f))
				{
					//UE_LOG(LogTemp, Warning, TEXT("ignore"));
				}
				else if (currentRPM > 0.0f)				//���� ���� ����� TargetRPM�� �ٲ���. CurrentRPM�� 0�� �ʰ��ϸ� �翬�� CurrentBikeSpeed�� 0�� �ʰ���.
				{
					TargetRPM = currentRPM;
					TargetBikeSpeed = currentBikeSpeed;
				}
				else									//�ð��� uint16 ���� �ʰ��ϸ� 0���� ���ư��鼭 �������� ���̳ʽ��� �Ǵ� �ʱ�ȭ
				{
					TargetRPM = 0.0f;
					TargetBikeSpeed = 0.0f;

					Prev_WheelRevolutions = 0;
					Prev_WheelEventTimeStamp = 0;
					Prev_CrankRevolutions = 0;
					Prev_CrankEventTimeStamp = 0;
				}
			}
			else if (RunTimer <= 0.0f) //runtimer�� ������ �ƴٸ� currentRPM�� 0���� �����ϰ� interpolate�ϵ��� ��.
			{
				TargetRPM = 0.0f;
				TargetBikeSpeed = 0.0f;
			}

			//RPM�� TargetRPM�� interpolate��.
			if (TargetRPM > 0.0f && RunTimer > 0.0f) // �⺻ interpolate
			{
				RPM = FMath::FInterpTo(RPM, TargetRPM, DataReadInterval, 5.0f);
				BikeSpeed = FMath::FInterpTo(BikeSpeed, TargetBikeSpeed, DataReadInterval, 5.0f);
			}
			else if (FMath::IsNearlyEqual(TargetRPM, 0.0f) && RunTimer > 0.0f) // runtimer�� reset�� ���� �ʾ����� currentRPM�� 0�� �ƴٸ� ���� ��¦�� 0���� ���δ�.
			{
				RPM = FMath::FInterpTo(RPM, TargetRPM, DataReadInterval, 0.1f);
				BikeSpeed = FMath::FInterpTo(BikeSpeed, TargetBikeSpeed, DataReadInterval, 0.1f);
			}
			else //runtimer�� reset�Ǹ� ������ 0���� ���δ�. target�� 0�� ���̴�.
			{
				RPM = FMath::FInterpTo(RPM, TargetRPM, DataReadInterval, 0.5f);
				BikeSpeed = FMath::FInterpTo(BikeSpeed, TargetBikeSpeed, DataReadInterval, 0.5f);
			}



		}

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DataReceiver is not Valid.."));
	}

}

