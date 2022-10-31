// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BluetoothDataReader.generated.h"

class FBluetoothDataReceiver;

UCLASS()
class BLUETOOTHWINDOWSPLUGIN_API ABluetoothDataReader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABluetoothDataReader();

	TSharedPtr<FBluetoothDataReceiver> DataReceiver;

	FTimerHandle BluetoothReadTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_WheelRevolutions = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_WheelEventTimeStamp = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_CrankRevolutions = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_CrankEventTimeStamp = 0;

	//���� RPM
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float RPM;
	//Interpolation Target
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float TargetRPM;

	//������ cm��. �⺻���� 46cm
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bluetooth")
		float Diameter = 46.0f;
	//������ km/h
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float BikeSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float TargetBikeSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float DataReadInterval;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float RunTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bluetooth")
		float TimerSet;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
		void ReadData();
};
