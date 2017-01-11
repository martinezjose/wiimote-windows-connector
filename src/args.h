// The code is copyrighted 2008 by Carsten Frigaard.
// All rights placed in public domain under GNU licence V2, 2008
//
// © 2008 Carsten Frigaard. Permission to use, copy, modify, and distribute this software
// and its documentation for any purpose and without fee is hereby granted, provided that
// the above copyright notice appear in all copies and that both that copyright notice
// and this permission notice appear in supporting documentation.

#ifndef __ARGS_H__
#define __ARGS_H__

class args{
	private:
		typedef vector<string> t_args;
   		t_args m_args;

		void Remove(t_args::iterator itt1,int n)
		{
			t_args args2;
			for(t_args::iterator itt2=m_args.begin();itt2!=m_args.end();++itt2) {
				if(itt2!=itt1) args2.push_back(*itt2);
				else {if (--n>0) ++itt1;}
			}
			m_args=args2;
		}

		t_args::iterator Find(const string& v)
		{
			for(t_args::iterator itt=m_args.begin();itt!=m_args.end();++itt) {
				if (v==*itt) return itt;
			}
			return m_args.end();
		}

		t_args::const_iterator Find(const string& v) const
		{
			for(t_args::const_iterator itt=m_args.begin();itt!=m_args.end();++itt) {
				if (v==*itt) return itt;
			}
			return m_args.end();
		}

	public:
		args(const int argc,char **argv,const bool printargs=false)
		{
			for(int i=0;i<argc;++i) {m_args.push_back(argv[i]);}
			if (printargs) cout << "% Call args: " << *this << endl;
		}

		string operator[](const size_t i) const
		{
			if(i>=size()) throw_("argument[] out of range");
			return m_args[i];
		}

		size_t size() const {return m_args.size();}
		bool                hasopt(const string& tag) const {return Find(tag)!=m_args.end();}
		template<class T> T Totype(const size_t i)    const {return totype<T>((*this)[i]);}
		template<class T> T Tounit(const size_t i)    const {
			const string& s=(*this)[i+1];
			#ifndef USE_UNITS
				// fake test of units, primitive test, must be one-of: kpc, per_kpc and msun1E10
				if (s!="kpc/h" && s!="h/kpc" && s!="msun1E10/h") throw_("bad unit=<" + s + ">, can only handle units of type: kpc/h, h/kpc, and msun1E10/h");
			#endif
			return totype<T>((*this)[i] + " " + s);
		}

		template<class T>
		T parseval(const string& tag,const T& defaultval)
		{
			t_args::iterator itt=Find(tag);
			if (itt==m_args.end() || itt+1==m_args.end()) return defaultval;

			const T v=totype<T>(*(++itt));
			Remove(--itt,2);

			return v;
		}

		template<class T,class R>
		pair<T,R> parseval(const string& tag,const T& defaultval1,const R& defaultval2)
		{
			t_args::iterator itt=Find(tag);
			if (itt==m_args.end() || itt+1==m_args.end() || itt+2==m_args.end()) return make_pair(defaultval1,defaultval2);

			const T v1=totype<T>(*(++itt));
			const R v2=totype<R>(*(++itt));
			Remove(----itt,3);

			return make_pair(v1,v2);
		}

		template<class T>
		T parseunit(const string& tag,const T& defaultval)
		{
			t_args::iterator itt=Find(tag);
			if (itt==m_args.end() || itt+2==m_args.end()) return defaultval;

			const string s1=*(++itt);
			const string s2=*(++itt);
			const T v=totype<T>(s1 + " " + s2);
			Remove(----itt,3);

			return v;
		}

		bool parseopt(const string& tag)
		{
			t_args::iterator itt=Find(tag);
			if (itt==m_args.end()) return false;
			Remove(itt,1);

			return true;
		}

		int getoptindex(const string& tag) const
		{
			int n=0;
			for(t_args::const_iterator itt=m_args.begin();itt!=m_args.end();++itt,++n) {
				if (tag==*itt) {
					assert( n>=0 && size_t(n)<m_args.size());
					return n;
				}
			}
			return -1;
		}

		void remove(const size_t i,const size_t n=1)
		{
			if (i>m_args.size() || i+n>m_args.size()) throw_("out of range");
			m_args.erase(m_args.begin()+i,m_args.begin()+i+n);
		}

		friend ostream& operator<<(ostream& s,const args& v)
		{
			for(t_args::const_iterator itt=v.m_args.begin();itt!=v.m_args.end();++itt){
				s <<  *itt << " ";
			}
			return s;
		}
};

#endif // __ARGS_H__
