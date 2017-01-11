// Delcom Engineering 
// DelcomDLL.h
// Version 0.6
// 02/11/07

#ifndef DELCOMDLL_HEADER
#define DELCOMDLL_HEADER 1


// defines
#define	OFF		0
#define	ON		1
#define	FLASH	2

#define USBIODS		1
#define USBDELVI	2
#define USBNDSPY	3

#define GREENLED	0
#define REDLED		1
#define BLUELED		2

#define MaxDeviceLen 512

// Typedef
typedef struct _DeviceNameStruct{
	char DeviceName[MaxDeviceLen];
} DeviceNameStruct, *pDeviceNameStruct;


typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef char				CHAR;
typedef unsigned short      WORD;
typedef void *  HANDLE;   
typedef CHAR *LPSTR, *PSTR;      
typedef unsigned char uchar;         
typedef unsigned int  uint;         

typedef struct _PacketStruct{
		uchar Recipient;
		uchar DeviceModel;
		uchar MajorCmd;
		uchar MinorCmd;
		uchar DataLSB;
		uchar DataMSB;
		short Length;
		uchar ExtData[8];
	} PacketStruct,*pPacketStruct;




// DLL Generic Functions
HANDLE DelcomGetDLLHandle(void);
double __stdcall DelcomGetDLLVersion(void);
DWORD __stdcall  DelcomGetDLLDate(LPSTR);
DWORD __stdcall DelcomVerboseControl( DWORD Mode, LPSTR Header );

DWORD __stdcall DelcomGetNthDevice(DWORD, DWORD, LPSTR);
DWORD __stdcall DelcomScanDevices( DWORD, DeviceNameStruct[], DWORD );
HANDLE __stdcall DelcomOpenDevice( LPSTR, DWORD);
DWORD __stdcall DelcomCloseDevice(HANDLE);
DWORD __stdcall DelcomGetDeviceCount( DWORD Type );

DWORD __stdcall DelcomReadDeviceVersion(HANDLE  );
DWORD __stdcall DelcomReadDeviceSerialNum(LPSTR, HANDLE );
DWORD __stdcall DelcomSendPacket( HANDLE, pPacketStruct, pPacketStruct); 


// USBDELVI - Visual Indicator Functions
DWORD __stdcall DelcomLEDControl( HANDLE hFile, DWORD Color, DWORD Mode );
DWORD __stdcall DelcomEnableAutoConfirm(HANDLE hUsb, DWORD Mode);
DWORD __stdcall DelcomEnableAutoClear(HANDLE hUsb, DWORD Mode);
DWORD __stdcall DelcomBuzzer(HANDLE hUsb, uchar Mode, uchar Freq, uchar Repeat, uchar OnTime, uchar OffTime);
DWORD __stdcall DelcomLoadInitialPhaseDelay(HANDLE hUsb, BYTE Color, BYTE Delay);
DWORD __stdcall DelcomSyncLeds(HANDLE hUsb);
DWORD __stdcall DelcomLoadPreScalar(HANDLE hUsb, BYTE PreScalar);
DWORD __stdcall DelcomLoadLedFreqDuty(HANDLE hUsb, BYTE Color, BYTE Low, BYTE High);
DWORD __stdcall DelcomGetButtonStatus(HANDLE hUsb );
DWORD __stdcall DelcomReadVersionNumber(HANDLE hUsb );
DWORD __stdcall DelcomReadSerialNumber(HANDLE hUsb );
DWORD __stdcall DelcomLEDPower( HANDLE hUsb, DWORD Color, DWORD Power );
	


// USBIODS  -  USB IO Functions
DWORD __stdcall DelcomWritePortPin(HANDLE hUsb, BYTE Port, BYTE Pin, BYTE Value );
DWORD __stdcall DelcomWritePorts(HANDLE hUsb, BYTE Port0, BYTE Port1 );
DWORD __stdcall DelcomReadPorts(HANDLE hUsb, BYTE* Port0, BYTE* Port1 );

DWORD __stdcall DelcomWrite64Bit(HANDLE hUsb, LPSTR DataExt );
DWORD __stdcall DelcomRead64Bit(HANDLE hUsb, LPSTR DataExt );

DWORD __stdcall DelcomWriteI2C(HANDLE hUsb, BYTE CmdAdd, BYTE Length, LPSTR DataExt );
DWORD __stdcall DelcomReadI2C(HANDLE hUsb, BYTE CmdAdd, BYTE Length, LPSTR DataExt );
DWORD __stdcall DelcomSelReadI2C(HANDLE hUsb, BYTE SetAddCmd, BYTE Address, BYTE ReadCmd, BYTE Length, LPSTR DataExt );
DWORD __stdcall DelcomWriteI2CEEPROM(HANDLE hUsb, DWORD Address, DWORD Length, BYTE CtrlCode, BYTE WriteDelay, LPSTR pData );
DWORD __stdcall DelcomReadI2CEEPROM(HANDLE hUsb, DWORD Address, DWORD Length, BYTE CtrlCode, LPSTR pData );


DWORD __stdcall DelcomRS232Ctrl(HANDLE hUsb, DWORD Mode, DWORD Value );
DWORD __stdcall DelcomWriteRS232(HANDLE hUsb, DWORD Length, LPSTR DataExt );
DWORD __stdcall DelcomReadRS232(HANDLE hUsb, LPSTR DataExt );

	// SPI Commands 
DWORD __stdcall DelcomSPIWrite( HANDLE hUsb, DWORD ClockCount, LPSTR pData);
DWORD __stdcall DelcomSPISetClock( HANDLE hUsb, DWORD ClockPeriod );
DWORD __stdcall DelcomSPIRead(HANDLE hUsb,  LPSTR pData);
DWORD __stdcall DelcomSPIWr8Read64( HANDLE hUsb, DWORD WrData, DWORD ClockCount, LPSTR pData);



// USBNDSPY Functions
DWORD __stdcall DelcomNumericMode(HANDLE hUsb, BYTE Mode, BYTE Rate );
DWORD __stdcall DelcomNumericScanRate(HANDLE hUsb, BYTE ScanRate );
DWORD __stdcall DelcomNumericSetup(HANDLE hUsb, BYTE Digits );
DWORD __stdcall DelcomNumericRaw(HANDLE hUsb, LPSTR Str );
DWORD __stdcall DelcomNumericInteger(HANDLE hUsb, DWORD Number, DWORD Base );
DWORD __stdcall DelcomNumericHexaDecimal(HANDLE hUsb, DWORD Number, DWORD Base );
DWORD __stdcall DelcomNumericDouble(HANDLE hUsb, double Number, DWORD Base );


#endif



