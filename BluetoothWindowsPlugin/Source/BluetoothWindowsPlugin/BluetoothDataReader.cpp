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
			if ((current_CT - Prev_CrankEventTimeStamp) != 0) //시간 값이 0이 되면 나누기 연산 실패
			{
				current = (float)(current_CR - Prev_CrankRevolutions) / ((float)(current_CT - Prev_CrankEventTimeStamp) / 1024.0f) * 60.0f;
			}

			//현재 값을 prev값에 넣어줌
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;


			//계산된 값이 양수면 runtimer를 2로 세팅
			if (current > 0.0f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("runner set"));
				RunTimer = 1.5f;
			}

			//시간이 uint16 값을 초과하면 측정값이 마이너스가 되니 초기화
			if (current < 0.0f)
			{
				current = 0.0f;

				Prev_WheelRevolutions = 0;
				Prev_WheelEventTimeStamp = 0;
				Prev_CrankRevolutions = 0;
				Prev_CrankEventTimeStamp = 0;

				return;
			}
			
			//값이 변하지 않았으므로 무시함.
			if (RunTimer > 0.0f && FMath::IsNearlyEqual(current, 0.0f))
			{
				//UE_LOG(LogTemp, Warning, TEXT("ignore"));
				return;
			}

			//runtimer가 음수가 됐다면 current를 0으로 세팅하고 0으로 interpolate하도록 함.
			if (RunTimer <= 0.0f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("runner is not ok"));
				current = 0.0f;
			}

			//RPM을 Current에 interpolate함.
			RPM = FMath::FInterpTo(RPM, current, DataReadInterval, 0.75f);
			//UE_LOG(LogTemp, Warning, TEXT("interpolate"));


		}

	}

}

