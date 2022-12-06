// Fill out your copyright notice in the Description page of Project Settings.


#include "BluetoothDataReceiver.h"

FBluetoothDataReceiver::FBluetoothDataReceiver()
{
    UE_LOG(LogTemp, Warning, TEXT("FBluetoothDataReceiver"));

	m_kill = false;
	m_pause = false;

	//// Create the worker thread
    Thread = MakeShareable(FRunnableThread::Create(this, TEXT("Cadence Thread"), 0, TPri_BelowNormal));

}

FBluetoothDataReceiver::~FBluetoothDataReceiver()
{
    UE_LOG(LogTemp, Warning, TEXT("~FBluetoothDataReceiver"));



	if (Thread.IsValid())
	{

		// Clean up the worker thread
        Thread->Kill();

	}
}

void FBluetoothDataReceiver::EnsureCompletion()
{
    UE_LOG(LogTemp, Warning, TEXT("EnsureCompletion"));

	Stop();

	if (Thread.IsValid())
	{
		Thread->WaitForCompletion();
	}
}

void FBluetoothDataReceiver::PauseThread()
{
	m_pause = true;
}

void FBluetoothDataReceiver::ContinueThread()
{
	m_pause = false;

}

bool FBluetoothDataReceiver::IsThreadPaused()
{
	return (bool)m_pause;
}

bool FBluetoothDataReceiver::Init()
{
    UE_LOG(LogTemp, Warning, TEXT("FBluetoothDataReceiver::Init()"));
	return true;
}

void FBluetoothDataReceiver::Stop()
{
    m_kill = true;
    m_pause = true;

    UE_LOG(LogTemp, Warning, TEXT("FBluetoothDataReceiver::Stop()"));

}

void FBluetoothDataReceiver::Exit()
{
    UE_LOG(LogTemp, Warning, TEXT("FBluetoothDataReceiver::Exit()"));
}

uint32 FBluetoothDataReceiver::Run()
{
    //Sleep 0.01 first
    FPlatformProcess::Sleep(0.01);

    while (true)
    {
        bIsOk_toLoop = true;

        //kill 상태가 되면 빠져나온다.
        if (m_kill)
        {
            break;
        }

        //pause 상태면 멈추고 kill상태로 전환되면 빠져나온다.
        if (m_pause)
        {
            if (m_kill)
            {
                break;
            }
        }



        //Step 1: find the BLE device handle from its GUID
        //SEE UUID in Header. it's Bluetooth UUID
        GUID AGuid;

        CLSIDFromString(TEXT(TO_SEARCH_DEVICE_UUID), &AGuid);
        // get the handle 
        HANDLE hLEDevice = GetBLEHandle(AGuid);

        if (!hLEDevice)
        {
            //디바이스가 존재하지 않으면, 다시 루프를 반복해서 디바이스를 찾을 때까지 반복함.
            UE_LOG(LogTemp, Warning, TEXT("device can't find.."));
            FPlatformProcess::Sleep(1.0f);
            continue;
        }

        //Step 2: Get a list of services that the device advertises
        // first send 0,NULL as the parameters to BluetoothGATTServices inorder to get the number of
        // services in serviceBufferCount
        USHORT serviceBufferCount;
        ////////////////////////////////////////////////////////////////////////////
        // Determine Services Buffer Size
        ////////////////////////////////////////////////////////////////////////////

        HRESULT hr = BluetoothGATTGetServices(
            hLEDevice,
            0,
            NULL,
            &serviceBufferCount,
            BLUETOOTH_GATT_FLAG_NONE);

        if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {

        }

        // my custom1
        if (serviceBufferCount == 0)
        {
            //서비스를 찾기 못했으면 어차피 다음 내용이 의미 없으므로 반복.
            UE_LOG(LogTemp, Warning, TEXT("service Can't Find.."));
            FPlatformProcess::Sleep(1.0f);
            continue;
        }
        // my custom1 end

        PBTH_LE_GATT_SERVICE pServiceBuffer = (PBTH_LE_GATT_SERVICE)
            malloc(sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);

        if (NULL == pServiceBuffer) {
            
        }
        else {
            RtlZeroMemory(pServiceBuffer,
                sizeof(BTH_LE_GATT_SERVICE) * serviceBufferCount);
        }

        ////////////////////////////////////////////////////////////////////////////
        // Retrieve Services
        ////////////////////////////////////////////////////////////////////////////

        USHORT numServices;
        hr = BluetoothGATTGetServices(
            hLEDevice,
            serviceBufferCount,
            pServiceBuffer,
            &numServices,
            BLUETOOTH_GATT_FLAG_NONE);

        if (S_OK != hr) {
            UE_LOG(LogTemp, Warning, TEXT("hr error1"));
            bIsOk_toLoop = false;
        }


        //Step 3: now get the list of charactersitics. note how the pServiceBuffer is required from step 2
        ////////////////////////////////////////////////////////////////////////////
        // Determine Characteristic Buffer Size
        ////////////////////////////////////////////////////////////////////////////

        USHORT charBufferSize;
        hr = BluetoothGATTGetCharacteristics(
            hLEDevice,
            pServiceBuffer,
            0,
            NULL,
            &charBufferSize,
            BLUETOOTH_GATT_FLAG_NONE);

        //set the appropriate callback function when the descriptor change value
        TArray<BLUETOOTH_GATT_EVENT_HANDLE> event_Handle_Arr;

        if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {

        }

        PBTH_LE_GATT_CHARACTERISTIC pCharBuffer = NULL;
        if (charBufferSize > 0) {
            pCharBuffer = (PBTH_LE_GATT_CHARACTERISTIC)
                malloc(charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));

            if (NULL == pCharBuffer) {

            }
            else {
                RtlZeroMemory(pCharBuffer,
                    charBufferSize * sizeof(BTH_LE_GATT_CHARACTERISTIC));
            }

            ////////////////////////////////////////////////////////////////////////////
            // Retrieve Characteristics
            ////////////////////////////////////////////////////////////////////////////
            USHORT numChars;
            hr = BluetoothGATTGetCharacteristics(
                hLEDevice,
                pServiceBuffer,
                charBufferSize,
                pCharBuffer,
                &numChars,
                BLUETOOTH_GATT_FLAG_NONE);

            if (S_OK != hr) {
                UE_LOG(LogTemp, Warning, TEXT("hr error2"));
                bIsOk_toLoop = false;
            }

            if (numChars != charBufferSize) {

            }

        }


        //Step 4: now get the list of descriptors. note how the pCharBuffer is required from step 3
        //descriptors are required as we descriptors that are notification based will have to be written
        //once IsSubcribeToNotification set to true, we set the appropriate callback function
        //need for setting descriptors for notification according to
        //http://social.msdn.microsoft.com/Forums/en-US/11d3a7ce-182b-4190-bf9d-64fefc3328d9/windows-bluetooth-le-apis-event-callbacks?forum=wdk
        PBTH_LE_GATT_CHARACTERISTIC currGattChar;
        for (int ii = 0; ii < charBufferSize; ii++) {
            currGattChar = &pCharBuffer[ii];
            //USHORT charValueDataSize;
            //PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer;


            ///////////////////////////////////////////////////////////////////////////
            // Determine Descriptor Buffer Size
            ////////////////////////////////////////////////////////////////////////////
            USHORT descriptorBufferSize;
            hr = BluetoothGATTGetDescriptors(
                hLEDevice,
                currGattChar,
                0,
                NULL,
                &descriptorBufferSize,
                BLUETOOTH_GATT_FLAG_NONE);

            if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {

            }

            PBTH_LE_GATT_DESCRIPTOR pDescriptorBuffer;
            if (descriptorBufferSize > 0) {
                pDescriptorBuffer = (PBTH_LE_GATT_DESCRIPTOR)
                    malloc(descriptorBufferSize
                        * sizeof(BTH_LE_GATT_DESCRIPTOR));

                if (NULL == pDescriptorBuffer) {

                }
                else {
                    RtlZeroMemory(pDescriptorBuffer, descriptorBufferSize);
                }

                ////////////////////////////////////////////////////////////////////////////
                // Retrieve Descriptors
                ////////////////////////////////////////////////////////////////////////////

                USHORT numDescriptors;
                hr = BluetoothGATTGetDescriptors(
                    hLEDevice,
                    currGattChar,
                    descriptorBufferSize,
                    pDescriptorBuffer,
                    &numDescriptors,
                    BLUETOOTH_GATT_FLAG_NONE);

                if (S_OK != hr) {
                    UE_LOG(LogTemp, Warning, TEXT("hr error3"));
                    bIsOk_toLoop = false;
                }

                if (numDescriptors != descriptorBufferSize) {

                }

                for (int kk = 0; kk < numDescriptors; kk++) {
                    PBTH_LE_GATT_DESCRIPTOR  currGattDescriptor = &pDescriptorBuffer[kk];
                    ////////////////////////////////////////////////////////////////////////////
                    // Determine Descriptor Value Buffer Size
                    ////////////////////////////////////////////////////////////////////////////
                    USHORT descValueDataSize;
                    hr = BluetoothGATTGetDescriptorValue(
                        hLEDevice,
                        currGattDescriptor,
                        0,
                        NULL,
                        &descValueDataSize,
                        BLUETOOTH_GATT_FLAG_NONE);

                    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {

                    }

                    PBTH_LE_GATT_DESCRIPTOR_VALUE pDescValueBuffer = (PBTH_LE_GATT_DESCRIPTOR_VALUE)malloc(descValueDataSize);

                    if (NULL == pDescValueBuffer) {

                    }
                    else {
                        RtlZeroMemory(pDescValueBuffer, descValueDataSize);
                    }

                    ////////////////////////////////////////////////////////////////////////////
                    // Retrieve the Descriptor Value
                    ////////////////////////////////////////////////////////////////////////////

                    hr = BluetoothGATTGetDescriptorValue(
                        hLEDevice,
                        currGattDescriptor,
                        (ULONG)descValueDataSize,
                        pDescValueBuffer,
                        NULL,
                        BLUETOOTH_GATT_FLAG_NONE);
                    if (S_OK != hr) {
                        UE_LOG(LogTemp, Warning, TEXT("hr error4"));
                        bIsOk_toLoop = false;
                    }
                    //you may also get a descriptor that is read (and not notify) andi am guessing the attribute handle is out of limits
                    // we set all descriptors that are notifiable to notify us via IsSubstcibeToNotification
                    if (currGattDescriptor->AttributeHandle < 255) {
                        BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

                        RtlZeroMemory(&newValue, sizeof(newValue));

                        newValue.DescriptorType = ClientCharacteristicConfiguration;
                        newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = 1;

                        hr = BluetoothGATTSetDescriptorValue(
                            hLEDevice,
                            currGattDescriptor,
                            &newValue,
                            BLUETOOTH_GATT_FLAG_NONE);
                        if (S_OK != hr) {
                            UE_LOG(LogTemp, Warning, TEXT("hr error5"));
                            bIsOk_toLoop = false;
                        }
                        else {

                        }

                    }

                    free(pDescValueBuffer);
                    pDescValueBuffer = nullptr;
                }

            }

            ////set the appropriate callback function when the descriptor change value
            //BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

            BLUETOOTH_GATT_EVENT_HANDLE EventHandle;

            //중요. 파싱 데이터 코드는 여기서 실행될 것이다.
            if (currGattChar->IsNotifiable) {

                //데이터가 바뀔 때만 이벤트가 동작한다.
                BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

                // PFNBLUETOOTH_GATT_EVENT_CALLBACK callback = this->ParsingBluetoothData;

                BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
                EventParameterIn.Characteristics[0] = *currGattChar;
                EventParameterIn.NumCharacteristics = 1;

                hr = BluetoothGATTRegisterEvent(
                    hLEDevice,
                    EventType,
                    &EventParameterIn,
                    ParsingBluetoothData,
                    this,
                    &EventHandle,
                    BLUETOOTH_GATT_FLAG_NONE);

                if (EventHandle)
                {
                    event_Handle_Arr.Add(EventHandle);
                }

                if (S_OK != hr) {
                    UE_LOG(LogTemp, Warning, TEXT("hr error6"));
                    bIsOk_toLoop = false;
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Loop Start"));
                    float timer = 5.0f;
                    while (bIsOk_toLoop == true && m_pause == false && m_kill == false)
                    {
                        timer -= 0.01f;
                        FPlatformProcess::Sleep(0.01);
                        if (timer < 0.0f)
                        {
                            break;
                        }
                        //Loop Until Something is Wrong.
                    }

                    UE_LOG(LogTemp, Warning, TEXT("Loop Ended"));
                }

            }

            //PBTH_LE_GATT_CHARACTERISTIC_VALUE pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);

            //if (currGattChar->IsReadable) {//currGattChar->IsReadable
            //                               ////////////////////////////////////////////////////////////////////////////
            //                               // Determine Characteristic Value Buffer Size
            //                               ////////////////////////////////////////////////////////////////////////////
            //    hr = BluetoothGATTGetCharacteristicValue(
            //        hLEDevice,
            //        currGattChar,
            //        0,
            //        NULL,
            //        &charValueDataSize,
            //        BLUETOOTH_GATT_FLAG_NONE);

            //    if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr) {

            //    }

            //    pCharValueBuffer = (PBTH_LE_GATT_CHARACTERISTIC_VALUE)malloc(charValueDataSize);

            //    if (NULL == pCharValueBuffer) {

            //    }
            //    else {
            //        RtlZeroMemory(pCharValueBuffer, charValueDataSize);
            //    }

            //    ////////////////////////////////////////////////////////////////////////////
            //    // Retrieve the Characteristic Value
            //    ////////////////////////////////////////////////////////////////////////////

            //    hr = BluetoothGATTGetCharacteristicValue(
            //        hLEDevice,
            //        currGattChar,
            //        (ULONG)charValueDataSize,
            //        pCharValueBuffer,
            //        NULL,
            //        BLUETOOTH_GATT_FLAG_NONE);

            //    if (S_OK != hr) {

            //    }


            //    // Free before going to next iteration, or memory leak.
            //    free(pCharValueBuffer);
            //    pCharValueBuffer = NULL;
            //}


        }

        //// Go into an inf loop that sleeps. you will ideally see notifications from the Cadence device
        //while (!m_pause && !m_kill)
        //{
        //    //prevHeartRate = heartRate;
        //    Sleep(1000);
        //    continue;
        //}



        //FPlatformProcess::Sleep(0.001);

        //////UE_LOG(LogTemp, Warning, TEXT("Register OK"));

        ////////문제가 없는 동안은 계속 이벤트를 실행하게 한다.
        //////while (true)
        //////{
        //////    if (m_pause)
        //////    {
        //////        UE_LOG(LogTemp, Warning, TEXT("break by m_pause"));
        //////        break;
        //////    }

        //////    if (m_kill)
        //////    {
        //////        UE_LOG(LogTemp, Warning, TEXT("break by m_kill"));
        //////        break;
        //////    }

        //////    if ((GetLastError() != NO_ERROR) && (GetLastError() != ERROR_NO_MORE_ITEMS))
        //////    {
        //////        UE_LOG(LogTemp, Warning, TEXT("break by error"));
        //////        break;
        //////    }
        //////}

        //////UE_LOG(LogTemp, Warning, TEXT("Escape"));

        ////FString lengthStr = FString("EventHandle Count : ") + FString::FromInt(event_Handle_Arr.Num());
        ////UE_LOG(LogTemp, Warning, TEXT("%s"), *lengthStr);

        //데이터 파싱 이벤트 unregister.
        for (auto eventHandle : event_Handle_Arr)
        {
            if (eventHandle != nullptr)
            {
                BluetoothGATTUnregisterEvent(eventHandle, BLUETOOTH_GATT_FLAG_NONE);
                //UE_LOG(LogTemp, Warning, TEXT("Unregister OK"));
            }
        }

        //핸들 작업 끝.
        CloseHandle(hLEDevice);

        //malloc한 부분 Free해준다.
        free(pCharBuffer);
        pCharBuffer = nullptr;
        free(pServiceBuffer);
        pServiceBuffer = nullptr;

        ////루프 재시작 위치
        //if (!m_pause && !m_kill)
        //{
        //    //루프를 다시 시작한다... 기기가 꺼져있는지 등등 사유로 재연결 -> 데이터 받기 등을 한다.
        //    continue;
        //}



    }

    return 0;
}

HANDLE FBluetoothDataReceiver::GetBLEHandle(GUID AGuid)
{
    HDEVINFO hDI;
    SP_DEVICE_INTERFACE_DATA did;
    SP_DEVINFO_DATA dd;
    GUID BluetoothInterfaceGUID = AGuid;
    HANDLE hComm = NULL;

    hDI = SetupDiGetClassDevs(&BluetoothInterfaceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

    if (hDI == INVALID_HANDLE_VALUE) return NULL;

    did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    dd.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDI, NULL, &BluetoothInterfaceGUID, i, &did); i++)
    {
        SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;

        DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        DWORD size = 0;

        if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, NULL, 0, &size, 0))
        {
            int err = GetLastError();

            if (err == ERROR_NO_MORE_ITEMS) break;

            PSP_DEVICE_INTERFACE_DETAIL_DATA pInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)GlobalAlloc(GPTR, size);

            pInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, pInterfaceDetailData, size, &size, &dd))
                break;

            hComm = CreateFile(
                pInterfaceDetailData->DevicePath,
                GENERIC_WRITE | GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            GlobalFree(pInterfaceDetailData);
        }
    }

    SetupDiDestroyDeviceInfoList(hDI);
    return hComm;
}

void FBluetoothDataReceiver::ParsingBluetoothData(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
    PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;

    UE_LOG(LogTemp, Warning, TEXT("Parsing"));

    HRESULT hr;
    if (0 == ValueChangedEventParameters->CharacteristicValue->DataSize) {
        hr = E_FAIL;
    }
    else {

        // CSCMeasurement Format
        // total 11 bytes. Data are Little Endian Expression.
        // 1 byte : flag -> 1 : wheel, 2: crank, 3 : both
        // 4 bytes : wheel revolutions
        // 2 bytes : wheel Event Time Stamp
        // 2 bytes : Crank Revolotions
        // 2 bytes : crank Event Time Stamp

        // 예시
        // 03 - A0 - B1 - C2 - D3 - E4 - F5 - G6 - H7 - I8 - J9 이란 데이터가 있다고 가정하자
        // 위의 CSCMeasurement에 의하면
        // 03 / A0 - B1 - C2 - D3 / E4 - F5 / G6 - H7 / I8 - J9
        // 데이터는 이렇게 슬래쉬(/) 대로 나뉜다.
        // 이 데이터는 각각 Little Endian 이라
        // 예를 들면, wheel revo 값인 A0 - B1 - C2 - D3 를
        // D3C2B1A0 으로 읽는다는 의미다.
        // 자세한 정보는 Little Endian 구글에 검색해서 확인할 것.
        // C#은 BitConverter가 기본 세팅이 Little Endian이라 함수를 그냥 사용하지만
        // C++에선 없는 기능이라 포인터로 만듬.

        const uint8 WheelRevolutionDataPresent = 0x01;
        const uint8 CrankRevolutionDataPresent = 0x02;

        //cumilative wheel revo
        uint32 currentWheelRevolutions = 0;
        //last wheel event time
        uint16 currentWheelEventTimestamp = 0;
        //cumulative crank revo
        uint16 currentCrankRevolutions = 0;
        //last crank event time
        uint16 currentCrankEventTimestamp = 0;

        auto* ptr = ValueChangedEventParameters->CharacteristicValue->Data;

        //flags 0x03, 0x02, 0x01
        uint8 flags = (uint8)(*ptr);
        ptr = ptr + 1;

        if ((flags & WheelRevolutionDataPresent) != 0)
        {

            currentWheelRevolutions = (uint32_t)(*ptr) | ((uint32_t)(*(ptr + 1))) << 8 | ((uint32_t)(*(ptr + 2))) << 16 | ((uint32_t)(*(ptr + 3))) << 24;
            ptr = ptr + 4;
            currentWheelEventTimestamp = (uint16_t)(*ptr) | ((uint16_t)(*(ptr + 1))) << 8;
            ptr = ptr + 2;
        }

        if ((flags & CrankRevolutionDataPresent) != 0)
        {
            currentCrankRevolutions = (uint16_t)(*ptr) | ((uint16_t)(*(ptr + 1))) << 8;
            ptr = ptr + 2;
            currentCrankEventTimestamp = (uint16_t)(*ptr) | ((uint16_t)(*(ptr + 1))) << 8;
        }

        FBluetoothDataReceiver* receiver = static_cast<FBluetoothDataReceiver*>(Context);
        if (receiver)
        {
            receiver->SetCadenceData(currentWheelRevolutions, currentWheelEventTimestamp, currentCrankRevolutions, currentCrankEventTimestamp);
        }

    }
}

void FBluetoothDataReceiver::SetCadenceData(uint32 InWheelRevo, uint16 InWheelTime, uint16 InCrankRevo, uint16 InCrankTime)
{
    if (m_mutex.TryLock())
    {
        WheelRevolutions = InWheelRevo;
        WheelEventTimestamp = InWheelTime;
        CrankRevolutions = InCrankRevo;
        CrankEventTimestamp = InCrankTime;

        m_mutex.Unlock();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Set Failed cause of mutex"));
    }
}

void FBluetoothDataReceiver::GetCadenceData(uint32& WheelRevo, uint16& WheelTime, uint16& CrankRevo, uint16& CrankTime)
{
    if (m_mutex.TryLock())
    {
        WheelRevo = WheelRevolutions;
        WheelTime = WheelEventTimestamp;
        CrankRevo = CrankRevolutions;
        CrankTime = CrankEventTimestamp;

        m_mutex.Unlock();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("! Get Failed cause of mutex"));
    }
}
