#ifndef __USBM_H__
#define __USBM_H__

class USBio
{
private:
	HINSTANCE m_hDll;

	// Discovery routine
	typedef int (__stdcall *USBm_FindDevices_type) ();

	// Info about devices
	typedef int (__stdcall *USBm_NumberOfDevices_type)   (void);
	typedef int (__stdcall *USBm_DeviceValid_type)   (unsigned char);
	typedef int (__stdcall *USBm_DeviceVID_type)   (unsigned char device);
	typedef int (__stdcall *USBm_DevicePID_type)   (unsigned char device);
	typedef int (__stdcall *USBm_DeviceDID_type)   (unsigned char device);
	typedef int (__stdcall *USBm_DeviceMfr_type)   (unsigned char, char *);
	typedef int (__stdcall *USBm_DeviceProd_type)   (unsigned char, char *);
	typedef int (__stdcall *USBm_DeviceSer_type)   (unsigned char, char *);

	// General U4xx functions
	typedef int (__stdcall *USBm_InitPorts_type)  (unsigned char);
	typedef int (__stdcall *USBm_WriteA_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_WriteB_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_WriteABit_type)  (unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_WriteBBit_type)  (unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_ReadA_type)  (unsigned char, unsigned char *);
	typedef int (__stdcall *USBm_ReadB_type)  (unsigned char, unsigned char *);
	typedef int (__stdcall *USBm_SetBit_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_ResetBit_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_DirectionA_type)  (unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_DirectionB_type)  (unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_StrobeWrite_type)  (unsigned char, unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_StrobeRead_type)  (unsigned char, unsigned char *, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_StrobeWrite2_type)  (unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_StrobeRead2_type)  (unsigned char, unsigned char *, unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_StrobeWrites_type)  (unsigned char, unsigned char *, unsigned char *);
	typedef int (__stdcall *USBm_StrobeReads_type)  (unsigned char, unsigned char *, unsigned char *);
	typedef int (__stdcall *USBm_InitLCD_type)  (unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_LCDCmd_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_LCDData_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_InitSPI_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_SPIMaster_type)  (unsigned char, unsigned char *, unsigned char *);
	typedef int (__stdcall *USBm_SPISlaveWrite_type)  (unsigned char, unsigned char, unsigned char *);
	typedef int (__stdcall *USBm_SPISlaveRead_type)  (unsigned char, unsigned char *, unsigned char *);
	typedef int (__stdcall *USBm_Stepper_type)  (unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
	typedef int (__stdcall *USBm_Reset1Wire_type)  (unsigned char, unsigned char *);
	typedef int (__stdcall *USBm_Write1Wire_type)  (unsigned char, unsigned char);
	typedef int (__stdcall *USBm_Read1Wire_type)  (unsigned char, unsigned char *);

	// DLL string info access
	typedef int (__stdcall *USBm_RecentError_type)  (char *);
	typedef int (__stdcall *USBm_ClearRecentError_type)  (void);
	typedef int (__stdcall *USBm_DebugString_type)  (char *);
	typedef int (__stdcall *USBm_Copyright_type)  (char *);
	typedef int (__stdcall *USBm_About_type)  (char *);
	typedef int (__stdcall *USBm_Version_type)  (char *);

public:
	USBio() :  m_hDll(0)
	{
		m_hDll = LoadLibrary("USBm.dll");
		if (m_hDll==0) throw_("could not locate USBm.dll");

		USBm_FindDevices =      (USBm_FindDevices_type)GetProcAddress(m_hDll, "USBm_FindDevices");
		USBm_NumberOfDevices =  (USBm_NumberOfDevices_type)GetProcAddress(m_hDll, "USBm_NumberOfDevices");
		USBm_DeviceValid =      (USBm_DeviceValid_type)GetProcAddress(m_hDll, "USBm_DeviceValid");
		USBm_DeviceVID =        (USBm_DeviceVID_type)GetProcAddress(m_hDll, "USBm_DeviceVID");
		USBm_DevicePID =        (USBm_DevicePID_type)GetProcAddress(m_hDll, "USBm_DevicePID");
		USBm_DeviceDID =        (USBm_DeviceDID_type)GetProcAddress(m_hDll, "USBm_DeviceDID");
		USBm_DeviceMfr =        (USBm_DeviceMfr_type)GetProcAddress(m_hDll, "USBm_DeviceMfr");
		USBm_DeviceProd =       (USBm_DeviceProd_type)GetProcAddress(m_hDll, "USBm_DeviceProd");
		USBm_DeviceSer =        (USBm_DeviceSer_type)GetProcAddress(m_hDll, "USBm_DeviceSer");
		USBm_InitPorts =        (USBm_InitPorts_type)GetProcAddress(m_hDll, "USBm_InitPorts");
		USBm_WriteA =           (USBm_WriteA_type)GetProcAddress(m_hDll, "USBm_WriteA");
		USBm_WriteB =           (USBm_WriteB_type)GetProcAddress(m_hDll, "USBm_WriteB");
		USBm_WriteABit =        (USBm_WriteABit_type)GetProcAddress(m_hDll, "USBm_WriteABit");
		USBm_WriteBBit =        (USBm_WriteBBit_type)GetProcAddress(m_hDll, "USBm_WriteBBit");
		USBm_ReadA =            (USBm_ReadA_type)GetProcAddress(m_hDll, "USBm_ReadA");
		USBm_ReadB =            (USBm_ReadB_type)GetProcAddress(m_hDll, "USBm_ReadB");
		USBm_SetBit =           (USBm_SetBit_type)GetProcAddress(m_hDll, "USBm_SetBit");
		USBm_ResetBit =         (USBm_ResetBit_type)GetProcAddress(m_hDll, "USBm_ResetBit");
		USBm_DirectionA =       (USBm_DirectionA_type)GetProcAddress(m_hDll, "USBm_DirectionA");
		USBm_DirectionB =       (USBm_DirectionB_type)GetProcAddress(m_hDll, "USBm_DirectionB");
		USBm_StrobeWrite =      (USBm_StrobeWrite_type)GetProcAddress(m_hDll, "USBm_StrobeWrite");
		USBm_StrobeRead =       (USBm_StrobeRead_type)GetProcAddress(m_hDll, "USBm_StrobeRead");
		USBm_StrobeWrite2 =     (USBm_StrobeWrite2_type)GetProcAddress(m_hDll, "USBm_StrobeWrite2");
		USBm_StrobeRead2 =      (USBm_StrobeRead2_type)GetProcAddress(m_hDll, "USBm_StrobeRead2");
		USBm_StrobeWrites =     (USBm_StrobeWrites_type)GetProcAddress(m_hDll, "USBm_StrobeWrites");
		USBm_StrobeReads =      (USBm_StrobeReads_type)GetProcAddress(m_hDll, "USBm_StrobeReads");
		USBm_InitLCD =          (USBm_InitLCD_type)GetProcAddress(m_hDll, "USBm_InitLCD");
		USBm_LCDCmd =           (USBm_LCDCmd_type)GetProcAddress(m_hDll, "USBm_LCDCmd");
		USBm_LCDData =          (USBm_LCDData_type)GetProcAddress(m_hDll, "USBm_LCDData");
		USBm_InitSPI =          (USBm_InitSPI_type)GetProcAddress(m_hDll, "USBm_InitSPI");
		USBm_SPIMaster =        (USBm_SPIMaster_type)GetProcAddress(m_hDll, "USBm_SPIMaster");
		USBm_SPISlaveWrite =    (USBm_SPISlaveWrite_type)GetProcAddress(m_hDll, "USBm_SPISlaveWrite");
		USBm_SPISlaveRead =     (USBm_SPISlaveRead_type)GetProcAddress(m_hDll, "USBm_SPISlaveRead");
		USBm_Stepper =          (USBm_Stepper_type)GetProcAddress(m_hDll, "USBm_Stepper");
		USBm_Reset1Wire =       (USBm_Reset1Wire_type)GetProcAddress(m_hDll, "USBm_Reset1Wire");
		USBm_Write1Wire =       (USBm_Write1Wire_type)GetProcAddress(m_hDll, "USBm_Write1Wire");
		USBm_Read1Wire =        (USBm_Read1Wire_type)GetProcAddress(m_hDll, "USBm_Read1Wire");
		USBm_RecentError =      (USBm_RecentError_type)GetProcAddress(m_hDll, "USBm_RecentError");
		USBm_ClearRecentError = (USBm_ClearRecentError_type)GetProcAddress(m_hDll, "USBm_ClearRecentError");
		USBm_DebugString =      (USBm_DebugString_type)GetProcAddress(m_hDll, "USBm_DebugString");
		USBm_Copyright =        (USBm_Copyright_type)GetProcAddress(m_hDll, "USBm_Copyright");
		USBm_About =            (USBm_About_type)GetProcAddress(m_hDll, "USBm_About");
		USBm_Version =          (USBm_Version_type)GetProcAddress(m_hDll, "USBm_Version");
	}

	~USBio()
	{
		assert( m_hDll );
// XXX		FreeLibrary(m_hDll);
		m_hDll=0;
	}

	int Devices()  const 
	{
		assert( m_hDll );
		unsigned char numdev=USBm_NumberOfDevices();
		return numdev;
	}

	string version() const 
	{	
		assert( m_hDll );

		string s;
		char textstr[300];
		const int d=Devices();

		s  = string("    USBm.dll version  = ") + USBm_Version(textstr) + "\n";
		s += string("    USBm.dll version  = ") + textstr + "\n";

		USBm_Copyright(textstr);
		s += string("    Copyright         = ") + textstr + "\n";

	    USBm_About(textstr);
		s += string("    About             =") + textstr + "\n";
		s += string("    Number of devices = ") + d + "\n";

		 // Gather info from each discovered device
		for(int i=0;i<d;++i){
			s += string("    Device[") + i + "]:\n";
			s += string("      VID    = ") + USBm_DeviceVID(i) + "\n";
			USBm_DeviceMfr(i,textstr);
			s += string("      Manuf  = ") + textstr + "\n";
			s += string("      PID    = ") + USBm_DevicePID(i) + "\n";
			USBm_DeviceProd(i,textstr);
			s += string("      Prod   = ") + textstr + "\n";
			s += string("      DID    = ") + USBm_DeviceDID(i) + "\n";
			USBm_DeviceSer(i,textstr);
			s += string("      Serial = ") + textstr + "\n";
		}
		return s;
	}

	USBm_FindDevices_type       USBm_FindDevices;
	USBm_NumberOfDevices_type   USBm_NumberOfDevices;
	USBm_DeviceValid_type       USBm_DeviceValid;
	USBm_DeviceVID_type         USBm_DeviceVID;
	USBm_DevicePID_type         USBm_DevicePID;
	USBm_DeviceDID_type         USBm_DeviceDID;
	USBm_DeviceMfr_type         USBm_DeviceMfr;
	USBm_DeviceProd_type        USBm_DeviceProd;
	USBm_DeviceSer_type         USBm_DeviceSer;
	USBm_InitPorts_type         USBm_InitPorts;
	USBm_WriteA_type            USBm_WriteA;
	USBm_WriteB_type            USBm_WriteB;
	USBm_WriteABit_type         USBm_WriteABit;
	USBm_WriteBBit_type         USBm_WriteBBit;
	USBm_ReadA_type             USBm_ReadA;
	USBm_ReadB_type             USBm_ReadB;
	USBm_SetBit_type            USBm_SetBit;
	USBm_ResetBit_type          USBm_ResetBit;
	USBm_DirectionA_type        USBm_DirectionA;
	USBm_DirectionB_type        USBm_DirectionB;
	USBm_StrobeWrite_type       USBm_StrobeWrite;
	USBm_StrobeRead_type        USBm_StrobeRead;
	USBm_StrobeWrite2_type      USBm_StrobeWrite2;
	USBm_StrobeRead2_type       USBm_StrobeRead2;
	USBm_StrobeWrites_type      USBm_StrobeWrites;
	USBm_StrobeReads_type       USBm_StrobeReads;
	USBm_InitLCD_type           USBm_InitLCD;
	USBm_LCDCmd_type            USBm_LCDCmd;
	USBm_LCDData_type           USBm_LCDData;
	USBm_InitSPI_type           USBm_InitSPI;
	USBm_SPIMaster_type         USBm_SPIMaster;
	USBm_SPISlaveWrite_type     USBm_SPISlaveWrite;
	USBm_SPISlaveRead_type      USBm_SPISlaveRead;
	USBm_Stepper_type           USBm_Stepper;
	USBm_Reset1Wire_type        USBm_Reset1Wire;
	USBm_Write1Wire_type        USBm_Write1Wire;
	USBm_Read1Wire_type         USBm_Read1Wire;
	USBm_RecentError_type       USBm_RecentError;
	USBm_ClearRecentError_type  USBm_ClearRecentError;
	USBm_DebugString_type       USBm_DebugString;
	USBm_Copyright_type         USBm_Copyright;
	USBm_About_type             USBm_About;
	USBm_Version_type           USBm_Version;
};

#endif // __USBM_H__