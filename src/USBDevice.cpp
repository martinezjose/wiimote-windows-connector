// Copyright (C) 2005 Alexander Enzmann
// Enzmann-196.zip at http://www.dtweed.com/circuitcellar/caj00196.htm
// Code to appear in the "Circular cellar" # 196 http://www.circuitcellar.com/archives/titledirectory/181to200.html

#include "USBDevice.h"
#ifdef USE_XANDER_USB_FRAMEWORK

#pragma comment ( lib, "Redist/DDK_XP/hid.lib")
#pragma comment ( lib, "Redist/DDK_XP/setupapi.lib")

USBDevice::USBDevice(void)
{
	DeviceDetected = false;
#if !defined(_WIN32_WCE)
	DevicePath = NULL;
	HidDevice = NULL; // A file handle to the hid device.
	ReadHandle = NULL; // File handle for overlapped I/O

	Ppd = NULL; // The opaque parser info describing this device

	InputReport = NULL;
	InputDataLength = 0;

	OutputReport = NULL;
	OutputDataLength = 0;

	FeatureReport = NULL;
	FeatureDataLength = 0;

	VID = 0;
	PID = 0;
#endif
}

USBDevice::~USBDevice(void)
{
	Flush();
	Close();
}

bool USBDevice::IsOpen()
{
	return DeviceDetected;
}

bool USBDevice::Flush()
{
#if !defined(_WIN32_WCE)
	return HidD_FlushQueue(HidDevice) == TRUE;
#else
	return 1;
#endif
}

int USBDevice::Close()
{
#if !defined(_WIN32_WCE)
	CloseHidDevice(true);
#endif

	return 1;
}

#if !defined(_WIN32_WCE)

bool USBDevice::FillDeviceInfo()
{
	InputDataLength = Caps.InputReportByteLength;
	if (InputDataLength > 0)
		InputReport = new BYTE[2*Caps.InputReportByteLength+1];
	OutputDataLength = Caps.OutputReportByteLength;
	if (OutputDataLength > 0)
		OutputReport = new BYTE[2*Caps.OutputReportByteLength+1];
	FeatureDataLength = Caps.FeatureReportByteLength;
	if (FeatureDataLength > 0)
		FeatureReport = new BYTE[2*Caps.FeatureReportByteLength+1];

	return true;
}

bool USBDevice::OpenHidDevice(TCHAR *inDevicePath)
{
	DWORD accessFlags = 0;
	DWORD sharingFlags = 0;
	bool bSuccess;

	DevicePath = inDevicePath;

	if (DevicePath == NULL)
		// No device name
		return false;

	accessFlags = GENERIC_READ | GENERIC_WRITE;
	sharingFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;

	HidDevice = CreateFile(DevicePath, accessFlags, sharingFlags,
					NULL, OPEN_EXISTING, 0, NULL);

	if (HidDevice == INVALID_HANDLE_VALUE)
	{
		DeviceDetected = false;
		DevicePath = NULL;
		return false;
	}

	ReadHandle = CreateFile(DevicePath, accessFlags, sharingFlags,
					NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

	if (ReadHandle == INVALID_HANDLE_VALUE)
	{
		DeviceDetected = false;
		DevicePath = NULL;
		return false;
	}
	HIDOverlapped.hEvent = NULL;

	// Fill in the reset of the HidDevice structure
	if (!HidD_GetPreparsedData(HidDevice, &Ppd))
	{
		DeviceDetected = false;
		DevicePath = NULL;
		CloseHandle(HidDevice);
		CloseHandle(ReadHandle);

		return false;
	}

	if (HidD_GetAttributes(HidDevice, &Attributes) == 0)
	{
		DeviceDetected = false;
		DevicePath = NULL;
		CloseHandle(HidDevice);
		CloseHandle(ReadHandle);
		HidD_FreePreparsedData(Ppd);

		return false;
	}

	if (HidP_GetCaps(Ppd, &Caps) == 0)
	{
		DeviceDetected = false;
		DevicePath = NULL;
		CloseHandle(HidDevice);
		CloseHandle(ReadHandle);
		HidD_FreePreparsedData(Ppd);

		return false;
	}

	DWORD numBuffers = 0;
	if (HidD_GetNumInputBuffers(HidDevice, &numBuffers))
	{
		int newNumBufs = numBuffers;
	}

	bSuccess = FillDeviceInfo();

	return bSuccess;
}

void USBDevice::CloseHidDevice(bool FreeDeviceInfo)
{
	DeviceDetected = false;

	DevicePath = NULL;

	if (HidDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(HidDevice);
		HidDevice = INVALID_HANDLE_VALUE;
	}

	if (InputReport != NULL)
	{
		delete[] InputReport;
		InputReport = NULL;
	}
	if (OutputReport != NULL)
	{
		delete[] OutputReport;
		OutputReport = NULL;
	}
	if (FeatureReport != NULL)
	{
		delete[] FeatureReport;
		FeatureReport = NULL;
	}

	VID = 0;
	PID = 0;
}

bool USBDevice::FindHID(DWORD VendorID, DWORD ProductID)
{
	//Use a series of API calls to find a HID with a matching Vendor and Product ID.
	SP_DEVICE_INTERFACE_DATA devInfoData;
	int index = 0;
	bool foundDevice = false;
	int Result;
	SP_DEVICE_INTERFACE_DETAIL_DATA detailData, *detailDataPtr;
	HANDLE hDevInfo = NULL;
	GUID HidGuid;
	DWORD Required = 0;
	DWORD Length = 0;

	// Get the GUID for HIDs
	HidD_GetHidGuid(&HidGuid);

	// Get a handle to a device information set for all installed devices.
	hDevInfo = SetupDiGetClassDevs(&HidGuid, NULL, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	devInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	for (index = 0; ; index++)
	{
		// Get the next device in the list
		Result = SetupDiEnumDeviceInterfaces(hDevInfo, 0, &HidGuid, index, &devInfoData);

		if (Result != 0)
		{
			// Found a HID - now see if it is the right one

			// Determine how much data is needed to represent the
			// device detail information
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData,
				NULL, 0, &Length, NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.
			detailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			BYTE *detailDataBuf = new BYTE[2*Length];
			detailDataPtr = (SP_DEVICE_INTERFACE_DETAIL_DATA *)detailDataBuf;
			detailDataPtr->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			// Retrieve the actual device information.
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData,
				(PSP_DEVICE_INTERFACE_DETAIL_DATA)detailDataBuf, Length, &Required, NULL);

			// Skip over cbsize (4 bytes) to get the address of the devicePathName.
			TCHAR *devicePathName = (TCHAR *)(detailDataBuf + 4);

			// Open the device for read/write
			if (!OpenHidDevice(devicePathName))
				continue;

			//Is it the desired device?
			foundDevice = false;

			if ((Attributes.VendorID == VendorID)
				&& (Attributes.ProductID == ProductID))
			{
				//Both the Product and Vendor IDs match.
				foundDevice = true;

				// Show the device's capablities.
				if (HIDOverlapped.hEvent == 0)
				{
					HIDOverlapped.hEvent = CreateEvent(0, 1, 1, _TEXT("")); ;
					HIDOverlapped.Offset = 0;
					HIDOverlapped.OffsetHigh = 0;
				}
				break;
			}
			else
				//The Vendor ID doesn't match.
				CloseHidDevice(true);

			//Free the memory used by the detailData structure (no longer needed).
			delete[] detailDataBuf;
		}
		else
			// No more HID devices in the list
			break;
	}

	//Free the memory reserved for hDevInfo by SetupDiClassDevs.
	SetupDiDestroyDeviceInfoList(hDevInfo);

	DeviceDetected = foundDevice;

	return DeviceDetected;
}
#endif // !defined(_WIN32_WCE)

bool USBDevice::Connect(const DWORD tVID, const DWORD tPID)
{
#if !defined(_WIN32_WCE)
	VID = tVID;
	PID = tPID;
	if (FindHID(tVID, tPID))
	{
		return true;
	}
	else
		return false;
#else
	return false;
#endif
}

int USBDevice::Write(BYTE reportID, const BYTE *bytes, int nBuffLen)
{
#if !defined(_WIN32_WCE)
    // Ensure the device is available.
    if (!DeviceDetected && (VID != 0) && (PID != 0))
    {
        if (!FindHID(VID, PID))
            return false;
    }

    // Ensure the feature report is clear before sending.
	memset(FeatureReport, 0, FeatureDataLength);
	FeatureReport[0] = reportID;

	if (nBuffLen > FeatureDataLength)
		nBuffLen = FeatureDataLength;

	memcpy(&FeatureReport[1], bytes, nBuffLen);

	int Result = HidD_SetFeature(HidDevice, FeatureReport, FeatureDataLength);

	return ((Result == TRUE) ? nBuffLen : -1);

#else
	return false;
#endif
}

#include <stdio.h>
int USBDevice::Read(BYTE reportID, BYTE *bytes, int nBuffLen)
{
#if !defined(_WIN32_WCE)
	if (FeatureReport == NULL)
		return 0;

	// Don't return more than what can be held in a single feature request
	if (nBuffLen > FeatureDataLength)
		nBuffLen = FeatureDataLength;

#if 1
	// Clear the buffer before reading - we can detect some read errors
	// if the buffer doesn't get any data written into it.
	memset(FeatureReport, 0, FeatureDataLength+1);
	FeatureReport[0] = reportID;
	int Result = HidD_GetFeature(HidDevice, FeatureReport, FeatureDataLength);

	if (Result == TRUE)
	{
		memcpy(bytes, &FeatureReport[1], nBuffLen);
		for (int i=0;i<FeatureDataLength;i++)
			if (bytes[i] != 0)
				return nBuffLen;

		return 0;
	}
#else
	for (int retryCtr=0;retryCtr<4;retryCtr++)
	{
		// Clear the feature buffer
		memset(FeatureReport, 0, FeatureDataLength+1);
		FeatureReport[0] = reportID;
		int Result = HidD_GetFeature(HidDevice, FeatureReport, FeatureDataLength);

		if (Result == TRUE)
		{
			memcpy(bytes, &FeatureReport[1], nBuffLen);
			for (int i=0;i<FeatureDataLength;i++)
				if (bytes[i] != 0)
					return nBuffLen;
		}
		printf("r: %d\n", retryCtr);
		::Sleep(1);
	}
#endif

	return 0;
#else
	return 0;
#endif
}

#endif