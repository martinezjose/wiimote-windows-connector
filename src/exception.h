// The code is copyrighted 2008 by Carsten Frigaard.
// All rights placed in public domain under GNU licence V2, 2008
//
// © 2008 Carsten Frigaard. Permission to use, copy, modify, and distribute this software
// and its documentation for any purpose and without fee is hereby granted, provided that
// the above copyright notice appear in all copies and that both that copyright notice
// and this permission notice appear in supporting documentation.

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

// forward defs to funs.h 
string Getlocaltime();
size_t GetThreadId();
int Message(const string& title,const string& msg,const int type=0);

// simple class for debugging call stack
#ifdef _DEBUG
	struct Stackinfo : public vector<string> {
		friend ostream& operator<<(ostream& s,const Stackinfo& x) 
		{
			s << "Function stack {" << endl;
			for(int i=x.size();i>0;--i){
				const string& f=x[i-1];
				s << "  [" << i-1 << "]: " << f << "(...)" << endl;
			}
			return s << "}" << endl;
		}
	};
	
	class Funstack {
		private:
			const string m_f;
			static map<size_t,Stackinfo> m_s;
			
			Funstack(const Funstack&);
			void operator=(const Funstack&); 

		public:
			Funstack(const string& f,const int line,const string& file) : m_f(f) {m_s[GetThreadId()].push_back(f);}
			~Funstack() 
			{
				const size_t tid=GetThreadId();
				assert(m_s.find(tid)!=m_s.end());
				assert(m_s[tid].size()>0 && m_s[tid].back()==m_f);
				m_s[tid].pop_back();		
			}

			static const Stackinfo GetStack() 
			{
				const size_t tid=GetThreadId();
				if (m_s.find(tid)==m_s.end()) return Stackinfo();
				else return m_s[tid];
			} 
	};
	 map<size_t,Stackinfo> Funstack::m_s; // initialize static var
	#define FUNSTACK Funstack __f_stack__(__FUNCTION__,__LINE__,__FILE__)
#else
	#define FUNSTACK
	#define DUMPSTACK(s)
	struct Stackinfo {
		friend ostream& operator<<(ostream& s,const Stackinfo& x) {return s;}
	};	
	class Funstack {
	public:
		static Stackinfo GetStack() {return Stackinfo();}
	};
#endif

// tee like logger class
class Logger 
{
private:
	//ostream* m_log;
	string    m_logfilename;
	ofstream  m_logfile;
	ostream*  m_log;
	const bool m_logstdout,m_logstderr;

	Logger(const Logger&);
	void operator=(const Logger&);

public:
	Logger(ostream* log,const bool logstdout=true,const bool logstderr=false) : m_log(log), m_logstdout(logstdout), m_logstderr(logstderr) {}

	void operator=(ostream* log){m_logfilename=""; m_logfile.close(); m_log=log;}
	void open(const string& logfilename,const ios_base::openmode mode)
	{
		m_log=0;
		if (m_logfile.is_open()) m_logfile.close();
		m_logfilename=logfilename;
		m_logfile.open(m_logfilename.c_str(),mode);
		if (!m_logfile) throw("cannot write to logfile <" + logfilename + ">"); // Exception uses logger class, so do not throw a nice Exception class here, use a plain throw
		m_log=&m_logfile;
	}

	void clear()
	{
		if (m_logfile.is_open()) m_logfile.close();
		m_logfile.open(m_logfilename.c_str());
	}

	template<class T>
	friend Logger& operator<<(Logger& log,const T& t)
	{
		if(log.m_logstdout) cout << t;
		if(log.m_logstderr) cerr << t;
		if(log.m_log!=0)    (*(log.m_log)) << t;
		return log;
	}

	// handle endl and like
	friend Logger& operator<<(Logger& log,std::ostream& (*fn)(std::ostream&)) 
	{
		if(log.m_logstdout) fn(cout);
		if(log.m_logstderr) fn(cerr);
		if(log.m_log!=0)    fn(*(log.m_log));
		return log;
	}

	void writelogheader(const string& msg)
	{
		if(m_log==0) return;
		else{
			(*m_log) << "********************************************************" << endl;
			(*m_log) << "** Logentry: " << msg << endl;
			(*m_log) << "**     Time: " << Getlocaltime();
			(*m_log) << "********************************************************" << endl;				
		}				
	}
};

// static global logging, default standand out
static Logger g_log(0,true,false);

class Exception{
private:
	const string m_msg;
	const string m_file;
	const int m_line;
	const Stackinfo m_stack;

public:
	Exception(const string msg,const string file,const int line,const Stackinfo s) : m_msg(msg), m_file(file), m_line(line), m_stack(s) {}
	Exception(const char*  msg,const string file,const int line,const Stackinfo s) : m_msg(msg), m_file(file), m_line(line), m_stack(s) {}

	inline static string FormatCompilerMsg(const string& file,const int line,const bool warnonly=false)
	{
		#ifdef WIN32
			return file + "(" + Exception::tostring(line) + ") : " + (warnonly ? "warning : " : "error : ");
		#else
			return file + ":" + Exception::tostring(line) + ": " + (warnonly ? "warning: " : "error: ");
		#endif
	}

	inline static void throw_fun(const string& msg,const string& file,const int line)
	{
		const string msg2=Exception::FormatCompilerMsg(file,line) + "throwing exception: " + msg;
		g_log << msg2;
		#ifdef _WINDOWS_
			const string f=tostring(Funstack::GetStack());
			const string msg3=msg2 + (f.size()>0 ? "\n\n" : "") + f.substr(0,300) + (f.size()>300 ? "\n..." : "");
			Message("Exception encountered...",msg3,0);
		#endif	
		throw Exception(msg,file,line,Funstack::GetStack());
	}

	inline string Msg() const
	{
		return FormatCompilerMsg(m_file,m_line) + "Exception: " + m_msg;
	}	

	friend ostream& operator<<(ostream& os,const Exception& e)
	{
		return os << e.Msg() << endl << e.m_stack;
	}

	template<typename T>
	static string tostring(const T& x)
	{
		ostringstream os;
		os << x;
		return os.str();
	}
};

#define throw_(msg) Exception::throw_fun(msg,__FILE__, __LINE__)
#define warn_(msg)  cerr << Exception::FormatCompilerMsg(__FILE__, __LINE__,true) << msg << endl;

#define CATCH_ALL\
	catch(const char*   s)   {cout.flush(); cerr << Exception::FormatCompilerMsg(__FILE__, __LINE__) << "caught exception chars: " << s;}\
	catch(const string& s)   {cout.flush(); cerr << Exception::FormatCompilerMsg(__FILE__, __LINE__) << "caught exception string: " << s;}\
	catch(const Exception& s){cout.flush(); cerr << Exception::FormatCompilerMsg(__FILE__, __LINE__) << "caught Exception class: "  << s;}\
	catch(...) 				 {cout.flush(); cerr << Exception::FormatCompilerMsg(__FILE__, __LINE__) << "caught unknown exception";}\
	cerr << "...aborting" << endl;

#endif // __EXCEPTION_H__
