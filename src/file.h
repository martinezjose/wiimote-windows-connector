// The code is copyrighted 2008 by Carsten Frigaard.
// All rights placed in public domain under GNU licence V2, 2008
//
// © 2008 Carsten Frigaard. Permission to use, copy, modify, and distribute this software
// and its documentation for any purpose and without fee is hereby granted, provided that
// the above copyright notice appear in all copies and that both that copyright notice
// and this permission notice appear in supporting documentation.

#ifndef __FILE_H__
#define __FILE_H__

inline bool FileExists(const std::string& f)
{
	ifstream s(f.c_str());
	return (!s)==false;
}

inline void AssertFileExists(const std::string& f)
{
	if (!FileExists(f)) throw_("File <" + f + "> does not exist");
}

inline size_t FileSize(const std::string& f)
{
	AssertFileExists(f);
	ifstream s(f.c_str());
	if (!s) throw_("Stream is bad (file <" + f + ">)");
	s.seekg(0,ios::end);
	if (!s) throw_("Stream is bad (file <" + f + ">)");
	return s.tellg();
}

inline void AssertFileNotEmpty(const std::string& f)
{
	if (FileSize(f)==0) throw_("File <" + f + "> is empty");
}

inline size_t FileTime(const string& file)
{
	AssertFileExists(file);
	const string t=System("date -r " + file + " +%s",true,true); // seconds since 1970-01-01 00:00:00 UTC
	return totype<size_t>(t);
	// 	#include <sys/stat.h>
	// 	#include <unistd.h>
	// 	struct stat s;
	// 	int n=stat(file.c_str(),&s);
	// 	if (n!=0) throw_("cannot stat file <" + file + ">");
	// 	assert( sizeof(time_t)==sizeof(size_t) );
	// 	return s.st_mtime;
}

inline bool isFileNewer(const string& file0,const string& file1)
{
	return FileTime(file0)>FileTime(file1);
}

inline bool DirExists(const std::string& f)
{
	const string file=f + "/.dummy.txt";
	ofstream s(file.c_str());
	if(!s) return false;
	s << "testfile";
	if(!s) return false;
	s.close();
  	return FileSize(file)==8;
}

inline string MakeSuffix(const int n)
{
	assert(n>=0);
	if (n<=9) return "00" + tostring(n);
	else if (n<=99) return "0" + tostring(n);
	else return tostring(n);
}

template<class T>
void Readdata(const string& tok,istream& is,T& t)
{
	if (!is) throw_("Stream is bad");
	is >> t;
	if (!is) throw_("Reading {" + tok + "} settings");
}

inline string Readline(istream& is)
{
	char buff[16*1024];
	is.getline(buff,16*1024);
	return string(buff);
}

template<class T>
inline T Readtyp(ifstream& s){
	T x;
	s.read(reinterpret_cast<char*>(&x),sizeof(x));
	if(!s) throw_("bad stream");
	return x;
}

inline string Readstring(ifstream& s){
	char c=0;
	string t;
	do{
		c=Readtyp<char>(s);
		if(c!=0) t+=c;
	}
	while (c!=0);
	return t;
}

template<class T>
inline vector<T> Readbin(std::ifstream& s,const int size)
{
	if(!s) throw_("bad stream");
	vector<T> x(size);
	s.read(reinterpret_cast<char*>(&x.front()),x.size()*sizeof(T));
	if(!s) throw_( "bad write");
	return x;
}

template<class T>
inline void Writetyp(ofstream& s,const T& x){
	s.write(reinterpret_cast<const char*>(&x),sizeof(x));
	if(!s) throw_( "bad stream");
}

template<class T>
inline void Writebin(std::ofstream& s,const std::vector<T>& x,const bool writetag)
{
	if(!s) throw_("bad stream");
	const size_t sz=x.size()*sizeof(T);
	if(writetag){
		Writetyp(s,sz);
	}
	if(!s) throw_( "bad stream" );
	s.write(reinterpret_cast<const char*>(&x.front()),sz);
	if(!s) throw_( "bad write");
	if (writetag) Writetyp(s,sz);
	if(!s) throw_( "bad stream");
}

template<class T,class R>
inline void Writebin(std::ofstream& s,const std::map<T,R>& x,const bool writetag)
{
	vector<T> t;
	vector<R> r;
	t.reserve(x.size());
	r.reserve(x.size());
	for(typename std::map<T,R>::const_iterator itt=x.begin();itt!=x.end();++itt){
		t.push_back(itt->first);
		r.push_back(itt->second);
	}
	if (writetag) {
		Writetyp(s,x.size());
		Writetyp(s,static_cast<unsigned int>(sizeof(T)));
		Writetyp(s,static_cast<unsigned int>(sizeof(R)));
	}
	Writebin(s,t,writetag);
	Writebin(s,r,writetag);
}

template<class T>
inline void Readbin(std::ifstream& s,vector<T>& x)
{
	if(!s) throw_("bad stream");
	const size_t sz=Readtyp<size_t>(s);
	if(!s) throw_( "bad stream" );
	if(sz%sizeof(T)!=0) throw_("bad size tag");
	x.resize(sz/sizeof(T));
	s.read(reinterpret_cast<char*>(&x.front()),sz);
	if(!s) throw_( "bad write");
	if (Readtyp<size_t>(s)!=sz) throw_("bad size tag");
	if(!s) throw_( "bad stream");
}

template<class T,class R>
inline void Readbin(std::ifstream& s,map<T,R>& x)
{
	vector<T> t;
	vector<R> r;

	const size_t sz=Readtyp<size_t>(s);
	const size_t szT=Readtyp<unsigned int>(s);
	const size_t szR=Readtyp<unsigned int>(s);

	if (szT!=sizeof(T)) throw_("type T size mismatch in Readbin (map)");
	if (szR!=sizeof(R)) throw_("type R size mismatch in Readbin (map)");

	Readbin(s,t);
	Readbin(s,r);
	if (t.size()!=r.size()) throw_("size mismatch in Readbin (map)");
	x.clear();
	for(size_t i=0;i<t.size();++i){
		x[t[i]]=r[i];
	}
	if (x.size()!=sz) throw_("map size mismatch in Readbin (map)");
}

template<class T>
inline void Writeascii(const string& filename,const std::vector<T>& x,const string& comment="",const char& newline='\n')
{
 	ofstream s(filename.c_str());
	if(!s) throw_("bad file <" + filename + ">");
	s << "% Writeascii: size=" << x.size() << " " << comment << "\n";
	for(size_t i=0;i<x.size();++i) s << x[i] << newline;
	if(!s) throw_( "bad writing to file <" + filename + ">");
}

#endif // __FILE_H__
