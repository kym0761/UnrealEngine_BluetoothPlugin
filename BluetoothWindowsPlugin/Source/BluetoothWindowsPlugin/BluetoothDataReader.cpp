// Fill out your copyright notice in the Description page of Project Settings.


#include "BluetoothDataReader.h"
#include "BluetoothDataReceiver.h"

// Sets default values
ABluetoothDataReader::ABluetoothDataReader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABluetoothDataReader::BeginPlay()
{
	Super::BeginPlay();
	
	DataReceiver = new BluetoothDataReceiver();
	
	if (DataReceiver)
	{
		GetWorldTimerManager().SetTimer(BluetoothReadTimer, this, &ABluetoothDataReader::ReadData, 1.0f, true, 1.0f);
	}

}

void ABluetoothDataReader::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	//free Tester
	if (DataReceiver)
	{
		GetWorldTimerManager().ClearTimer(BluetoothReadTimer);

		DataReceiver->EnsureCompletion();
		delete DataReceiver;
		DataReceiver = nullptr;
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
	if (DataReceiver)
	{
		int32 a = 0;
		int32 b = 0;
		int32 c = 0;
		int32 d = 0;
		DataReceiver->GetCadenceData(a, b, c, d);


		//시작할 때는 데이터 값만 가져오고 끝낸다.
		if (Prev_WheelRevolutions == 0 && Prev_WheelEventTimeStamp == 0 && Prev_CrankRevolutions == 0 && Prev_CrankEventTimeStamp == 0)
		{
			Prev_WheelRevolutions = a;
			Prev_WheelEventTimeStamp = b;
			Prev_CrankRevolutions = c;
			Prev_CrankEventTimeStamp = d;

			return;
		}
		else
		{
			//케이던스 센서 데이터로 RPM 계산법을 모른다. 그래서 임시적으로 적절한 값을 만들도록 해놨으니
			//나중에 정확한 공식을 알면 꼭 고쳐야한다.
			if (Prev_WheelRevolutions != a && Prev_WheelEventTimeStamp != b)
			{
				RPM = (a - Prev_WheelRevolutions) / ((b - Prev_WheelEventTimeStamp) / 1024.0f) * 60.0f;
			}
			else
			{
				//값이 변하지 않았다면, 움직이지 않았다는 뜻이니
				//RPM값을 줄여야한다.
				RPM = FMath::Lerp(0.0f, RPM, 0.25f);
			}

			Prev_WheelRevolutions = a;
			Prev_WheelEventTimeStamp = b;
			Prev_CrankRevolutions = c;
			Prev_CrankEventTimeStamp = d;
		}

	}
	else
	{
		RPM = 0;
	}
}

