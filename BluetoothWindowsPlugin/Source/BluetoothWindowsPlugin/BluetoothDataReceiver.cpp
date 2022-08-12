// Fill out your copyright notice in the Description page of Project Settings.


#include "BluetoothDataReceiver.h"

BluetoothDataReceiver::BluetoothDataReceiver()
{
	m_kill = false;
	m_pause = false;

	// Initialise FEvent
	m_semaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);

	// Create the worker thread
	Thread = FRunnableThread::Create(this, TEXT("Cadence Thread"), 0, TPri_BelowNormal);
}

BluetoothDataReceiver::~BluetoothDataReceiver()
{
	if (m_semaphore)
	{
		// Clean up the FEvent
		FGenericPlatformProcess::ReturnSynchEventToPool(m_semaphore);
		m_semaphore = nullptr;
	}

	if (Thread)
	{

		// Clean up the worker thread
        Thread->Kill();
		delete Thread;
		Thread = nullptr;
	}
}

void BluetoothDataReceiver::EnsureCompletion()
{
	Stop();

	if (Thread)
	{
		Thread->WaitForCompletion();
	}
}

void BluetoothDataReceiver::PauseThread()
{
	m_pause = true;
}

void BluetoothDataReceiver::ContinueThread()
{
	m_pause = false;

	if (m_semaphore)
	{
		// FEvent->Trigger() will wake up the thread
		m_semaphore->Trigger();
	}
}

bool BluetoothDataReceiver::IsThreadPaused()
{
	return (bool)m_pause;
}

bool BluetoothDataReceiver::Init()
{
	return true;
}

uint32 BluetoothDataReceiver::Run()
{
    FPlatformProcess::Sleep(0.03);

    while (m_kill == false)
    {
        if (m_pause)
        {
            // FEvent->Wait() will sleep the thread until given a signal Trigger()
            m_semaphore->Wait();

            if (m_kill)
            {
                return 0;
            }

        }
        else {

            //Step 1: find the BLE device handle from its GUID
            //SEE UUID in Header. it's Bluetooth UUID
            GUID AGuid;

            CLSIDFromString(TEXT(TO_SEARCH_DEVICE_UUID), &AGuid);
            // get the handle 
            HANDLE hLEDevice = GetBLEHandle(AGuid);

            if (!hLEDevice)
            {
                //����̽��� �������� ������, �ٽ� ������ �ݺ��ؼ� ����̽��� ã�� ������ �ݺ���.
                UE_LOG(LogTemp, Warning, TEXT("device can't find?"));
                FPlatformProcess::Sleep(1);
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
            BLUETOOTH_GATT_EVENT_HANDLE EventHandle = nullptr;

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

                //�߿�. �Ľ� ������ �ڵ�� ���⼭ ����� ���̴�.
                if (currGattChar->IsNotifiable) {

                    BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

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

                    if (S_OK != hr) {

                    }

                    UE_LOG(LogTemp, Warning, TEXT("Register OK"));
                }


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


            //���� ����� �� ��� ��ٸ��� �ð�.
            FPlatformProcess::Sleep(0.05);

            //������ �Ľ� �̺�Ʈ unregister.
            if (EventHandle != nullptr)
            {
                BluetoothGATTUnregisterEvent(EventHandle, BLUETOOTH_GATT_FLAG_NONE);
                UE_LOG(LogTemp, Warning, TEXT("Unregister OK"));
            }

            //�ڵ� �۾� ��.
            CloseHandle(hLEDevice);

            //malloc�� �κ� Free���ش�.
            free(pCharBuffer);
            pCharBuffer = nullptr;
            free(pServiceBuffer);
            pServiceBuffer = nullptr;

            if (GetLastError() != NO_ERROR &&
                GetLastError() != ERROR_NO_MORE_ITEMS)
            {
                // Insert error handling here.
                UE_LOG(LogTemp, Warning, TEXT("Error Handling?"));
                return 1;
            }

            //���� ����� ��ġ
            if (!m_pause && !m_kill)
            {
                //������ �ٽ� �����Ѵ�... ��Ⱑ �����ִ��� ��� ������ �翬�� -> ������ �ޱ� ���� �Ѵ�.
                continue;
            }

        }

    }

    return 0;
}

void BluetoothDataReceiver::Stop()
{
    m_kill = true;
    m_pause = false;

    if (m_semaphore)
    {
        // Trigger the FEvent in case the thread is sleeping
        m_semaphore->Trigger();
    }
}

HANDLE BluetoothDataReceiver::GetBLEHandle(GUID AGuid)
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

void BluetoothDataReceiver::ParsingBluetoothData(BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context)
{
    PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)EventOutParameter;

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

        // ����
        // 03 - A0 - B1 - C2 - D3 - E4 - F5 - G6 - H7 - I8 - J9 �̶� �����Ͱ� �ִٰ� ��������
        // ���� CSCMeasurement�� ���ϸ�
        // 03 / A0 - B1 - C2 - D3 / E4 - F5 / G6 - H7 / I8 - J9
        // �����ʹ� �̷��� ������(/) ��� ������.
        // �� �����ʹ� ���� Little Endian �̶�
        // ���� ���, wheel revo ���� A0 - B1 - C2 - D3 ��
        // D3C2B1A0 ���� �д´ٴ� �ǹ̴�.
        // �ڼ��� ������ Little Endian ���ۿ� �˻��ؼ� Ȯ���� ��.
        // C#�� BitConverter�� �⺻ ������ Little Endian�̶� �Լ��� �׳� ���������
        // C++���� ���� ����̶� �����ͷ� ����.

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

        /*   UE_LOG(LogTemp, Warning, TEXT("wheel revo : %d    wheel time : %d    crank revo : %d   crank time : %d"),
               currentWheelRevolutions, currentWheelEventTimestamp, currentCrankRevolutions, currentCrankEventTimestamp);*/

        //WheelRevolutions = currentWheelRevolutions;
        //WheelEventTimestamp = currentWheelEventTimestamp;
        //CrankRevolutions = currentCrankRevolutions;
        //CrankEventTimestamp = currentCrankEventTimestamp;

        BluetoothDataReceiver* receiver = static_cast<BluetoothDataReceiver*>(Context);
        if (receiver)
        {
            receiver->SetCadenceData(currentWheelRevolutions, currentWheelEventTimestamp, currentCrankRevolutions, currentCrankEventTimestamp);
        }

    }
}

void BluetoothDataReceiver::SetCadenceData(int32 InWheelRevo, int32 InWheelTime, int32 InCrankRevo, int32 InCrankTime)
{
    if (m_mutex.TryLock())
    {
        WheelRevolutions = InWheelRevo;
        WheelEventTimestamp = InWheelTime;
        CrankRevolutions = InCrankRevo;
        CrankEventTimestamp = InCrankTime;

        m_mutex.Unlock();
    }
    //else
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Set Failed cause of mutex"));
    //}
}

void BluetoothDataReceiver::GetCadenceData(int32& WheelRevo, int32& WheelTime, int32& CrankRevo, int32& CrankTime)
{
    if (m_mutex.TryLock())
    {
        WheelRevo = (int32)WheelRevolutions;
        WheelTime = (int32)WheelEventTimestamp;
        CrankRevo = (int32)CrankRevolutions;
        CrankTime = (int32)CrankEventTimestamp;

        m_mutex.Unlock();
    }
    //else
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("! Get Failed cause of mutex"));
    //}
}
