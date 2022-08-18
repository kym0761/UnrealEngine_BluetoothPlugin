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

		//������ ���� ������ ���� �������� ������.
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
			//WheelData�� ���� ���� RPM�� ����ϰ�, WheelData ����� ���� RPM�̶�� �����Ѵ�.
			//Bluetooth�� ������ �Է� �ֱ⸦ ������ �� ����
			//�ʹ� ���� �����ϸ� current_CrankRevo - Prev_CrankRevo �� ���� 0�� �Ǿ� RPM�� ���� 0�� �ǹǷ�
			//�� 10���� ���� ���� ����� �Ű� RPM�� �����ϴ� ���� �ٶ����ϴٰ� ������.

			float current = 0;
			if ((current_CT - Prev_CrankEventTimeStamp) != 0) //�ð� ���� 0�� �Ǹ� ������ ���� ����
			{
				current = (current_CR - Prev_CrankRevolutions) / ((current_CT - Prev_CrankEventTimeStamp) / 1024.0f) * 60;
			}
			
			//�ð� uint16 ���� �ʰ��ϸ� 0�� �Ǿ� �������� ���̳ʽ��� �Ǵµ� 0���� ���� ���� ���ٰ� �Ǵ�.
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

			//��հ� ��� ��, ���� RPM�� ��հ��� ������.
			int32 num = RPM_Data.Num();
			float total = 0.0f;
			for (int i = 0; i < num; i++)
			{
				total += RPM_Data[i];
			}

			RPM = total / num;
			
			//���� ������ ���� ���� prev���� �־��ְ� ���� ����� ��.
			Prev_WheelRevolutions = current_WR;
			Prev_WheelEventTimeStamp = current_WT;
			Prev_CrankRevolutions = current_CR;
			Prev_CrankEventTimeStamp = current_CT;
		}

	}

}

