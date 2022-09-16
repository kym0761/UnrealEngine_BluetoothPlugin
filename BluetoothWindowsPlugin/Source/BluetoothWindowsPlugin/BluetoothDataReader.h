// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BluetoothDataReader.generated.h"

class BluetoothDataReceiver;

UCLASS()
class BLUETOOTHWINDOWSPLUGIN_API ABluetoothDataReader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABluetoothDataReader();

	TSharedPtr<BluetoothDataReceiver> DataReceiver;

	FTimerHandle BluetoothReadTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_WheelRevolutions = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_WheelEventTimeStamp = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_CrankRevolutions = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
	int32 Prev_CrankEventTimeStamp = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float RPM;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float DataReadInterval;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Bluetooth")
		float RunTimer;

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
