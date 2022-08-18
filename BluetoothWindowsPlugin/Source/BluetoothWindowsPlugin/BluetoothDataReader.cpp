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
	
	RPM_Data.Init(0, RecordBufferSize);

	DataReceiver = MakeShareable(new BluetoothDataReceiver());

	GetWorldTimerManager().SetTimer(BluetoothReadTimer, this, &ABluetoothDataReader::ReadData, 0.75f, true, 1.0f);

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
		int32 current_WR = 0;
		int32 current_WT = 0;
		int32 current_CR = 0;
		int32 current_CT = 0;

		DataReceiver->GetCadenceData(current_WR, current_WT, current_CR, current_CT);

		//시작할 때는 데이터 값만 가져오고 끝낸다.
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
			//WheelData에 현재 기준 RPM을 기록하고, WheelData 평균을 실제 RPM이라고 추정한다.
			//Bluetooth의 데이터 입력 주기를 조절할 수 없고
			//너무 빨리 측정하면 current_CrankRevo - Prev_CrankRevo 의 값이 0이 되어 RPM이 순간 0이 되므로
			//약 10개의 측정 값의 평균을 매겨 RPM을 추정하는 것이 바람직하다고 생각함.

			float current = 0;
			if ((current_CT - Prev_CrankEventTimeStamp) != 0) //시간 값이 0이 되면 나누기 연산 실패
			{
				current = (current_CR - Prev_CrankRevolutions) / ((current_CT - Prev_CrankEventTimeStamp) / 1024.0f) * 60;
			}
			
			//시간 uint16 값을 초과하면 0이 되어 측정값이 마이너스가 되는데 0값이 들어가는 것이 낫다고 판단.
			if (current < 0.0f)
			{
				current = 0.0f;
			}
				
			if (RPM_Data.Num() < RecordBufferSize)
			{
				RPM_Data.Add(current);
			}
			else
			{
				RPM_Data.Add(current);
				while (RPM_Data.Num() > RecordBufferSize)
				{
					RPM_Data.RemoveAt(0);
				}
			}

			//평균값 계산 후, 실제 RPM에 평균값을 대입함.
			int32 num = RPM_Data.Num();
			float total = 0.0f;
			for (int i = 0; i < num; i++)
			{
				total += RPM_Data[i];
			}

			RPM = total / num;
			
			//계산기 끝나면 현재 값을 prev값에 넣어주고 다음 계산을 함.
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;
		}

	}

}

