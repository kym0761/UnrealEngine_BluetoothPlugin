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

		//처음 시작할 때 최초 초기화.
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
			if ((current_CT - Prev_CrankEventTimeStamp) != 0) //만약 현재시간-이전시간 값이 0이 되면 나누기 연산 실패
			{
				current = ((float)(current_CR - Prev_CrankRevolutions) / ((current_CT - Prev_CrankEventTimeStamp) / 1024.0f)) * 60.0f;
			}

			//계산이 완료됐다면 현재 값을 prev값에 넣어줌
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;

			//계산된 값이 0 이상이면 뛰고 있다 판단하여 runtimer를 Set함.
			if (current > 0.0f)
			{
				RunTimer = TimerSet;
			}


			if (RunTimer > 0.0f)
			{
				//Runtimer가 활성화 중인데도, current가 0이 되었다면 CurrentRPM 값이 변하지안게 무시함.
				if (FMath::IsNearlyEqual(current, 0.0f))
				{
					//UE_LOG(LogTemp, Warning, TEXT("ignore"));

				}
				else if (current > 0.0f)//계산된 값이 양수면 CurrentRPM을 바꿔줌.
				{
					CurrentRPM = current;
				}
				else//시간이 uint16 값을 초과하면 0으로 돌아가면서 측정값이 마이너스가 되니 초기화
				{
					CurrentRPM = 0.0f;

					Prev_WheelRevolutions = 0;
					Prev_WheelEventTimeStamp = 0;
					Prev_CrankRevolutions = 0;
					Prev_CrankEventTimeStamp = 0;

				}
			}
			else if (RunTimer <= 0.0f) //runtimer가 음수가 됐다면 currentRPM을 0으로 세팅하고 interpolate하도록 함.
			{
				//UE_LOG(LogTemp, Warning, TEXT("runner is not ok"));

				CurrentRPM = 0.0f;
			}

			//RPM을 CurrentRPM에 interpolate함.
			//UE_LOG(LogTemp, Warning, TEXT("interpolate"));

			if (current > 0.0f && RunTimer > 0.0f) // 기본 interpolate
			{
				RPM = FMath::FInterpTo(RPM, CurrentRPM, DataReadInterval, 2.0f);
			}
			else if (FMath::IsNearlyEqual(current, 0.0f) && RunTimer > 0.0f) // runtimer가 reset이 되지 않았지만 currentRPM이 0이 됐다면 아주 살짝씩 0으로 줄인다.
			{
				RPM = FMath::FInterpTo(RPM, CurrentRPM, DataReadInterval, 0.1f);
			}
			else //runtimer가 reset되면 빠르게 0으로 줄인다.
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

