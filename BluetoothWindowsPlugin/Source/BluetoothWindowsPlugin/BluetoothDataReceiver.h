// Fill out your copyright notice in the Description page of Project Settings.

//Reference : https://forums.unrealengine.com/t/including-bluetoothleapis-h-not-working/410334/7
// https://pastebin.com/Wh6JbndT
// https://pastebin.com/4UcCLyWx

#pragma once

#define NTDDI_VERSION NTDDI_WIN8
#define _WIN32_WINNT _WIN32_WINNT_WIN8
#define TO_SEARCH_DEVICE_UUID "{00001816-0000-1000-8000-00805F9B34FB}" //Cadence Sensor UUID

#pragma warning( disable : 4068 )
#pragma warning( disable : 4668 )

#include "CoreMinimal.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/COMPointer.h"
#include "windows.h"
#include "setupapi.h"
#include "devguid.h"
#include "regstr.h"
#include "bthdef.h"
#include "Bluetoothleapis.h"
#include "Windows/HideWindowsPlatformTypes.h"

#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")

/**
 * FRunnable을 사용하여 다른 쓰레드에서 블루투스 데이터를 받도록 한다.
 */
class BLUETOOTHWINDOWSPLUGIN_API BluetoothDataReceiver : public FRunnable
{
public:
	BluetoothDataReceiver();
	~BluetoothDataReceiver();

    // Thread handling functions
    void EnsureCompletion();        // Function for killing the thread
    void PauseThread();             // Function for pausing the thread
    void ContinueThread();          // Function for continuing/unpausing the thread
    bool IsThreadPaused();          // Function to check the state of the thread

    // FRunnable interface functions
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;
    // Bluetooth Windows API functions
    HANDLE GetBLEHandle(__in GUID AGuid);

    //블루투스 데이터 파싱
    static void ParsingBluetoothData(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context);

    // Function to get Cadence Data
    //setter
    void SetCadenceData(int32 InWheelRevo, int32 InWheelTime, int32 InCrankRevo, int32 InCrankTime);
    //getter outter
    void GetCadenceData(int32& WheelRevo , int32& WheelTime, int32& CrankRevo, int32& CrankTime);

private:

    // Thread to run the worker FRunnable on
    FRunnableThread* Thread;

    FCriticalSection m_mutex;
    FEvent* m_semaphore;

    // Thread-safe booleans for changing the state of the thread
    FThreadSafeBool m_kill;
    FThreadSafeBool m_pause;

    uint32 WheelRevolutions = 0;
    uint16 WheelEventTimestamp = 0;
    uint16 CrankRevolutions = 0;
    uint16 CrankEventTimestamp = 0;

};
