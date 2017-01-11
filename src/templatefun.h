#ifndef __TEMPLATE_FUN__
#define __TEMPLATE_FUN__

template<class T>
class Autobool
{
private:
	volatile T* m_b;
	Autobool(const Autobool&);
	void operator=(const Autobool&);
public:
	Autobool(volatile T* b) : m_b(b) {assert(m_b && *m_b==false); *m_b=true;}
	~Autobool()                      {assert(m_b && *m_b==true);  *m_b=false; m_b=0;}
};

template<class T,class R>
class DeviceAutoClose
{
private:	
	T m_dev;
	R (__stdcall *m_fun)(void *);
	bool m_init;

	// private Copy CTOR and assignment operator
	DeviceAutoClose(const DeviceAutoClose<T,R>&);
	void    operator=(const DeviceAutoClose<T,R>&);

public:
	DeviceAutoClose(T dev,R(__stdcall *fun)(void*)) : m_dev(dev), m_fun(fun), m_init(true) 
	{
		FUNSTACK;
		assert(m_fun!=NULL);
	}
	~DeviceAutoClose() 
	{
		FUNSTACK;
		assert(m_init);
		assert(m_fun!=NULL);
		if (m_dev!=NULL){
			R r=m_fun(m_dev);
			if (!r) throw_("DeviceClose() failed"); // throw in DTOR -> bad, bad!
		}
		m_dev=NULL;
		m_fun=NULL;
		m_init=false;
	}
	const T& operator()() const {FUNSTACK; assert(m_init); return m_dev;}
	T&       operator()()       {FUNSTACK; assert(m_init); return m_dev;}
};


#endif // __TEMPLATE_FUN__