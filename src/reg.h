#ifndef __REGISTRY_H__
#define __REGISTRY_H__

#ifdef WIN32 
#ifdef _WINDOWS

namespace Registry{
	void* StringtoRoot(const string& t)
	{
		if      (t=="HKEY_CLASSES_ROOT")  return HKEY_CLASSES_ROOT;
		else if (t=="HKEY_CURRENT_USER")  return HKEY_CURRENT_USER;
		else if (t=="HKEY_LOCAL_MACHINE") return HKEY_LOCAL_MACHINE;
		else if (t=="HKEY_USERS")         return HKEY_USERS;
		else    {throw_("bad root path in registry"); return 0;}
	}

	pair<void*,pair<string,string> > GetPath(const string& fullpath)
	{
		const int n=fullpath.find_first_of('\\');
		if (n==string::npos) throw_("mallformated registry entry");
		const string t=fullpath.substr(0,n);
		void* root=StringtoRoot(t);

		const string r=fullpath.substr(n+1,-1);
		const int m=r.find_last_of('\\');
		if (m==string::npos) throw_("mallformated registry entry");
		
		const string path=r.substr(0,m);
		const string key =r.substr(m+1,-1);

		return make_pair(root,make_pair(path,key));
	}

	bool hasKey(void* root,const string& path,const string& key,string* val=0)
	{
		assert( sizeof(void*)==sizeof(HKEY) && root!=0 );
		if (root!=HKEY_CLASSES_ROOT && root!=HKEY_CURRENT_USER && root!=HKEY_LOCAL_MACHINE && root!=HKEY_USERS) throw_("unknown root path in registry");

		DWORD buffersize=1024*16;
		char buff[1024*16];
		buff[0]=0;

		HKEY hKey;
		if (ERROR_SUCCESS!=RegOpenKeyEx(static_cast<HKEY>(root),path.c_str(),NULL,KEY_READ,&hKey)) return false;
		if (ERROR_SUCCESS!=RegQueryValueEx(hKey,key.c_str(),NULL,NULL,(LPBYTE)buff,&buffersize)) return false;
		if (ERROR_SUCCESS!=RegCloseKey(hKey)) return false;

		if (val!=0) *val=buff;
		return true;
	}

	string GetKey(void* root,const string& path,const string& key)
	{
		string val;
		if (!hasKey(root,path,key,&val)) throw_("could not read registry entry");
		return val;
	}

	string GetKey(const string& fullpath)
	{
		const pair<void*,pair<string,string> > p=GetPath(fullpath);
		return GetKey(p.first,p.second.first,p.second.second);
	}

	bool SetKey(void* root,const string& path,const string& key,const string& val)
	{
		assert( sizeof(void*)==sizeof(HKEY) && root!=0 );
		if (root!=HKEY_CLASSES_ROOT && root!=HKEY_CURRENT_USER && root!=HKEY_LOCAL_MACHINE && root!=HKEY_USERS) throw_("unknown root path in registry");
		if (val.size()+1>1024*16) throw_("lenght of value to long");

		char buff[1024*16];		
		size_t i;
		for(i=0;i<val.size();++i) buff[i]=val[i];
		buff[i]=0;
		const DWORD buffersize=i;
		
		HKEY hKey;		
		if (ERROR_SUCCESS!=RegCreateKeyEx(static_cast<HKEY>(root),path.c_str(),0,NULL,REG_OPTION_VOLATILE,KEY_ALL_ACCESS,NULL,&hKey,NULL)) return false;
		if (ERROR_SUCCESS!=RegSetValueEx(hKey,key.c_str(),NULL,REG_SZ,(LPBYTE)buff,buffersize)) return false;
		if (ERROR_SUCCESS!=RegCloseKey(hKey)) return false;

		return true;
	}

	bool SetKey(const string& fullpath,const string& val)
	{
		const pair<void*,pair<string,string> > p=GetPath(fullpath);
		return SetKey(p.first,p.second.first,p.second.second,val);
	}

}; // namespace Registry

#endif // _WINDOWS
#endif // WIN32
#endif // __REGISTRY_H__