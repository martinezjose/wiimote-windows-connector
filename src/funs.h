// The code is copyrighted 2008 by Carsten Frigaard.
// All rights placed in public domain under GNU licence V2, 2008
//
// © 2008 Carsten Frigaard. Permission to use, copy, modify, and distribute this software
// and its documentation for any purpose and without fee is hereby granted, provided that
// the above copyright notice appear in all copies and that both that copyright notice
// and this permission notice appear in supporting documentation.

#ifndef __FUNS_H__
#define __FUNS_H__

#define Unimplemented throw_("Function unimplemented")
#define Dontgethere   throw_("Dontgethere")

#ifndef NDEBUG
	#define ON_DEBUG(a) a
#else
	#define ON_DEBUG(a)
#endif

#ifdef USE_FFTW
	extern int posix_memalign(void **__memptr, size_t __alignment, size_t __size);
#endif

// Small template funs
template<class T,class R> bool isinmap (const map<T,R>& m,const T& t) {return m.size()>=2 && m.begin()->first<=t && t<(--m.end())->first;}
template<class T> const size_t getsteps(const T& r0,const T& r1,const T& rstep,const bool logarithmic) {size_t N=0; for(T r=r0;r<r1;logarithmic ? r*=rstep/T(1) : r+=rstep) ++N; return N;}

inline std::string Version(){return string("VERSION: ") + VERSION + "." + VERSION_REV;}

inline std::string Config()
{
	std::string s;
	#ifdef NDEBUG
		s+="NDEBUG";
	#else
		#ifdef PROFILE
			s+="PROFILE";
		#else
			s+="DEBUG";
		#endif
	#endif
	#ifdef USE_UNITS
		s+= " USE_UNITS";
	#endif
	#ifdef USE_FLOAT_CHECKS
		s+= " USE_FLOAT_CHECKS";
	#endif
	#ifdef USE_FFTW_IC
		s+= " USE_FFTW_IC";
	#endif
	#ifdef USE_FFTW
		s+= " USE_FFTW";
	#endif
	#ifdef USE_MPI
		s+= " USE_MPI";
	#endif
	if      (sizeof(void*)==4) s+=" 32BIT";
	else if (sizeof(void*)==8) s+=" 64BIT";
	else                       s+=" XXBIT";

    const long one= 1;
    const int big=!(*(reinterpret_cast<const char *>(&one)));
	if (big) s+=" BIGENDIAN";
	else     s+=" LITENDIAN";

	return s;
}

inline string FormatCompilerMsg(const string& file,const int line)
{
	#ifdef WIN32
		return file + ":" + line + ":";
	#else
		return file + "(" + line + ")";
	#endif
}

#ifdef WIN32
	extern "C"{
		#ifdef  _UNICODE 
			#pragma message("ERROR: toolsfun cannot handle unicode...giving up compilation");
			ERROR_complier
		#endif

		#ifdef _AFXDLL
			__declspec(dllimport) void* __stdcall GetCurrentThread();
			__declspec(dllimport) void* __stdcall GetCurrentThreadId();
			__declspec(dllimport) int   __stdcall SetThreadPriority(void* hThread,int nPriority);
		#else
			//void* __stdcall GetCurrentThread();
			//void* __stdcall GetCurrentThreadId();
			//int __stdcall SetThreadPriority(void* hThread,int nPriority);
			__declspec(dllimport) void*  __stdcall GetCurrentThread();
			__declspec(dllimport) unsigned long __stdcall GetCurrentThreadId();
			__declspec(dllimport) int __stdcall SetThreadPriority(void* hThread,int nPriority);
			__declspec(dllimport) int __stdcall GetThreadPriority(void* hThread);
			__declspec(dllimport) unsigned int __stdcall WinExec(const char* lpCmdLine,unsigned int uCmdShow);
			#ifndef _WINDOWS_
				__declspec(dllimport) unsigned int __stdcall MessageBox(void* hWnd,const char* lpText,const char* lpCaption,unsigned int Type);
				__declspec(dllimport) unsigned long __stdcall GetCurrentDirectory(unsigned long nBufferLength,char* lpBuffer);
			#endif
		#endif
	}
#else
	int nice(int inc);
#endif

inline int Message(const string& title,const string& msg,const int type)
{
#ifdef WIN32
	// 0 = MB_OK
	// 1 = MB_OKCANCEL
	// 2 = MB_ABORTRETRYIGNORE
	// 3 = MB_YESNOCANCEL  
	// 4 = MB_YESNO
	// 5 = MB_RETRYCANCEL
	// 6 = MB_CANCELTRYCONTINUE: if(WINVER >= 0x0500)
	const long hr=MessageBox(NULL,msg.c_str(),title.c_str(),type);
#else 
	Unimplemented;
#endif
	return 1;
}
/*
string GetCurrentDir()
{
#ifdef WIN32
	char buff[16*1024];
	if(GetCurrentDirectory(16*1024,buff)==0) throw_("GetCurrentDirectory() failed");
	return tostring(buff);
#else 
	Unimplemented;
	return "";
#endif
}
*/

inline void SetNiceLevel(const int level)
{
	#ifdef WIN32
		// THREAD_PRIORITY_ABOVE_NORMAL 1	Priority 1 point above the priority class.
		// THREAD_PRIORITY_BELOW_NORMAL -1	Priority 1 point below the priority class.
		// THREAD_PRIORITY_HIGHEST 2		Priority 2 points above the priority class.
		// THREAD_PRIORITY_IDLE	-15         Base priority of 1 for IDLE_PRIORITY_CLASS,... 
		// THREAD_PRIORITY_LOWEST -2        Priority 2 points below the priority class.
		// THREAD_PRIORITY_NORMAL 0         Normal priority for the priority class.
		// THREAD_PRIORITY_TIME_CRITICAL 15 Base priority of 15 for IDLE_PRIORITY_CLASS,...
		if (level!=0 && level!=1 && level!=-1 && level!=2 && level!=-2 && level!=15 && level!=-15) throw_("wrong Win32 nice level, must be oneof -15,-2,-1,0,1,2,15"); 
		SetThreadPriority(GetCurrentThread(),-level);
		assert( GetThreadPriority(GetCurrentThread())==-level );
	#else
		const int n=nice(level);
		if (n<0) throw_("Could not set nice level");
	#endif
}

inline size_t GetThreadId()
{
	#ifdef WIN32
		assert( sizeof(size_t)==sizeof(unsigned long) );
		return GetCurrentThreadId();
	#else
		// may be replaced by return 0; if phtread not found!
		assert( sizeof(size_t)==sizeof(pthread_t) );
		const pthread_t p=pthread_self();
		size_t q=0;
		memcpy(&q,&p,sizeof(q));
		return q;
	#endif
}

#ifdef TOOLSFUN_QUIET_WIN32_SYSTEM	
#ifdef WIN32
	// make a special non-console system call, instead of the standard system()
	// XXX SystemWin, does not wait for process to finish, CreatProc still create window

	int SystemWin(const string& cmd)
	{
		//ShellExecute, CreateProcess, WinExec or system						
		// HINSTANCE hi=ShellExecute(NULL,NULL,cmdx,"","",SW_SHOSNOACTIVATE);
		// if (reinterpret_cast<int>(hi)<32) MessageBox(NULL,(string("ShellExecute <") + cmd + "> failed").c_str(),"Error",0);

		// const string cmd2="\\\"" + cmd + "\\\""; // fix problem with spaces in executable, not working yet
		unsigned int r=WinExec(cmd.c_str(),4);
		return r<32 ?  -1 : 0;		
	}

	bool CreateProc(const string& cmd,const bool throwexception=true,const bool waitforprocesstofinish=true)
	{
		STARTUPINFO s;
		PROCESS_INFORMATION p;
		memset(&s,0,sizeof(s));
		memset(&p,0,sizeof(p));
		s.cb=sizeof(s);

		// to avoid const cast of char* in CreateProcess
		char cmdx[16*1024];
		strcpy_s(cmdx,16*1024,cmd.c_str()); 
		
		const int r=CreateProcess(0,cmdx,0,0,false,CREATE_DEFAULT_ERROR_MODE,0,0,&s,&p);
		if (r!=0) {if(waitforprocesstofinish) WaitForSingleObject(p.hProcess,INFINITE);}
		else      {
			if (throwexception) throw_(string("CreateProcess() failed with return code <") + GetLastError() + ">");
			else return false;
		}
		
		// Release handles
		assert(r!=0);
		CloseHandle(p.hProcess);
		CloseHandle(p.hThread);
		return true;
	}
#endif
#endif

inline string System(const string& cmd,const bool throwexception=true,const bool captureoutput=false,int* pret=0)
{
	if (!captureoutput){
		#ifdef TOOLSFUN_QUIET_WIN32_SYSTEM	
			const int n=SystemWin(cmd);	
		#else
			const int n=system(cmd.c_str());
		#endif
		if (n!=0 && throwexception) throw_(string("system command failed with code=") + n + " cmd=<" + cmd + ">");
		if (pret!=0) *pret=n;
		return "";
	} else {
		static size_t n=0;
		#ifdef WIN32
			const string rm="del ";
			char tmp[1024];
			if (tmpnam_s(tmp,1024)) throw_("error in creating win32 temp name");
			const string file(tmp);
		#else
			const string rm="rm ";
			const string file=tmpnam("tempfile");
		#endif	
		ifstream s1(file.c_str());
		if(s1) {
			s1.close();
			System((rm + file).c_str(),true,false);
		}
		System(cmd + " > " + file,throwexception,false,pret);

		string t;
		char buff[16*1024];
		ifstream s2(file.c_str());
		while(s2) {
			s2.getline(buff,16*1024);
			if (s2) t += buff;
		}
		s2.close();
		System((rm + file).c_str(),true,false);
		return t;
	}
}

string Getlocaltime()
{
	FUNSTACK;
	time_t rawtime;
	time(&rawtime);
	struct tm timeinfo;
	localtime_s(&timeinfo,&rawtime);
	char buff[1024];
	asctime_s(buff,1014,&timeinfo);
	return string(buff);
}

class timer
{
	private:
	double  m_t,m_cpu_t;
	double* m_addt;
	mutable double m_last_t,m_last_eta;

	static double gettime()
	{
		#ifdef WIN32
			return 1.0*clock()/CLOCKS_PER_SEC; // use low-res clock
			// FILETIME ft;
		    // unsigned __int64 tmpres = 0;
			// 
			// GetSystemTimeAsFileTime(&ft);
			// 
			// tmpres |= ft.dwHighDateTime;
			// tmpres <<= 32;
			// tmpres |= ft.dwLowDateTime;
		    // 
			// converting file time to unix epoch
			// tmpres -= DELTA_EPOCH_IN_MICROSECS; 
			// tmpres /= 10;  // convert into microseconds
			// tv->tv_sec = (long)(tmpres / 1000000UL);
			// tv->tv_usec = (long)(tmpres % 1000000UL);		 
		#else
			struct timeval tv;
			gettimeofday(&tv,NULL);
			return tv.tv_sec +  static_cast<double>(tv.tv_usec)/1000000;
		#endif
	}

	static double getcputime()
	{
		static const double f=1.0/CLOCKS_PER_SEC;
		return f*clock();
	}

	template<class T>
	static double Remaining(const double t,const T& n,const T& N)
	{
		if (n>=N || N<=T(0)) throw_("value out of range in timer::Remaining, n>=N or N<=0, n=" + tostring(n) + " N=" + tostring(N));
		const double p=static_cast<double>(n/T(1)+1)/(N/T(1));
		const double p2=p>0 ? t/p : 0;
		return p2>t ? p2-t : 0;
	}

	public:
	timer()          : m_t(gettime()), m_cpu_t(getcputime()), m_addt(0),  m_last_t(-1), m_last_eta(-1) {}
	timer(double& t) : m_t(gettime()), m_cpu_t(getcputime()), m_addt(&t), m_last_t(-1), m_last_eta(-1) {}
	~timer() {if (m_addt!=0) (*m_addt) += elapsed();}

	void   reset  ()       {m_t=gettime(); m_cpu_t=getcputime(); m_last_t=-1; m_last_eta=-1;}
	double elapsed() const {return gettime()-m_t;}
	double cputime() const {return getcputime()-m_cpu_t;}

	static string ToHMS(const double& t)
	{
		assert( t>=0 );
		const unsigned int it=static_cast<unsigned int>(t+.5);
		const unsigned int hours=it/(60*60);
		const unsigned int mins=(it-hours*60*60)/(60);
		const unsigned int secs=(it-hours*60*60-mins*60);
		assert( secs<60 && mins<60);
		return tostring(hours) + ":" + (mins<10 ? "0": "") + tostring(mins) + ":" +  (secs<10 ? "0": "") + tostring(secs);
	}

	template<class T> static inline int Topercent(const T x,const T N,const int decimals=-1)
	{
		assert(x<N && N!=T(0));
		float pf=static_cast<float>(x/T(1))/(N/T(1));
		if (decimals>0) pf=static_cast<int>(pf*decimals)/(1.0*decimals);
		return static_cast<int>(pf*100+.5);
	}

	template<class T> void ToEta(const T& n,const T& N,ostream& s,const double timeprintsteps=30,const bool verboseprint=false) const
	{
		if (n>=N) return;
		assert( n<N );

		const double t=gettime()-m_t;
		if (n==T(0)) {
			m_last_t=t;
			return;
		}

		const double e=(t-m_last_t);
		if (e>timeprintsteps) {
			const double f=timeprintsteps*60;
			const double r=Remaining(t,n,N);
			if (m_last_eta<0 || r<f || (r>f && e>f) /*|| (m_last_eta>0 && r>1.2*m_last_eta) */ ){
				time_t tm;
				time(&tm);
				const string systime=ctime(&tm);
				if (m_last_eta<0) s << "Current system time: " << systime;
				tm += static_cast<time_t>(r);
				const string eta=ctime(&tm);
				const bool extraday=(eta.substr(0,3)!=systime.substr(0,3));
				const string eday=extraday ? " "+eta.substr(0,3)+" " : "";
				s << "Time [h:m:s]=" << ToHMS(t) <<  ", R=" << ToHMS(r) << ", ETA=" << eday << eta.substr(11,8);
				if(verboseprint) {
					const string t=", n/N=" + tostring(n) + "/" +  tostring(N) + "=" +  tostring(Topercent(n,N,10)) + " percent,";
					s << t;
					for(size_t i=t.size();i<42;++i) s << " ";
					s << " CPU=" << ToHMS(cputime());
				}
				s << endl;

				m_last_t=t;
				m_last_eta=r;
			}
		}
	}
	friend ostream& operator<<(ostream& s,const timer x)
	{
		return s << "Time [h:m:s]= " << x.ToHMS(x.elapsed()) << " CPU=" << x.ToHMS(x.cputime());
	}
};

/*
int SwapEndian(void *data,const size_t size) {

	short xs;
	long xl;

	switch (size){
		case 2:
			xs = *(short *)data;
			*(short *)data = ( ((xs & 0x0000ff00) >> 8) | ((xs & 0x000000ff) << 8) );
			break;
		case 4:
			xl = *(long *)data;
			*(long *)data = ( ((xl & 0xff000000) >> 24) | ((xl & 0x00ff0000) >> 8) |
					((xl & 0x0000ff00) << 8) | ((xl & 0x000000ff) << 24) );
			break;
			default: break;
	}

	return 0;
}
*/

#endif // __FUNS_H__
