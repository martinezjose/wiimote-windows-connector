// The code is copyrighted 2008 by Carsten Frigaard.
// All rights placed in public domain under GNU licence V2, 2008
//
// © 2008 Carsten Frigaard. Permission to use, copy, modify, and distribute this software
// and its documentation for any purpose and without fee is hereby granted, provided that
// the above copyright notice appear in all copies and that both that copyright notice
// and this permission notice appear in supporting documentation.

#ifndef __STRINGFUN_H__
#define __STRINGFUN_H__

struct outputoperator_tags
{
	string preline,endline;
	int  prewidth,postwidth,precision;
	bool maptags,printnumbers,serialize;

	outputoperator_tags()
	: endline("\n"), prewidth(-1), postwidth(-1), precision(-1), maptags(false), printnumbers(false), serialize(false) {}
};

static outputoperator_tags g_tags;

inline ostream& operator<<(ostream& os,const outputoperator_tags& x){
	g_tags=x;
	if (g_tags.precision>0) os.precision(g_tags.precision);
	return os;
}

template<typename T>
inline string tostring(const T& x)
{
	ostringstream os;
	outputoperator_tags t1=g_tags;
	g_tags=outputoperator_tags();
	g_tags.serialize=true;
	g_tags.endline="";
	os << x;
	g_tags=t1;
	return os.str();
}

template<typename T>
inline T totype(const string& s)
{
	istringstream is(s);
	T x;
	is >> x;
	return x;
}

inline string operator+ (const string& s,const char* c)				{return s+string(c);}
inline string operator+ (const string& s,const short& c) 	        {return s+tostring(c);}
inline string operator+ (const string& s,const unsigned short& c) 	{return s+tostring(c);}
inline string operator+ (const string& s,const int& c) 	          	{return s+tostring(c);}
inline string operator+ (const string& s,const long& c)    		  	{return s+tostring(c);}
inline string operator+ (const string& s,const unsigned long& c)   	{return s+tostring(c);}
inline string operator+ (const string& s,const float&  c) 			{return s+tostring(c);}
inline string operator+ (const string& s,const double& c) 			{return s+tostring(c);}
inline string operator+ (const string& s,const long double& c) 		{return s+tostring(c);}
inline string operator+ (const char* a  ,const string& b)			{return string(a)+b;}

#ifdef OS_Linux
#ifdef __GCC_V4__
	// does not work under some compilers where size_t==unsigned int
	inline string operator+ (const string& s,const size_t&  c)      {return s+tostring(c);}
#endif
#endif

inline string tostring(const int argc,char** argv)
{
	string s;
	for(int i=0;i<argc;++i) s += string(argv[i]) + " ";
	return s;
}

template<typename T>
inline string fwidth2(const T& x,const size_t wdt,const bool prefix)
{
	string s=tostring(x);
	if(wdt>1024 || s.size()>=wdt) return s;
	size_t n=wdt-s.size();
	while(n>0) {
		if (prefix) s=' '+ s;
		else s+=' ';
		--n;
	}
	return s;
}

template<typename T>
inline string fwidth(const T& x,const size_t wdt=8,const size_t tailwdt=4)
{
	string s=tostring(x);
	if(wdt>1024 || tailwdt>1024 || s.size()>wdt+tailwdt) return s;

	const size_t m=min(s.find('.'),s.find(' '));
	if (m==string::npos) {
		s=fwidth2(s,wdt,true);
		return fwidth2(s,wdt+tailwdt,false);
	}
	else{
		if(wdt<m) {
			return s;
		}
	    assert( wdt>=m );
		size_t n1=wdt-m;
		while(n1>0) {s= ' ' + s; --n1;}
		return fwidth2(s,wdt+tailwdt,false);
	}
}

inline string strip(const string& s,const char ch=' ')
{
	const size_t n=s.find_first_not_of(ch);
	const size_t m=s.find_last_not_of (ch);

	if (n==string::npos || m==string::npos) return "";
	return s.substr(n,m-n+1);
}

inline string replace(const string& s,const string& f,const string& r)
{
	if (f.size()==0) return s;
	const size_t n=s.find(f);
	if (n==string::npos) return s;
	else return replace(s.substr(0,n) + r + s.substr(n+f.size()),f,r);
}

inline string indent(const string& s,const string& indent)
{
	string t,q=s;
	while(q.size()){
		const string::size_type n=q.find_first_of("\n");
		t += indent + q.substr(0,n) + "\n";
		if (n==string::npos)  break;
		assert(n+1<=q.size());
		q = q.substr(n+1,q.size());
	}
	return t;
}

inline string removerems(const string& s,const string rem)
{
    const size_t n=s.find_first_of(rem);
	return s.substr(0,n);
}

inline string suffix(const int n)
{
	assert(n>=0 && n<999);
	if (n<=9) return "00" + tostring(n);
	else if (n<=99) return "0" + tostring(n);
	else return tostring(n);
}

string tail(const string& s,const string& delimiter)
{
	const size_t n=s.find_last_of(delimiter);
	if (n==string::npos) return s;
	else return s.substr(n+delimiter.size(),-1);
}

string tail(const string& s,const char c) {
	string t;
	t.resize(1);
	t[0]=c;
	return tail(s,t);
}

template<typename T>
inline ostream& operator<<(ostream& os,const pair<T*,int> x)
{
	for(int i=0;i<x.second;++i) os << x.first[i] << " ";
	return os;
}

template<typename T>
inline istream& operator>>(istream& is,pair<T*,int> x)
{
	for(int i=0;i<x.second;++i) is >> x.first[i];
	return is;
}

template<typename T>
inline ostream& operator<<(ostream& s,const vector<T>& x)
{
	int i=0;
	if (g_tags.serialize) s << "vector: " << x.size() << " { ";
	for(typename vector<T>::const_iterator itt=x.begin();itt!=x.end();++itt) {
		s << g_tags.preline;
		if(g_tags.printnumbers) s << "[" << fwidth(++i,3,0) << "] ";
		s << *itt << " " << g_tags.endline;
	}
	if (g_tags.serialize) s << "} ";
	return s;
}

template<typename T>
inline istream& operator>>(istream& s,vector<T>& x)
{
	size_t n=0;
	string t;
	s >> t;
	if (t!="vector:") throw_("bad format in vector serialization stream, tag missing");
 	s >>  n >> t;
	if (t!="{") throw_("bad format in vector serialization stream, missing begin brace");
	x.resize(n);
	for(size_t i=0;i<n;++i) s >> x[i];
	s >> t;
	if (t!="}") throw_("bad format in vector serialization stream, missing end brace");
	return s;
}

template<typename T>
inline ostream& operator<<(ostream& s,const list<T>& x)
{
	if (g_tags.serialize) s << "list: " << x.size() << " { ";
	int i=0;
	for(typename list<T>::const_iterator itt=x.begin();itt!=x.end();++itt){
		s << g_tags.preline;
		if(g_tags.printnumbers) s << "[" << fwidth(++i,3,0) << "] ";
		s << *itt << " " << g_tags.endline;
	}
	if (g_tags.serialize) s << "} ";
	return s;
}

template<typename T>
inline istream& operator>>(istream& s,list<T>& x)
{
	size_t n=0;
	string t;
	s >> t;
	if (t!="list:") throw_("bad format in list serialization stream, tag missing");
 	s >> n >> t;
	if (t!="{") throw_("bad format in list serialization stream, missing begin brace");
	for(size_t i=0;i<n;++i) {
		T y;
		s >> y;
		x.push_back(y);
	}
	s >> t;
	if (t!="}") throw_("bad format in list serialization stream, missing end brace");
	return s;
}

template<typename T,typename R>
inline ostream& operator<<(ostream& s,const map<T,R>& x)
{
	if (g_tags.serialize) s << "map: " << x.size() << " { ";
	int i=0;
	for(typename map<T,R>::const_iterator itt=x.begin();itt!=x.end();++itt) {
		s << g_tags.preline;
		if(g_tags.printnumbers) s << "[" << fwidth(++i,3,0) << "] ";
		s << fwidth(itt->first,g_tags.prewidth,g_tags.postwidth) << (g_tags.maptags ? " |-> " : " ");
		s << fwidth(itt->second,g_tags.prewidth,g_tags.postwidth) << " " << g_tags.endline;
	}
	if (g_tags.serialize) s << "} ";
	return s;
}

template<typename T,typename R>
inline istream& operator>>(istream& s,map<T,R>& x)
{
	size_t n=0;
	string t;
	s >> t;
	if (t!="map:") throw_("bad format in map serialization stream, tag missing");
 	s >> n >> t;
	if (t!="{") throw_("bad format in map serialization stream, missing begin brace");
	for(size_t i=0;i<n;++i) {
		T y;
		R z;
		s >> y >> z;
		if (x.find(y)!=x.end()) throw_("bad stream, key value no unique");
		x[y]=z;
	}
	s >> t;
	if (t!="}") throw_("bad format in map serialization stream, missing end brace");
	return s;
}

#endif // __STRINGFUNS_H__
