// Copyright (C) 2005 Alexander Enzmann
// Enzmann-196.zip at http://www.dtweed.com/circuitcellar/caj00196.htm
// Code to appear in the "Circular cellar" # 196 http://www.circuitcellar.com/archives/titledirectory/181to200.html

#pragma once

// #define USE_XANDER_USB_FRAMEWORK

#ifdef USE_XANDER_USB_FRAMEWORK

#include <windows.h>
#include <tchar.h>
#if !defined(_WIN32_WCE)
extern "C" {
#include <setupapi.h>
#include <hidsdi.h>
}
#endif

class USBDevice
{
public:
	USBDevice(void);
	~USBDevice(void);

public:
	bool Connect(const DWORD VendorID, const DWORD ProductID);
	int Write(BYTE reportID, const BYTE *bytes, int bufLen);
	int Read(BYTE reportID, BYTE *bytes, int bufLen);
	bool Flush();
	bool IsOpen();

private:
	bool DeviceDetected;
	int Close();

#if !defined(_WIN32_WCE)
	bool FillDeviceInfo();
	bool OpenHidDevice(TCHAR *inDevicePath);
	void CloseHidDevice(bool FreeDeviceInfo);
	bool FindHID(const DWORD VendorID, const DWORD ProductID);

	// Properties
	TCHAR *DevicePath;
	HANDLE HidDevice; // A file handle to the hid device.
	HANDLE ReadHandle; // File handle for overlapped I/O
	OVERLAPPED HIDOverlapped;

	BYTE  *InputReport;
	short InputDataLength;

	BYTE *OutputReport;
	short OutputDataLength;

	BYTE  *FeatureReport;
	short FeatureDataLength;
	PHIDP_PREPARSED_DATA Ppd; // The opaque parser info describing this device
	HIDP_CAPS Caps; // The Capabilities of this hid device.
	HIDD_ATTRIBUTES Attributes;

	DWORD VID, PID;
#endif
};

#endif