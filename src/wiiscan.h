// Copyright © 2009 MergeIt, Aps.
//
// License LGPLv3+: GNU lesser LGPL version 3 or later <http://gnu.org/licenses/lgpl.html>.
// This is free software: you are free to change and redistribute it.
// There is NO WARRANTY, to the extent permitted by law.
//
//	This file is part of wiiscan.
//
//  wiiscan is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  any later version.
//
//  wiiscan is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with wiiscan.  If not, see <http://www.gnu.org/licenses/>.

#define VERSION     0
#define VERSION_REV 9

#define VC_EXTRALEAN

#include "toolsfun.h"
#include "wiiuse.h"
#include "usbm.h"
#include "delcomdll.h"
#include "USBDevice.h"
#include <bthdef.h>
#include <BluetoothAPIs.h>

#pragma comment ( lib, "Lib/wiiuse.lib")
#pragma comment ( lib, "Lib/delcomdll.lib")
#pragma comment ( lib, "Irprops.lib")
#pragma comment ( lib, "Ws2_32.lib")

namespace Wiiscan {

// default values
const int MAX_DEVICE_COUNT  =256;
const int DEFAULT_timeout   =2000;
const int DEFAULT_wiitimeout=2000;
const int DEFAULT_usbsleep  =500;
const int DEFAULT_usbmsleep =3000;
const int DEFAULT_btsleep   =2000;
const int DEFAULT_usbmode   =0;
const int DEFAULT_removemode=0;
const int DEFAULT_scanretries=1;
const string NINTENDO_DEV="Nintendo RVL-CNT-01";
const string DEFAULT_configfile="wiiscan.ini";
const string DEFAULT_logfile="cout";
const string DEFAULT_wiilib="wiimotelib";

Logger& log=g_log;
void (*g_automode_callback)(const int)=0;
void DummyCallback(const int){FUNSTACK;}

string ConvertName(const WCHAR* szName)
{
	FUNSTACK;
	assert( MAX_PATH >= BLUETOOTH_MAX_NAME_SIZE );
	CHAR szDevName[MAX_PATH];
	memset(szDevName, 0x00, sizeof(szDevName));
	sprintf_s(szDevName,MAX_PATH,"%S", szName);
	return string(szDevName);
}

string ConvertAddress(const BLUETOOTH_ADDRESS& address)
{
	FUNSTACK;
	string s;
	char t[256];
	for(int i=5;i>=0;i--) {
		sprintf_s(t,256,"%X",address.rgBytes[i]);
		string x=t;
		if (x.size()==1) x="0"+x;
		s+=x+(i==0 ? "" : ":");
	} 
	return s;
}

string tostring(const SYSTEMTIME& x)
{
	FUNSTACK;
	// XXX warning: no converstion of GMT to local time
	ostringstream s;
	if (x.wHour==0 && x.wMinute==0 && x.wDay==0 && x.wYear==0 && x.wYear==0) s << "<N/A>";
	else s << x.wHour << ":" << (x.wMinute<10 ? "0" : "") <<  x.wMinute << " d. " << x.wDay << "/" << x.wMonth << ", " << x.wYear;
	return s.str();
}

string tostring(const BLUETOOTH_DEVICE_INFO& x)
{	
	FUNSTACK;
	ostringstream s;
	s << "  BLUETOOTH_DEVICE_INFO:" << endl;
	s << "    dwSize:         " << x.dwSize << endl;
	s << "    Address:        " << ConvertAddress(x.Address) << endl;
	s << "    ulClassofDevice:" << x.ulClassofDevice << endl;
	s << "    fConnected:     " << x.fConnected << endl;	
	s << "    fRemembered:    " << x.fRemembered << endl;   
	s << "    fAuthenticated: " << x.fAuthenticated << endl;
	s << "    stLastSeen:     " << tostring(x.stLastSeen) << " [GMT]" << endl;  
	s << "    stLastUsed:     " << tostring(x.stLastUsed) << " [GMT]" << endl;
	s << "    szName:         " << ConvertName(x.szName) << endl;
	return s.str();
}

string tostring(const BLUETOOTH_RADIO_INFO& x)
{	
	FUNSTACK;
	ostringstream s;
	s << "  PBLUETOOTH_RADIO_INFO:" << endl;
	s << "    dwSize:         " << x.dwSize << endl;
	s << "    Address:        " << ConvertAddress(x.address) << endl;
	s << "    szName:         " << ConvertName(x.szName) << endl;
	s << "    ulClassofDevice:" << x.ulClassofDevice  << endl; 
	s << "    lmpSubversion:  " << x.lmpSubversion  << endl;	
	s << "    manufacturer:   " << x.manufacturer << endl;   
	return s.str();
}

bool USB_hub_updown(const bool up,const string& usbhub)
{
	FUNSTACK;
	log << (up ? "Enabling" : "Disabling") << " 'USB hub..." << endl;	
	int n=-1;
	System(string("devcon ") + (up ? "enable " : "disable ") + usbhub,false,false,&n);
	if (n!=0) throw_("devcon failed, this can be to a erroneous usbhub string or if devcon.exe is not found in path, please install it from http://support.microsoft.com/kb/311272");
	log << "Done [OK]" << endl;
	return true;
}

int ScanforUSBmicrodevs()
{
	USBio io;
	io.USBm_FindDevices();
	return io.Devices();
}

bool USB_microio_updown(const bool up,const bool dbg)
{
	FUNSTACK;
	log << (up ? "Enabling" : "Disabling") << " USBm io..." << endl;	
	
    // Discover the USBmicro devices
	USBio io;
	io.USBm_FindDevices();
	const int d=io.Devices();
	if      (d==0) return false; // throw_("could not find an USBm device");
	else if (d>1)  throw_("found more that one USBm devices");

	static bool premable=true;
	if (dbg && premable) {
		premable=false;
		log << "  USBm info:" << endl << io.version();
	}

    io.USBm_InitPorts(0);
    io.USBm_DirectionA(0, 0xff, 0xff);

	if(up) io.USBm_ResetBit(0,5);
	else   io.USBm_SetBit  (0,5);

	log << "Done [OK]" << endl;
	return true;
}

bool __stdcall CloseDelcomhandle(HANDLE h){return DelcomCloseDevice(h)==0;}

void do_cmd(HANDLE di, int ma, int mi, int lsb, int msb, unsigned char *data, unsigned datalen)
{
	//static struct delcom_packet p;
	PacketStruct p;
	memset(&p, 0, sizeof(p));
	p.MajorCmd = ma;
	p.MinorCmd = mi;
	p.DataLSB = lsb;
	p.DataMSB = msb;
	if (DelcomSendPacket(di,&p,&p)<0) throw_("DelcomSendPacket() failed");
}

int ScanforDelcomdevs()
{
	// Discover the USB Delcom devices
	DeviceNameStruct names[10];  // array to hold the device names found
	return DelcomScanDevices(USBIODS, names, 10);
}

bool USB_delcomio_updown(const bool up,const bool dbg)
{
	FUNSTACK;
	log << (up ? "Enabling" : "Disabling") << " USB Delcom io..." << endl;	

	DeviceNameStruct names[10];  // array to hold the device names found
	const int d=DelcomScanDevices(USBIODS, names, 10);
	if      (d==0) return false; //throw_("could not find an USB Delcon device");
	else if (d>1)  throw_("found more that one USB Delcon devices");
	
	static bool premable=true;
	if (dbg && premable) {
		premable=false;
		log << "  USB Delcon info:" << (char*)&names[0] << "\n  SN=" << DelcomReadDeviceSerialNum((char*)&names[0], NULL) << endl;
	}

	DeviceAutoClose<HANDLE,bool> h(DelcomOpenDevice((char*)&names[0],0),CloseDelcomhandle);	
	if(up) do_cmd(h(),10,1,0xFE,0,0,0);
	else   do_cmd(h(),10,1,0xFF,0,0,0);

	log << "Done [OK]" << endl;
	return true;
}

#ifdef USE_XANDER_USB_FRAMEWORK

bool USB_xander_updown(const bool up,const bool dbg)
{
	FUNSTACK;
	log << (up ? "Enabling" : "Disabling") << " USB Xander io..." << endl;	

	USBDevice d;
	if (!d.Connect(0x04d8,0xff39)) 	return false; //throw_("could not find an USB Xander device");
	
	BYTE bytes[8];

	//int i=0;
	//if (d.Read(0,bytes,3)!=3) throw_("could not read from xander device");
	//log << "##  Read(" << i << "): {" << (int)bytes[0] << ";" << (float)((unsigned int)bytes[1]*256+bytes[2])/1024 << " " << (unsigned int)bytes[1] << " " << (unsigned int)bytes[2] << "}" << endl; 
	//Sleep(10);

	bytes[0]=up ? 'a' : 'b';
	bytes[1]=0;

	if (d.Write(0,bytes,1)!=1) throw_("could not write to xander device");

	//for(;i<10;++i){
	//	//Sleep(10);
	//	if (d.Read(0,bytes,3)!=3) throw_("could not read from xander device");
	//	log << "##  Read(" << i << "): {" << (int)bytes[0] << ";" << (float)((unsigned int)bytes[1]*256+bytes[2])/1024 << " " << (unsigned int)bytes[1] << " " << (unsigned int)bytes[2] << "}" << endl; 
	//}

	return true;
}
#else

bool USB_xander_updown(const bool up,const bool dbg)
{
	FUNSTACK;
	log << (up ? "Enabling" : "Disabling") << " USB Xander io..." << endl;	
	throw_("module not enabled in compiled code (USE_XANDER_USB_FRAMEWORK)");
	return false;
}

#endif

bool USBupdown(const int iomode,const bool up,const bool dbg,const string& usbhub="")
{
	if      (iomode==0) return true;
	if      (iomode==1) return USB_hub_updown(up,usbhub);
	if      (iomode==2) return USB_microio_updown(up,dbg);
	if      (iomode==3) return USB_delcomio_updown(up,dbg);
	else if (iomode==4) return USB_xander_updown(up,dbg);
	else throw_("bad io mode");
	return false;
}

class Wiilib
{
	// Wiilib class based on wiiuse library by Michael Laforest.
	// Library located at http://www.wiiuse.net
	// Wiiuse premable from wiiuse_v0.12/example/example.c:

	//
	//	wiiuse
	//
	//	Written By:
	//		Michael Laforest	< para >
	//		Email: < thepara (--AT--) g m a i l [--DOT--] com >
	//
	//	Copyright 2006-2007
	//
	//	This file is part of wiiuse.
	//
	//	This program is free software; you can redistribute it and/or modify
	//	it under the terms of the GNU General Public License as published by
	//	the Free Software Foundation; either version 3 of the License, or
	//	(at your option) any later version.
	//
	//	This program is distributed in the hope that it will be useful,
	//	but WITHOUT ANY WARRANTY; without even the implied warranty of
	//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	//	GNU General Public License for more details.
	//
	//	You should have received a copy of the GNU General Public License
	//	along with this program.  If not, see <http://www.gnu.org/licenses/>.

private:
	enum {MAX_WIIMOTES=1}; // use only one wiimote, but can connect to four
	wiimote** m_wiimotes;
	static int m_instantiations;

	int finddevices(const int wiitimeout,const bool rumble,const bool dbg) 
	{	
		FUNSTACK;

		//	Find wiimote devices
		const int to=wiitimeout/1000;
		if (to<1) throw_("bad wiitimeout value, should be >=1000");
		const int found=wiiuse_find(m_wiimotes,MAX_WIIMOTES,to);
		if (dbg) log << "  wiiuse_find(0x0," << MAX_WIIMOTES << "," << to << ") return " << found << endl; 
		
		if (found==0) {
			if (dbg) log << "  ** error: no wiimotes found" << endl;
			return 0;
		}

		// Connect to the wiimotes
		assert(found>0);

		const int connected = wiiuse_connect(m_wiimotes, MAX_WIIMOTES);
		if (connected==0){
			log << "  ** error: could not connect to any wiimotes" << endl;
			return 0;
		}
		if (dbg) log  << "  connected to " << connected<< " wiimotes (of " << MAX_WIIMOTES << " found)" << endl;


		// Now set the LEDs and rumble for a second so it's easy
		// to tell which wiimotes are connected (just like the wii does).
		wiiuse_set_leds(m_wiimotes[0], WIIMOTE_LED_1);
	
		if (rumble){
			wiiuse_rumble(m_wiimotes[0], 1);
			#ifndef WIN32
				usleep(200000);
			#else	
				Sleep(200);
			#endif
			wiiuse_rumble(m_wiimotes[0], 0);
		}

		// read some data to be sure of connection
		for(int i=0;i<8;++i){
			Sleep(50);

			int led=0;
			switch(i%6){
				case 0 : led=WIIMOTE_LED_1; break;
				case 1 : led=WIIMOTE_LED_2; break;
				case 2 : led=WIIMOTE_LED_3; break;
				case 3 : led=WIIMOTE_LED_4; break;
				case 4 : led=WIIMOTE_LED_3; break;
				case 5 : led=WIIMOTE_LED_2; break;
			}
			wiiuse_rumble(m_wiimotes[0], 0); // to be sure
			wiiuse_set_leds(m_wiimotes[0],led);
			wiiuse_status(m_wiimotes[0]);
			if (led!=m_wiimotes[0]->leds) throw_("bad LED status on wiimote, is it connected properly?");
		}

		// set all leds, retry a couple of time to be sure
		for(int i=0;i<4;++i){
			wiiuse_set_leds(m_wiimotes[0],WIIMOTE_LED_1 | WIIMOTE_LED_2 | WIIMOTE_LED_3 | WIIMOTE_LED_4);
			Sleep(100);
		}

		return connected;
	}	

	Wiilib() : m_wiimotes(0)
	{
		FUNSTACK;
		if (m_instantiations!=0) throw_("can only instatiate one Wiilib at a time");
		++m_instantiations; // assumes a single-thread application, else we have a race condition here

		// Initialize an array of wiimote objects.
		// The parameter is the number of wiimotes I want to create.
		m_wiimotes =  wiiuse_init(MAX_WIIMOTES);
		// wiiuse_set_bluetooth_stack(m_wiimotes, MAX_WIIMOTES,WIIUSE_STACK_MS); must be done automatically
		assert(m_instantiations==1);
	}

	~Wiilib() 
	{
		FUNSTACK;
		assert( m_instantiations==1 );

		// Disconnect the wiimotes
		if (m_wiimotes!=0) wiiuse_cleanup(m_wiimotes, MAX_WIIMOTES);
		m_wiimotes=0;
		--m_instantiations;
		assert(m_instantiations==0);
	}
	
	static int OpenDevices_wiilib(const int wiitimeout,const bool rumble,const bool dbg,const bool precheck=false)
	{
		FUNSTACK;
		int n=0,retries=0;
		while(n==0 && retries++<10) {
			n=Wiilib().finddevices(wiitimeout,rumble,dbg);
			if (precheck) break;
			if (n==0) { 
				if (dbg) log << "  OpenDevice()...delaying " << (retries<5 ? "250" : "1000") << " [ms]" << endl;
				if (retries<5) Sleep(250);
				else Sleep(1000);
			}
		}
		if (dbg) {cerr.flush(); log << "  Wiilib::OpenDevice..." << n << endl;}
		return n;
	}

	static int OpenDevices_wiimotelib(const int wiitimeout,const bool rumble,const bool dbg,const bool precheck=false)
	{
		FUNSTACK;
		int n=0,retries=0;
		while(n==0 && retries++<10) {
			n=system("wiimotelibpoll")==0 ? 1 : 0;
			if (precheck) break;
			if (n==0) { 
				if (dbg) log << "  OpenDevice()...delaying " << (retries<5 ? "250" : "1000") << " [ms]" << endl;
				if (retries<5) Sleep(250);
				else Sleep(1000);
			}
		}
		if (dbg) {cerr.flush(); log << "  Wiilib::OpenDevice..." << n << endl;}
		return n;
	}

public:

	static int OpenDevices(const string& wiilib,const int wiitimeout,const bool rumble,const bool dbg,const bool precheck=false)
	{
		FUNSTACK;
		if      (wiilib=="wiiuse")     return OpenDevices_wiilib(wiitimeout,rumble,dbg,precheck);
		else if (wiilib=="wiimotelib") return OpenDevices_wiimotelib(wiitimeout,rumble,dbg,precheck);
		else if (wiilib=="none")       return -1;
		else throw_("Wiilib::OpenDevices(), library must be one-of {wiiuse,wiimotelib,none}");
		return -1; // avoid compiler error
	}

};

int Wiilib::m_instantiations=0;

BLUETOOTH_DEVICE_SEARCH_PARAMS Get_BLUETOOTH_DEVICE_SEARCH_PARAMS(const int timeout)
{
	FUNSTACK;
	BLUETOOTH_DEVICE_SEARCH_PARAMS b;
	ZeroMemory(&b, sizeof(b));
	b.dwSize = sizeof(b);
	b.fReturnAuthenticated = TRUE;
	b.fReturnRemembered = TRUE;
	b.fReturnUnknown = TRUE;
	b.fReturnConnected = TRUE;
	b.fIssueInquiry = TRUE;

	const int to=static_cast<int>(1.0*timeout/1000.0/1.28+0.5);
	assert(to>=0 && to<48 );
	b.cTimeoutMultiplier = (to==0 ? 1 : to);  // timeout of 0 causes havac
	b.hRadio = NULL;

	assert( b.cTimeoutMultiplier>0 );
	return b;
}

BLUETOOTH_DEVICE_INFO Get_BLUETOOTH_DEVICE_INFO()
{
	FUNSTACK;
	BLUETOOTH_DEVICE_INFO b;
	ZeroMemory(&b, sizeof(b));
	b.dwSize = sizeof(b);
	return b;
}

BLUETOOTH_RADIO_INFO Get_BLUETOOTH_RADIO_INFO()
{
	FUNSTACK;
	BLUETOOTH_RADIO_INFO b;
	ZeroMemory(&b, sizeof(b));
	b.dwSize = sizeof(b);
	return b;
}

BLUETOOTH_FIND_RADIO_PARAMS Get_BLUETOOTH_FIND_RADIO_PARAMS()
{
	FUNSTACK;
	BLUETOOTH_FIND_RADIO_PARAMS b;
	ZeroMemory(&b, sizeof(b));
	b.dwSize = sizeof(b);
	return b;
}

bool ReachedMax(const int i)
{
	FUNSTACK;
	if (i>MAX_DEVICE_COUNT) {
		log << "  ** warning: too many devices found, can only handle " << MAX_DEVICE_COUNT << " devices" << endl;
		return true;
	}
	return false;
}

bool RemoveDev(const BLUETOOTH_DEVICE_INFO& bdi) 
{
	FUNSTACK;
	DWORD status=BluetoothUpdateDeviceRecord(&bdi);
	if (status!=ERROR_SUCCESS) throw_("BluetoothUpdateDeviceRecord() failed");

	status=BluetoothRemoveDevice(&bdi.Address);
	if (status==ERROR_SUCCESS) log << "  removed device successfully..." << endl << "Done [OK]" << endl;
	else                       log << "  ** error: removing the device failed, was it registred at all?" << endl <<"Done [FAILED]" << endl;

	return status==ERROR_SUCCESS;
}
	
HANDLE RadioInfo(const string& btr,const bool dbg)
{
	FUNSTACK;
	if (dbg) log << "  Radio info" << endl;
	assert( btr.size()==17 || btr.size()==0 || btr=="all");

	HANDLE hRadio=NULL;
	BLUETOOTH_FIND_RADIO_PARAMS btfrp=Get_BLUETOOTH_FIND_RADIO_PARAMS();
	DeviceAutoClose<HBLUETOOTH_RADIO_FIND,BOOL> hbf(BluetoothFindFirstRadio(&btfrp,&hRadio),&BluetoothFindRadioClose);

	if (hbf()==NULL) { 
		if (dbg) log << "  <EMPTY>"  << endl; 
		return NULL;
	} 
	
	while(hbf()!=NULL){
		if (hbf()==NULL) throw_("BluetoothFindFirstRadio() failed");

		BLUETOOTH_RADIO_INFO r=Get_BLUETOOTH_RADIO_INFO();		
		if (BluetoothGetRadioInfo(hRadio,&r)!=ERROR_SUCCESS) throw_("BluetoothGetRadioInfo() failed");

		const int c=BluetoothIsConnectable(hRadio);
		const int d=BluetoothIsDiscoverable(hRadio);

		if (dbg){
			log << indent(tostring(r),"  ");
			log << "    connectable:    " << c << endl;
			log << "    discoverabel:   " << d << endl;
		}

		if (btr.size()==0) break;
		else if (btr==ConvertAddress(r.address)){
			if (dbg) log << "  found radio address match " << (btr.size()>0 ? ": " + btr : "") << endl;
			break;
		}

		if (!BluetoothFindNextRadio(&btfrp,&hRadio)) break;
	}
	
	if (dbg) log << "Done [" << (hRadio==NULL ? "FAILED" : "OK") << "]" << endl;
	return hRadio;
}

bool ScanDevices(const int timeout)
{		
	FUNSTACK;
	log << "Scanning devices"  << endl;

	vector<BLUETOOTH_DEVICE_INFO> infos;

	BLUETOOTH_DEVICE_SEARCH_PARAMS bdsp=Get_BLUETOOTH_DEVICE_SEARCH_PARAMS(timeout);
	BLUETOOTH_DEVICE_INFO bdi=Get_BLUETOOTH_DEVICE_INFO();

	DeviceAutoClose<HBLUETOOTH_DEVICE_FIND,BOOL> hbf(BluetoothFindFirstDevice(&bdsp,&bdi),&BluetoothFindDeviceClose);
	const DWORD dwErr = GetLastError();

	if (hbf()==NULL) {
		log << "  ** warning: no devices found"  << endl << "Done [FAILED]" << endl;
		return false;;
	}

	if (hbf()!= NULL) {
		log << "Looking for devices..." << endl;

		int i=0;
		while(1){
			if (ReachedMax(i)) break;

			const string t=ConvertName(bdi.szName);
			log << "  found device [" << i << "]: <" << (t.size()>0 ? t : "NONAME") << ">" << endl;

			infos.push_back(bdi);

			if (BluetoothFindNextDevice(hbf(), &bdi) == FALSE)
			break;

			++i;
		}
	}

	log << "BTDevices info:" << endl;
	if (infos.size()==0) log << "  <EMPTY>" << endl;
	else for(size_t i=0;i<infos.size();++i){
		const BLUETOOTH_DEVICE_INFO& t=infos[i]; 
		const string s=ConvertName(t.szName);
		log << "Device [" << i << "]: " << (s.size()>0 ? s : "<NONAME>") << endl;
		log << tostring(t);
	}
	log << "Done [OK]" << endl;

	return true;
} 

bool RemoveDevice(const string& d,const bool dbg)
{		
	FUNSTACK;
	log << "Removing device <" << d << ">"  << endl;

	BLUETOOTH_DEVICE_SEARCH_PARAMS bdsp=Get_BLUETOOTH_DEVICE_SEARCH_PARAMS(1); // timout must be greater than zero, eventhough I only return remembered devices!
	BLUETOOTH_DEVICE_INFO bdi=Get_BLUETOOTH_DEVICE_INFO();

	bdsp.fReturnAuthenticated = FALSE;
	bdsp.fReturnRemembered = TRUE;
	bdsp.fReturnUnknown = FALSE;
	bdsp.fReturnConnected = TRUE;

	DeviceAutoClose<HBLUETOOTH_DEVICE_FIND,BOOL> hbf(BluetoothFindFirstDevice(&bdsp,&bdi),&BluetoothFindDeviceClose);

	const DWORD dwErr = GetLastError();

	if (hbf()==NULL) {
		log << "  ** error: failed to find device" << endl << "Done [FAILED]" << endl;
		return false;
	}

	int i=0;
	while(true){
		if (ReachedMax(i)) break;
		const string fd=ConvertName(bdi.szName);
		log << "  found device [" << i << "]: <" << (fd.size()>0 ? fd : "NONAME") << ">" << endl;

		if (fd==d){
			log << "  found match..." << endl;
			log << "  trying to remove..." << endl;

			return RemoveDev(bdi);
		}
		if (BluetoothFindNextDevice(hbf(), &bdi) == FALSE) break;
		++i;
	}
	log << "  ** error: failed to find device" << endl << "Done [FAILED]" << endl;
	return false;
}

bool MatchAdr(const string& pattern,const string& adr)
{
	assert(pattern.size()==17 && adr.size()==17);
	for(size_t i=0;i<17;++i){
		if (!(pattern[i]=='?' || pattern[i]==adr[i])) return false;
	}
	return true;
}

bool MatchDevice(const string& d,const vector<string>& known_adr,const BLUETOOTH_DEVICE_INFO& bdi,const bool dbg)
{
	FUNSTACK;

	const string found_adr=ConvertAddress(bdi.Address);
	bool wildcard=false;
	for(size_t i=0;i<known_adr.size();++i) {
		if (known_adr[i].size()!=17) throw_("bad format in bluetooth adr, expected af form of 00:00:00:00:00:00, found " + known_adr[i]); 
		if (MatchAdr(known_adr[i],found_adr)) {
			if (dbg) log << "  match on adr " << found_adr << endl;
			return true;
		}
		else if (known_adr[i]=="00:00:00:00:00:00") wildcard=true;
	}
	if (wildcard || known_adr.size()==0) return d==ConvertName(bdi.szName);
	else return false;
}

struct BluetoothFindFirstDevice_struct
{
	int timeout;
	BLUETOOTH_DEVICE_INFO bdi;
	HBLUETOOTH_DEVICE_FIND hbf;
	volatile bool done;

	BluetoothFindFirstDevice_struct(const int t) : timeout(t), bdi(Get_BLUETOOTH_DEVICE_INFO()), hbf(0), done(false) {}
};

unsigned long __stdcall BluetoothFindFirstDevice_thread(void* p)
{
	// no funstack due to thread safety, FUNSTACK;

	BluetoothFindFirstDevice_struct* q=reinterpret_cast<BluetoothFindFirstDevice_struct*>(p);
	assert(p!=0 && q->done==false && q->timeout>0 && q->hbf==0 );

	BLUETOOTH_DEVICE_SEARCH_PARAMS bdsp=Get_BLUETOOTH_DEVICE_SEARCH_PARAMS(q->timeout);
  
	q->hbf=BluetoothFindFirstDevice(&bdsp,&q->bdi);
	const DWORD status=GetLastError();

	if      (status==ERROR_INVALID_PARAMETER) throw_("BluetoothFindFirstDevice(), returned status=ERROR_INVALID_PARAMETER");
	else if	(status==ERROR_REVISION_MISMATCH) throw_("BluetoothFindFirstDevice(), returned status=ERROR_REVISION_MISMATCH");

	q->done=true;
	return 0;
}

bool OpenDevice(const string& d,const string& btr,const int timeout,const int wiitimeout,const vector<string>& known_adr,const int btsleep,const string& lib,const bool precheckwiimotes,const bool rumble,const bool dbg,bool& invalidargflag)
{		
	FUNSTACK;
	assert( invalidargflag==false );
	log << "Open device <" << d << ">" << endl;

	if (precheckwiimotes && Wiilib::OpenDevices(lib,wiitimeout,false,dbg,true)>=1) {
		log << "  service is already opened..." << endl << "Done [OK]" << endl; 
		return true;
	}

	if (dbg && known_adr.size()>0) {
		log << "  known addresses=";
		for(size_t i=0;i<known_adr.size();++i) log << known_adr[i] << " ";
		log << endl;
	}	

	BluetoothFindFirstDevice_struct p(timeout);
	{
		// Fix of bug 15: BluetoothFindFirstDevice() stalls (FIXED)
		// The BluetoothFindFirstDevice() sometimes newer returns. Happens only for MS stack version 
		// "Microsoft BT stack: date 13-04-2008, driver 5.1.2600.5512" (eee stack).
		//
		// Code stalls here:
		//
		// DeviceAutoClose<HBLUETOOTH_DEVICE_FIND,BOOL> hbf(BluetoothFindFirstDevice(&bdsp,&bdi),&BluetoothFindDeviceClose);
		//
		// FIX: start the BluetoothFindFirstDevice() function in a thread, terminate the thread if it has run for longer than, say 2+timeout.
		//
		// Old code:
		// BLUETOOTH_DEVICE_SEARCH_PARAMS bdsp=Get_BLUETOOTH_DEVICE_SEARCH_PARAMS(timeout);
		// BLUETOOTH_DEVICE_INFO bdi=Get_BLUETOOTH_DEVICE_INFO();
		// DeviceAutoClose<HBLUETOOTH_DEVICE_FIND,BOOL> hbf(BluetoothFindFirstDevice(&bdsp,&bdi),&BluetoothFindDeviceClose);
		// DWORD status=GetLastError();

		unsigned long tid=0;
		DeviceAutoClose<HANDLE,BOOL> h(CreateThread(0,0,BluetoothFindFirstDevice_thread,&p,0,&tid),CloseHandle);	
		timer tthread;

		Sleep(timeout);
		while(!p.done && tthread.elapsed()*1000<2*timeout) Sleep(20); // or use WaitForMultipleObjects(...);
	}

	if(!p.done){
		log << "  ** error: BluetoothFindFirstDevice() stalled" << endl<< "Done [FAILED]" << endl;
		return false;
	}
	else if(p.hbf==0){
		log << "  ** error: BluetoothFindFirstDevice() returned null hbf" << endl<< "Done [FAILED]" << endl;
		return false;
	}

	assert(p.hbf!=0);
	BLUETOOTH_DEVICE_INFO bdi=p.bdi;
	DeviceAutoClose<HBLUETOOTH_DEVICE_FIND,BOOL> hbf(p.hbf,&BluetoothFindDeviceClose);	

	if (hbf()==NULL) {
		log << "  ** error: no devices found" << endl << "Done [FAILED]" << endl;
		return false;
	}

	int i=0;
	while(true){
		if (ReachedMax(i)) break;
		const string fd=ConvertName(bdi.szName);
		log << "  found device [" << i << "]: <" << (fd.size()>0 ? fd : "NONAME") << ">" << endl;
		
		if (MatchDevice(d,known_adr,bdi,dbg)){
			log << "  found match..." << endl;
			if (dbg) log << tostring(bdi);

			// write binary dbi, test code for now
			//ofstream ox("bdi.out",ios::binary);
			//ox.write(reinterpret_cast<const char*>(&bdi),sizeof(bdi));

			log << "  trying to open..." << endl;
			DWORD status=BluetoothUpdateDeviceRecord(&bdi);	
			if (status!=ERROR_SUCCESS) throw_("BluetoothUpdateDeviceRecord() failed");
			if (dbg) log << "  BluetoothUpdateDeviceRecord()...OK" << endl;

			{
				DeviceAutoClose<HANDLE,BOOL> hRadio(RadioInfo(btr,dbg),&CloseHandle);
				if (hRadio()==NULL) throw_("failed to get radio");
				if (dbg) log << "  RadioInfo()...OK" << endl;

				const GUID service=HumanInterfaceDeviceServiceClass_UUID;
				status=BluetoothSetServiceState(hRadio(),&bdi,&service,BLUETOOTH_SERVICE_ENABLE);

				if (dbg && status==ERROR_SUCCESS) log << "  BluetoothSetServiceState()...OK" << endl;
				if (dbg && status!=ERROR_SUCCESS) log << "  BluetoothSetServiceState()...ERROR" << endl;
			}

			if (status!=ERROR_SUCCESS) {				
				string t;
				if      (status==ERROR_INVALID_PARAMETER)      t="ERROR_INVALID_PARAMETER";
				else if (status==ERROR_SERVICE_DOES_NOT_EXIST) t="ERROR_SERVICE_DOES_NOT_EXIST";
				else if (status==E_INVALIDARG)                 t="E_INVALIDARG";
				// if      (status!=E_INVALIDARG)                 throw_("BluetoothSetServiceState() failed with errorcode " + t);
				// else    log << "  ** error: BluetoothSetServiceState() returned " <<  t << endl;
				
				if (status==E_INVALIDARG) invalidargflag=true;
				log << "  ** error: BluetoothSetServiceState() returned " <<  t << endl;
				log << endl << "Done [FAILED]" << endl; 
				return false;				
			}

			// sleep while windows slowly does strange things
			Sleep(btsleep);

			if (lib!="none")
			{
				if (Wiilib::OpenDevices(lib,wiitimeout,rumble,dbg)==0) {
					log << "  ** error: service could not be opened..." << endl<< "Done [FAILED]" << endl; 
					return false;
				}
			}

			log << "  service on device enabled..." << endl << "  opended device successfully..." << endl << "Done [OK]" << endl;
			return true;
		}


		if (BluetoothFindNextDevice(hbf(), &bdi) == FALSE) {
			
			break;
		}

		++i;
	}
	log << "  ** error: device not mathced" << endl<< "Done [FAILED]" << endl;
	return false;
} 

bool AutoOpenDevice(const string& d,const string& btr,const int timeout,const int wiitimeout,const int usbsleep,const int btsleep,const int usbmsleep,const string& cf,const string& lib,const int removemode,const int usbmode,const bool wb,const int scanretries,const bool dbg,bool& invalidargflag)
{		
	FUNSTACK;
	
	int cl=0;
	if (g_automode_callback==0) g_automode_callback=&DummyCallback;
	
	if (usbmode==2 && ScanforUSBmicrodevs()!=1) {
		log << "Could not find any USBm devices" << endl <<  "Wiiscan done [FAILED]" << endl;
		return false;
	}
	if (usbmode==3 && ScanforDelcomdevs()!=1) {
		log << "Could not find any Delcom devices" << endl <<  "Wiiscan done [FAILED]" << endl;
		return false;
	}

	g_automode_callback(cl++);

	timer total;
	assert(usbsleep>=0 && btsleep>=0 && usbmsleep>=0);
	
	log << "Auto-connecting to device <" << d << ">" << endl;

	// check valid config file
	if (!FileExists(cf))                    throw_("config file <" + cf + "> is missing");
	const Configfile c(cf); 
	if (dbg) log << c;
	if (!c.hasEntry("all_usb_hubs"))        throw_("config file missing <all_usb_hubs> entry");
	if (!c.hasEntry("active_usb_hub"))      throw_("config file missing <active_usb_hub> entry");
	if (!c.hasEntry("allowed_wiimote_adr")) throw_("config file missing <allowed_wiimote_adr> entry");
	if (!c.hasEntry("whiteboard_software")) throw_("config file missing <whiteboard_software> entry");

	// test if already opened
	g_automode_callback(cl++);

	if (removemode==0 && Wiilib::OpenDevices(lib,wiitimeout,false,dbg,true)>=1) {
		log << "  service is already opened..." << endl << "Done [OK]" << endl; 

		if (wb){			
			// fireup whiteboard software
			if (!c.hasEntry("whiteboard_software")) log << "  ** warning: config file is missing entry <whiteboard_software>" << endl;
			else System(c.Get<string>("whiteboard_software",true),false,false);
		}

		return true;
	}

	// power down now if using USB io board
	g_automode_callback(cl++);	
	USBupdown(usbmode,false,dbg);
	timer t;

	// remove old entries in HID
	g_automode_callback(cl++);
	if(removemode!=2) RemoveDevice(d,dbg);

	// Cycle usb hub, turn off usb power and restart the wiimote
	g_automode_callback(cl++);
	if(usbmode==1){
		const string usbhub=c.Get<string>("active_usb_hub");
		if(usbhub!="\"\""){
			USBupdown(usbmode,false,dbg,usbhub);
			Sleep(usbsleep);
			USBupdown(usbmode,true,dbg,usbhub);

			g_automode_callback(cl++);

			// wait for usb hub to be up, takes some seconds (1000ms to 2000ms approx)
			Sleep(1800);

			// wait for usb radio to be ready
			g_automode_callback(cl++);
			HANDLE hRadio=RadioInfo(btr,dbg);

			int i=0;
			while(hRadio==NULL) {
				if (i++==0) log << "  radio not ready, delaying.";
				else log << ".";
				Sleep(500);
				hRadio=RadioInfo(btr,dbg);
				if (i>10) throw_("could not connect to bluetooth radio device"); 
			}
			CloseHandle(hRadio);
			if (i>0) log << endl;
		}
	} else if (usbmode!=0) {
		while (t.elapsed()*1000<usbsleep) Sleep(20);
		USBupdown(usbmode,true,dbg); // just do an up, power down was done before
		g_automode_callback(cl++);
		Sleep(usbmsleep);      // wait for wiimote to be powered up
	} else g_automode_callback(cl++);

	// try to connect bt to wii
	g_automode_callback(cl++);	
	
	int retries=0;
	bool openok=false;

	assert( scanretries>=1 );
	while(!openok && ++retries<=scanretries){
		openok=OpenDevice(d,btr,min(retries*timeout,8000),wiitimeout,c("allowed_wiimote_adr"),btsleep,lib,false,false,dbg,invalidargflag);
	}
	
	if (openok){
		g_automode_callback(cl++);
		if (wb){
			g_automode_callback(cl++);
			// terminate running wb's

			// fireup whiteboard software
			if (!c.hasEntry("whiteboard_software")) log << "  ** warning: config file is missing entry <whiteboard_software>" << endl;
			else System(c.Get<string>("whiteboard_software",true),false,false);
		}
	}
	if (dbg) log << "  Elapsed time: " << total.elapsed() << " [s]" << endl;
	
	// taking longer than 20 sec will bring up an annoying windows reboot dialog, 
	// that can be ignored, hence warn at 18 sec
	if (total.elapsed()>18) log << "  ** warning: connection took a long time to finish, this may cause a windows reboot dialog, that can be ignored" << endl;
	
	if (!openok) log << "  ** warning: could not open device" << endl << "Done [FAILED]" << endl;
	
	assert(cl<10);
	g_automode_callback(42);
	return openok;
}

string PrintErrorCode(const DWORD r)
{
	FUNSTACK;

	if (r==BTH_ERROR_SUCCESS)                          return "OK";
	if (r==BTH_ERROR_NO_CONNECTION)                    return "ERROR = BTH_ERROR_NO_CONNECTION" ;
	if (r==BTH_ERROR_PAGE_TIMEOUT)                     return "ERROR = BTH_ERROR_PAGE_TIMEOUT" ;
	if (r==BTH_ERROR_HARDWARE_FAILURE)                 return "ERROR = BTH_ERROR_HARDWARE_FAILURE" ;
	if (r==BTH_ERROR_AUTHENTICATION_FAILURE)           return "ERROR = BTH_ERROR_AUTHENTICATION_FAILURE" ;
	if (r==BTH_ERROR_MEMORY_FULL)                      return "ERROR = BTH_ERROR_MEMORY_FULL" ;
	if (r==BTH_ERROR_CONNECTION_TIMEOUT)               return "ERROR = BTH_ERROR_CONNECTION_TIMEOUT" ;
	if (r==BTH_ERROR_LMP_RESPONSE_TIMEOUT)             return "ERROR = BTH_ERROR_LMP_RESPONSE_TIMEOUT" ;
	if (r==BTH_ERROR_MAX_NUMBER_OF_CONNECTIONS)        return "ERROR = BTH_ERROR_MAX_NUMBER_OF_CONNECTIONS" ;
	if (r==BTH_ERROR_PAIRING_NOT_ALLOWED)              return "ERROR = BTH_ERROR_PAIRING_NOT_ALLOWED" ;
	if (r==BTH_ERROR_UNSPECIFIED_ERROR)                return "ERROR = BTH_ERROR_UNSPECIFIED_ERROR" ;	
	if (r==BTH_ERROR_LOCAL_HOST_TERMINATED_CONNECTION) return "ERROR = BTH_ERROR_LOCAL_HOST_TERMINATED_CONNECTION" ;	

	return "ERROR = unkown error code (" + ::tostring(r) + ")";
}

int isConnected(const string& d,HANDLE hRadio,const vector<string>& known_adr,const bool dbg=false)
{
	FUNSTACK;

	BLUETOOTH_DEVICE_INFO bdi=Get_BLUETOOTH_DEVICE_INFO();
	BLUETOOTH_DEVICE_SEARCH_PARAMS bdsp=Get_BLUETOOTH_DEVICE_SEARCH_PARAMS(0);

	bdsp.fReturnAuthenticated=1;
	bdsp.fReturnConnected=1;
	bdsp.fReturnUnknown=0;
//	bdsp.fReturnRemembered=0;

	BluetoothFindFirstDevice(&bdsp,&bdi);
	const DWORD status=GetLastError();

	if      (status==ERROR_INVALID_PARAMETER) throw_("BluetoothFindFirstDevice(), returned status=ERROR_INVALID_PARAMETER");
	else if	(status==ERROR_REVISION_MISMATCH) throw_("BluetoothFindFirstDevice(), returned status=ERROR_REVISION_MISMATCH");

	DeviceAutoClose<HBLUETOOTH_DEVICE_FIND,BOOL> hbf(BluetoothFindFirstDevice(&bdsp,&bdi),&BluetoothFindDeviceClose);
	if (hbf()==NULL) return -1;

	int i=0;
	while(true){
		if (ReachedMax(i)) break;		
		if (MatchDevice(d,known_adr,bdi,dbg)) return bdi.fConnected>0;		
		if (BluetoothFindNextDevice(hbf(), &bdi) == FALSE) break;
		++i;
	}

	return -1; // no devices found at all
}
	
bool Register(const string& d,const Configfile& c,const string& btr,const int timeout,const bool dbg,BLUETOOTH_DEVICE_INFO* pbdi=0,BLUETOOTH_RADIO_INFO* pradio=0)
{
	FUNSTACK;

	DeviceAutoClose<HANDLE,BOOL> hRadio(RadioInfo(btr,dbg),&CloseHandle);
	if (hRadio()==NULL) throw_("failed to get radio");
	if (dbg) log << "  RadioInfo()...OK" << endl;

	BluetoothEnableIncomingConnections(hRadio(),true);
	BluetoothEnableDiscovery(hRadio(),true);

	BLUETOOTH_DEVICE_INFO bdi=Get_BLUETOOTH_DEVICE_INFO();
	BLUETOOTH_DEVICE_SEARCH_PARAMS bdsp=Get_BLUETOOTH_DEVICE_SEARCH_PARAMS(timeout);
		
	BluetoothFindFirstDevice(&bdsp,&bdi);
	const DWORD status=GetLastError();

	if      (status==ERROR_INVALID_PARAMETER) throw_("BluetoothFindFirstDevice(), returned status=ERROR_INVALID_PARAMETER");
	else if	(status==ERROR_REVISION_MISMATCH) throw_("BluetoothFindFirstDevice(), returned status=ERROR_REVISION_MISMATCH");

	DeviceAutoClose<HBLUETOOTH_DEVICE_FIND,BOOL> hbf(BluetoothFindFirstDevice(&bdsp,&bdi),&BluetoothFindDeviceClose);

	if (hbf()==NULL) {
		log << "  ** error: no devices found" << endl << "Done [FAILED]" << endl;
		return false;
	}

	const vector<string> known_adr=c("allowed_wiimote_adr");

	int i=0;
	while(true){
		if (ReachedMax(i)) break;
		const string fd=ConvertName(bdi.szName);
		log << "  found device [" << i << "]: <" << (fd.size()>0 ? fd : "NONAME") << ">" << endl;
		
		if (MatchDevice(d,known_adr,bdi,dbg)){
			log << "  found match..." << endl;
			if (dbg) log << tostring(bdi);

			const GUID service=HumanInterfaceDeviceServiceClass_UUID;
			DWORD status=BluetoothSetServiceState(hRadio(),&bdi,&service,BLUETOOTH_SERVICE_ENABLE);

			if (dbg && status==ERROR_SUCCESS) log << "  BluetoothSetServiceState()...OK" << endl;
			if (dbg && status!=ERROR_SUCCESS) log << "  BluetoothSetServiceState()...ERROR" << endl;

			Sleep(1000);


			BLUETOOTH_RADIO_INFO radio=Get_BLUETOOTH_RADIO_INFO();		
			if (BluetoothGetRadioInfo(hRadio(),&radio)!=ERROR_SUCCESS) throw_("BluetoothGetRadioInfo() failed");
			if (pradio!=0) *pradio=radio;

			// adr of btadapter, put into passphrase
			WCHAR pass[6];
			for(int i=0;i<6;++i) pass[i]=radio.address.rgBytes[i];

			log << "  BluetoothAuthenticateDevice()..";

			DWORD r=-1;
			int retries=0;
			do {
				Sleep(200);
				log << ".";
				r=BluetoothAuthenticateDevice(0,hRadio(),&bdi,pass,6);
			} while (r!=0 && ++retries<10);
			
			if (r!=0) {
				log << PrintErrorCode(r) << endl;
				return false;
			}

			//r=BluetoothUpdateDeviceRecord(&bdi);	
			//if (r!=ERROR_SUCCESS) throw_("BluetoothUpdateDeviceRecord() failed");

			log << "OK" << endl;
			if (pbdi!=0) *pbdi=bdi;

			return true;
		}
		
		if (BluetoothFindNextDevice(hbf(), &bdi) == FALSE) break;
		++i;
	}
	log << "  ** error: device not mathced" << endl<< "Done [FAILED]" << endl;
	return false;
}

int Usage(const args& arg,const string msg="")
{
	FUNSTACK;
	if (msg.size()>0) log << msg << endl;
	log << "Usage: " << arg[0] << " <-a <device> | -c <device> | -d <device> | -r | -s | -usbup | -usbdown> [-cf <file>] [-lf <file>] [-b <sleep>] [-t <sleep>] [-u <sleep>] [-p <sleep>] [-w <sleep>] [-q <usbradio>] [-f <removemode>]  [-m <powermode>] [-l <wiilibrary>] [-y] [-wb] [-v] " << endl;
	log << "  " <<  Version() << " " << Config() << endl;
	log << "  modes:" << endl;
	log << "    -a <name>: autoconnect to device" << endl;
	log << "    -c <name>: connect the device, that matches this name" << endl;
	log << "    -d <name>: deletes the device, that matches this name" << endl;
	log << "    -e: register wiimote device" << endl;
	log << "    -r: lookup and list bluetooth radio devices" << endl;
	log << "    -s: scan external bluetooth devices" << endl;
	log << "    -usbdown: disable usb hubs" << endl;
	log << "    -usbup: enable usb hubs" << endl;
	log << "  options:" << endl;
	log << "    -cf <file>: specify a distinct configurationfile, default="              << DEFAULT_configfile << endl;
	log << "    -lf <file>: specify a distinct logfile, default="                        << DEFAULT_logfile    << endl;
	log << "    -b <int>: automode bluetooth connection sleep in milliseconds, default=" << DEFAULT_btsleep    << endl;
	log << "    -t <int>: timeout for bluetooth stack in milliseconds, default="         << DEFAULT_timeout    << endl;
	log << "    -u <int>: automode usb connection sleep in milliseconds, default="       << DEFAULT_usbsleep   << endl;
	log << "    -p <int>: automode usbm post-connection sleep in milliseconds, default=" << DEFAULT_usbmsleep  << endl;
	log << "    -w <int>: timeout for wiimote in milliseconds, default="                 << DEFAULT_wiitimeout << endl;
	log << "    -q <string>: use bluetooth radio with this address (not working), default=any device"  << endl;
	log << "    -f <int>: pre-remove mode of device, 0=remove if not connectable, 1=always remove, 2=never remove, default=" << DEFAULT_removemode << endl;
	log << "    -m <int>: choose  USB powercycle mode, 0=no power cycle, 1=use USB hub, 2=use USBm IO hardware, 3=use USB Delcon IO hardware" << endl;
	log << "    -l <string>: use specific wiimote library, lib can be one-of {wiiuse,wiimotelib, default=" << DEFAULT_wiilib << endl;
	log << "    -y <int>: scan retries in automode, default="                            << DEFAULT_scanretries<< endl;
	log << "    -wb: start whiteboard in automode" << endl;
	log << "    -nowb: do not start whiteboard in automode" << endl;
	log << "    -v: enable extra debugging printouts" << endl;
	log << "  default mode: -a \"" << NINTENDO_DEV << "\"" << endl;
	log << "  note: \"nintendo\" is a shortcut for \"" << NINTENDO_DEV << "\"" << endl;
	return -1;
}

int main(int argc,char** argv)
{
	FUNSTACK;
	bool invalidargflag=false;
	try{
		SetNiceLevel(-15); // set to time-critical
		args arg(argc,argv);

		// first, parse the command line configfile, then load the config, finally override any configfile options with given command line options
		string cf=arg.parseval<string>("-cf",DEFAULT_configfile);

		bool v=false,wb=false;
		int timeout   =DEFAULT_timeout;
		int wiitimeout=DEFAULT_wiitimeout;
		int usbsleep  =DEFAULT_usbsleep;
		int usbmsleep =DEFAULT_usbmsleep;
		int btsleep   =DEFAULT_btsleep;
		int usbmode   =DEFAULT_usbmode;
		int removemode=DEFAULT_removemode;
		int scanretries=DEFAULT_scanretries;
		string a,c,d,btr,lf=DEFAULT_logfile,dev=NINTENDO_DEV,lib=DEFAULT_wiilib;

		if (FileExists(cf)){
			const Configfile cnf(cf); 
			if (cnf.hasEntry("option_device"))             dev=strip(cnf.Get<string>("option_device"),'"'); 
			if (cnf.hasEntry("option_timeout"))        timeout=cnf.Get<int>("option_timeout");
			if (cnf.hasEntry("option_wiitimeout"))  wiitimeout=cnf.Get<int>("option_wiitimeout");
			if (cnf.hasEntry("option_usbsleep"))      usbsleep=cnf.Get<int>("option_usbsleep");
			if (cnf.hasEntry("option_usbmsleep"))    usbmsleep=cnf.Get<int>("option_usbmsleep");
			if (cnf.hasEntry("option_btsleep"))        btsleep=cnf.Get<int>("option_btsleep");
			if (cnf.hasEntry("option_usbpowermode"))   usbmode=cnf.Get<int>("option_usbpowermode");
			if (cnf.hasEntry("option_removemode"))  removemode=cnf.Get<int>("option_removemode");
			if (cnf.hasEntry("option_debug"))                v=cnf.Get<bool>("option_debug");
			if (cnf.hasEntry("option_startwhiteboard"))     wb=cnf.Get<bool>("option_startwhiteboard");
			if (cnf.hasEntry("option_logfile"))             lf=cnf.Get<string>("option_logfile");
			if (cnf.hasEntry("option_btradio"))            btr=cnf.Get<string>("option_btradio");
			if (cnf.hasEntry("option_wiilib"))             lib=cnf.Get<string>("option_wiilib");
			if (cnf.hasEntry("option_scanretries"))scanretries=cnf.Get<int>("option_scanretries");
		}
		
		// parse argumets via args class
		const bool       r=arg.parseopt("-r");
		const bool       s=arg.parseopt("-s");
		const bool       e=arg.parseopt("-e");
		const bool       u=arg.parseopt("-?");
		const bool usbdown=arg.parseopt("-usbdown");
		const bool   usbup=arg.parseopt("-usbup");
    	v=arg.parseopt("-v") || v;
		wb=arg.parseopt("-wb") || wb;
		wb=!arg.parseopt("-nowb") && wb;
		timeout   =arg.parseval<int>("-t",timeout);
		wiitimeout=arg.parseval<int>("-w",wiitimeout);
		usbsleep  =arg.parseval<int>("-u",usbsleep);
		usbmsleep =arg.parseval<int>("-p",usbmsleep);
		btsleep   =arg.parseval<int>("-b",btsleep);
		usbmode   =arg.parseval<int>("-m",usbmode);
		removemode=arg.parseval<int>("-f",removemode);
		a=arg.parseval<string>("-a","");
		c=arg.parseval<string>("-c","");
		d=arg.parseval<string>("-d","");
		lf=arg.parseval<string>("-lf",lf);
		btr=arg.parseval<string>("-q",btr);
		lib=arg.parseval<string>("-l",lib);
		scanretries=arg.parseval<int>("-y",scanretries);

		if (arg.size()!=1) return Usage(arg);
	
		// apply shortcuts
		if (a=="nintendo") a=dev;
		if (c=="nintendo") c=dev;
		if (d=="nintendo") d=dev;

		// open log file before Usage call
		if(lf!=DEFAULT_logfile){
			lf=strip(lf,'"'); // cannot handle filenames in citation quotes
			if      (lf=="cout" || lf=="std::cout") g_log=&cout;
			else if (lf=="cerr" || lf=="std::cerr") g_log=&cerr;
			else {				
				g_log.open(lf,ios_base::app);
				g_log.writelogheader("Wiiscan::main()");
				if (v) log << "Using logfile <" << lf << ">" << endl;				
			}
		}

		const int check=(r ? 1 : 0) + (s ? 1 : 0) + (e ? 1 : 0) + (a.size()>0 ? 1 : 0) + (c.size()>0 ? 1 : 0) + (d.size()>0 ? 1 : 0) + usbup + usbdown;
		if (check>1 || u) return Usage(arg);

		if (timeout<20)   throw_("bad value of timeout, should be >=20");
		if (wiitimeout<20)throw_("bad value of wiitimeout, should be >=20");
		if (usbsleep<20)  throw_("bad value of usbsleep, should be >=20");
		if (usbmsleep<20) throw_("bad value of usbmsleep, should be >=20");
		if (btsleep<20)   throw_("bad value of btsleep, should be >=20");
		if (usbdown && usbup)   throw_("bad value of usbdown/up, cannot be used at the same time");
		if (wiitimeout%1000!=0) throw_("bad value, wiitimeout should be divisable with 1000");
		if (btr.size()>0 && btr.size()!=17) throw_("bluetooth radio must be in the form xx:xx:xx:xx:xx:xx");
		if (usbmode<0 || usbmode>4) throw_("bad usbpowermode, must be 0,1,2,3 or 4");
		if (removemode<0 || removemode>2) throw_("bad removemode, must be 0,1 or 2");
		if (scanretries<=0) throw_("bad value of scanretries, should be > 0");
		if (a.size()==0) a=NINTENDO_DEV;
		
		if (v) {
			log << Version() << " " << Config() << endl;
			log << "Values:" << endl;
			log << "  a=" << a << endl;
			log << "  c=" << c << endl;
			log << "  d=" << d << endl;
			log << "  e=" << d << endl;
			log << "  r=" << r << endl;
			log << "  s=" << s << endl;
			log << "  v=" << v << endl;
			log << "  cf=" << cf << endl;
			log << "  lf=" << lf << endl;
			log << "  wb=" << wb << endl;
			log << "  btr=" << btr << endl;
			log << "  lib=" << lib << endl;
			log << "  usbmode=" << usbmode << endl;
			log << "  removemode=" << removemode << endl;
			log << "  scanretries=" << scanretries << endl;
			log << "  usbup  =" << usbup << endl;
			log << "  usbdown=" << usbup << endl;
			log << "Timeouts:" << endl;
			log << "  timeout   =" << timeout << endl;
			log << "  wiitimeout=" << wiitimeout << endl;
			log << "  usbsleep  =" << usbsleep << endl;
			log << "  usbmsleep =" << usbmsleep << endl;
			log << "  btsleep   =" << btsleep << endl;
		}

		bool ret=true;

		if      (r)          ret=RadioInfo   (btr.size()==0 ? "all" : btr,v)!=NULL;
		else if (s)          ret=ScanDevices (timeout);
		else if (c.size()>0) ret=OpenDevice  (c,btr,timeout,wiitimeout,vector<string>(),btsleep,lib,true,true,v,invalidargflag);
		else if (d.size()>0) ret=RemoveDevice(d,v);
		else if (usbup)      ret=USBupdown   (usbmode,true, v,Configfile(cf).Get<string>("active_usb_hub",true));
		else if (usbdown)    ret=USBupdown   (usbmode,false,v,Configfile(cf).Get<string>("active_usb_hub",true));
		else if (e)          ret=Register    (d,cf,btr,timeout,v);
		else          		 ret=AutoOpenDevice(a,btr,timeout,wiitimeout,usbsleep,btsleep,usbmsleep,cf,lib,removemode,usbmode,wb,scanretries,v,invalidargflag); // default mode

		log << "Wiiscan done " << (ret ? "[OK]" : "[FAILED]") << endl;

		if (invalidargflag) {
			if (v) log << "** warning: return=-3 (E_INVALIDARG)" << endl;
			return -3;
		}

		if (v) log << "returning: return=" << (ret ? "0" : "-1") << endl;
		return ret ? 0 : -1; // 0=Ok, -1=fail, -2=fail with exception, -3=fail with E_INVALIDARG 
	}
	CATCH_ALL;

	assert(invalidargflag==false); // flag not setted => no throw in when E_INVALIDARG encoutered
	log << "Wiiscan done [FAILED]" << endl;
	return -2;
}

}; // namespace Wiiscan
