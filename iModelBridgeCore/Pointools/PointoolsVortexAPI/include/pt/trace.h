/*--------------------------------------------------------------------------*/ 
/*  Trace.h																	*/ 
/*	Tracing (Logging) class													*/ 
/*  (C) 2005 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 12 Jan 2005 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef COMMONCLASSESTRACER
#define COMMONCLASSESTRACER
#include <pt/classes.h>
#include <iostream>
#include <fstream>

#include <boost/thread/mutex.hpp>

//#define FILE_TRACE 1

namespace pt
{
	//
	// trace context
	//
	struct TraceContext
	{
		TraceContext() : context(0) {};
		inline void push() { context++; }
		inline void pop() { context--; if (context <0) context = 0; }
		int context;
	};
	CCLASSES_API TraceContext *tracecontext();
	
	//
	// Trace Output
	//
	template <class S>
	struct TraceOut
	{
		inline TraceOut() 	{ for (int i=0; i<tracecontext()->context+1; i++) S() << "    "; }
		inline ~TraceOut()	{ S() << "\n"; }
		template <class T> inline S &operator << (const T &d)		{ return S() << d; };
	};
	template <class S>
	struct DummyTraceOut
	{
		inline DummyTraceOut() {}
		inline ~DummyTraceOut(){}
		template <class T> inline S &operator << (const T &d) { return S() << d; };
	};
	struct DummyTrace
	{
		template <class T> DummyTrace &operator << (const T &)		{ return *this; };
	};
	struct CoutTrace
	{
		template <class T>	CoutTrace &operator << (const T &d)		{ std::cout << d; return *this;};
	};
	struct CCLASSES_API FileTrace
	{
		static boost::mutex &mutex();  
		static std::ofstream &file();
		template <class T> FileTrace &operator << (const T &d)
		{ 
			boost::mutex::scoped_lock lock(mutex());
			file() << d << std::flush; return *this; 
		};
	};
	//
	// Trace Object
	//
	template <class S>
	struct Trace
	{
		inline Trace(const char*f)
		{
			func = f;
			S() << ">" << func;
			tracecontext()->push();

		}
		inline ~Trace()
		{
			tracecontext()->pop();
			S() << "<" << func;
		}
		const char *func;
	};
	
	template <class S>
	struct FuncTrace
	{
		inline FuncTrace(const char *func)
		{
			S() << ">" << func;
			tracecontext()->push();
		}
		inline FuncTrace(const char *func, const char *file, int line)
		{
			S() << ">" << func << ": " << file << ": " << line;
			tracecontext()->push();
		}
		inline ~FuncTrace()
		{
			tracecontext()->pop();
			S() << "<";
		}
	};

	struct DoNothing
	{
		inline static void doNothing() {};		
	};
}
#ifdef FILE_TRACE
	#define PTTRACE_LINE (PTTRACEOUT << __FILE__ << ":" << __LINE__);
	#define PTTRACE_FUNC pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj(__FUNCTION__);
	#define PTTRACE_FUNC_POS pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj(__FUNCTION__, __FILE__,__LINE__);
	#define PTTRACE(a) pt::Trace<pt::TraceOut<pt::FileTrace> > _trace_obj(a)
	#define PTTRACEOUT pt::TraceOut<pt::FileTrace>()
#else
	#ifdef COUT_TRACE
		#define PTTRACE_LINE (PTTRACEOUT << __FILE__ << ":" << __LINE__);
		#define PTTRACE_FUNC pt::FuncTrace<pt::TraceOut<pt::CoutTrace> > _trace_obj(__FUNCTION__);
		#define PTTRACE_FUNC_POS pt::FuncTrace<pt::TraceOut<pt::CoutTrace> > _trace_obj(__FUNCTION__, __FILE__,__LINE__);
		#define PTTRACE(a) pt::Trace<pt::TraceOut<pt::CoutTrace> > _trace_obj(a)
		#define PTTRACEOUT pt::TraceOut<pt::CoutTrace>()
	#else
		/*optimising compliler should remove this call*/ 
		#define PTTRACE_LINE 
		#define PTTRACE_FUNC 
		#define PTTRACE_FUNC_POS 
		#define PTTRACE(a)
		#define PTTRACEOUT pt::DummyTraceOut<pt::DummyTrace>()
	#endif
#endif

#endif