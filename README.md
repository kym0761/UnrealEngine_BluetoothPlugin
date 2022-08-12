# UnrealEngine_BluetoothPlugin

```
UnrealEngine Bluetooth Plugin For Windows
UnrealEngine ver 4.26 or Higher (Maybe? I Use This Plugin in 4.26 Project, And It also works with Unreal Engine 5 in my Computer.)
```

```
Thanks For Information
Reference : https://forums.unrealengine.com/t/including-bluetoothleapis-h-not-working/410334/7
```

```
I revised Codes for using with a Cadence Sensor.
I'm NOT familiar with Bluetooth APIs. So, I Can not Fully Explain it. Please Consider it.
```

-------

## HOWTO?

```
first of all, if you want connect a Cadence Sensor, You Must Have a Bluetooth Dongle.
Connect Dongle to USB. And Search the Cadence and pair with your Computer.
```
```
To Use this Code, You must know Bluetooth UUID.
You can Search UUID in this Website : https://www.bluetooth.com/specifications/assigned-numbers/
Candence UUID is 0x1816
So, TO_SEARCH_DEVICE_UUID will be "{00001816-0000-1000-8000-00805F9B34FB}" //(XXXXXXXX-0000-1000-8000-00805F9B34FB)
You can find this Variable in BluetoothDataReceiver.h 11 line.
Maybe it could Do with other Bluetooth Device if you change UUID and know about your device Data Format.
```

```
FRunnable Class will Run Bluetooth APIs. See virtual uint32 Run();
```

> Finding Handle...
> Finding Service...
> Finding Characteristic...
> Finding Descriptor...
> Just Pass it. BluetoothGATTRegisterEvent API will run ParsingBluetoothData().

```
in ParsingBluetoothData(), You Can Access the Bluetooth Data in EventOutParameter.
it will Parse Raw Data to CSCMeasurement format Data.
```

```
You Can Receive Bluetooth Data With BluetoothDataReceiver.
And, You Can Read Data with ABluetoothDataReader Actor. (not only AActor, UObject also can do it.)
There is Example in BluetoothDataReader.h And .cpp
You must know that "My RPM Formula" in "Reader.cpp code" is not Correct Answer Because I Don't Know (>_< sorry)
it is the temporary Fomula for just check that it works!
```
