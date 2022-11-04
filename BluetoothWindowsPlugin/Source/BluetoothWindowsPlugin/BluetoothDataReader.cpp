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
	//ReadData()는 DataReadInterval에 설정된 0.01f로 인해 0.01초마다 실행됩니다.

	if (DataReceiver.IsValid())
	{
		RunTimer -= DataReadInterval;

		int32 current_WR = 0;
		int32 current_WT = 0;
		int32 current_CR = 0;
		int32 current_CT = 0;

		DataReceiver->GetCadenceData(current_WR, current_WT, current_CR, current_CT);

		//처음 시작할 때 최초 초기화.
		if (Prev_WheelRevolutions == 0.0f && Prev_WheelEventTimeStamp == 0.0f && Prev_CrankRevolutions == 0.0f && Prev_CrankEventTimeStamp == 0.0f)
		{

			//WheelRev, WheelTimeStamp은 쓰지 않지만 기록은 해놓고 있는 상태.

			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;

			return;
		}
		else
		{
			//RPM 계산 
			float currentRPM = 0.0f;
			float div = current_CT - Prev_CrankEventTimeStamp;
			if (div != 0.0f) //만약 현재시간-이전시간 값이 0이 되면 나누기 연산 실패
			{
				currentRPM = ((float)(current_CR - Prev_CrankRevolutions) / (div / 1024.0f)) * 60.0f;
			}

			//speed km/h == 거리 / 초 * 0.036f
			float currentBikeSpeed = 0.0f;
			int rounds = current_CR - Prev_CrankRevolutions;
			float pi = 3.1415926535f;

			if (div != 0.0f)
			{
				currentBikeSpeed = rounds * Diameter * pi / (div / 1024.0f) * 0.036f;
			}

			//계산이 완료됐다면 현재 값을 prev값에 넣어줌
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;

			//계산된 값이 0을 넘는다면 뛰고 있다 판단하여 runtimer를 Set함. TimerSet이 2초이므로, 2초로 Set됨.
			if (currentRPM > 0.0f)
			{
				RunTimer = TimerSet;
			}

			if (RunTimer > 0.0f)
			{
				//Runtimer가 활성화 중인데도, current가 0이 되었다면 TargetRPM 값이 변하지 않게 무시함.
				if (FMath::IsNearlyEqual(currentRPM, 0.0f))
				{
					//UE_LOG(LogTemp, Warning, TEXT("ignore"));
				}
				else if (currentRPM > 0.0f)				//계산된 값이 양수면 TargetRPM을 바꿔줌. CurrentRPM이 0을 초과하면 당연히 CurrentBikeSpeed도 0을 초과함.
				{
					TargetRPM = currentRPM;
					TargetBikeSpeed = currentBikeSpeed;
				}
				else									//시간이 uint16 값을 초과하면 0으로 돌아가면서 측정값이 마이너스가 되니 초기화
				{
					TargetRPM = 0.0f;
					TargetBikeSpeed = 0.0f;

					Prev_WheelRevolutions = 0;
					Prev_WheelEventTimeStamp = 0;
					Prev_CrankRevolutions = 0;
					Prev_CrankEventTimeStamp = 0;
				}
			}
			else if (RunTimer <= 0.0f) //runtimer가 음수가 됐다면 currentRPM을 0으로 세팅하고 interpolate하도록 함.
			{
				TargetRPM = 0.0f;
				TargetBikeSpeed = 0.0f;
			}

			//RPM을 TargetRPM에 interpolate함.
			if (TargetRPM > 0.0f && RunTimer > 0.0f) // 기본 interpolate
			{
				RPM = FMath::FInterpTo(RPM, TargetRPM, DataReadInterval, 5.0f);
				BikeSpeed = FMath::FInterpTo(BikeSpeed, TargetBikeSpeed, DataReadInterval, 5.0f);
			}
			else if (FMath::IsNearlyEqual(TargetRPM, 0.0f) && RunTimer > 0.0f) // runtimer가 reset이 되지 않았지만 currentRPM이 0이 됐다면 아주 살짝씩 0으로 줄인다.
			{
				RPM = FMath::FInterpTo(RPM, TargetRPM, DataReadInterval, 0.1f);
				BikeSpeed = FMath::FInterpTo(BikeSpeed, TargetBikeSpeed, DataReadInterval, 0.1f);
			}
			else //runtimer가 reset되면 빠르게 0으로 줄인다. target도 0일 것이다.
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

