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
        template <class T> inline S &operator << (const T &d) { m_s << d; return m_s; };

        S m_s;
	};
	struct DummyTrace
	{
		template <class T> inline DummyTrace &operator << (const T &) { return *this; };
	};
	struct CoutTrace
	{
		template <class T>	CoutTrace &operator << (const T &d)		{ std::cout << d; return *this;};
	};
	struct CCLASSES_API FileTrace
	{
		static std::mutex &mutex();  
		static std::wofstream &file();
		template <class T> FileTrace &operator << (const T &d)
		{ 
            std::lock_guard<std::mutex> lock(mutex());
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
			wfunc = 0;
			S() << ">" << func;
			tracecontext()->push();

		}
		inline Trace(const wchar_t*f)
		{
			func = 0;
			wfunc = f;
			S() << ">" << func;
			tracecontext()->push();

		}
		inline ~Trace()
		{
			tracecontext()->pop();
			S() << "<" << (func ?  func : wfunc);
		}
		const char *func;
		const char *wfunc;
	};
	
	template <class S>
	struct FuncTrace
	{
		inline FuncTrace(void)	{};

		inline FuncTrace(const char *func)
		{
			S() << func;
			tracecontext()->push();
		}
		inline FuncTrace(const char *func, const char *file, int line)
		{
			S()  << func << ": " << file << ": " << line;
			tracecontext()->push();
		}

		template <typename P1>
		void func(const char *func, P1 p1)
		{
			S()  << func << "( " << p1 << " ); ";
			tracecontext()->push();
		}
		template <typename P1, typename P2>
		void func(const char *func, P1 p1, P2 p2)
		{
			S()  << func << "( " << p1 << ", " << p2 << " ); ";
			tracecontext()->push();
		}
		template <typename P1, typename P2, typename P3>
		void func(const char *func, P1 p1, P2 p2, P3 p3)
		{
			S()  << func << "( " << p1 << ", " << p2 <<  ", " << p3 << " ); ";
			tracecontext()->push();
		}
		template <typename P1, typename P2, typename P3, typename P4>
		void func(const char *func, P1 p1, P2 p2, P3 p3, P4 p4)
		{
			S()  << func << "( " << p1 << ", " << p2 <<  ", " << p3 << ", " << p4 << " ); ";
			tracecontext()->push();
		}
		template <typename P1, typename P2, typename P3, typename P4, typename P5>
		void func(const char *func, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
		{
			S()  << func << "( " << p1 << ", " << p2 <<  ", " << p3 << ", " << p4 <<  ", " << p5 << " ); ";
			tracecontext()->push();
		}
		template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
		void func(const char *func, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
		{
			S()  << func << "( " << p1 << ", " << p2 <<  ", " << p3 << ", " << p4 <<  ", " << p5 <<  ", " << p6 << " ); ";
			tracecontext()->push();
		}
		template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
		void func(const char *func, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
		{
			S()  << func << "( " << p1 << ", " << p2 <<  ", " << p3 << ", " << p4 <<  ", " << p5 <<  ", " << p6  << ", " << p7 << " ); ";
			tracecontext()->push();
		}
		template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
		void func(const char *func, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
		{
			S()  << func << "( " << p1 << ", " << p2 <<  ", " << p3 << ", " << p4 <<  ", " << p5 <<  ", " << p6  << ", " << p7 << ", " << p8 << " ); ";
			tracecontext()->push();
		}
		inline ~FuncTrace()
		{
			tracecontext()->pop();
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

	#define PTTRACE_FUNC_P1(p1) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1));
	#define PTTRACE_FUNC_P2(p1, p2) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1), (p2));
	#define PTTRACE_FUNC_P3(p1, p2, p3) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1), (p2), (p3));
	#define PTTRACE_FUNC_P4(p1, p2, p3, p4) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1), (p2), (p3), (p4));
	#define PTTRACE_FUNC_P5(p1, p2, p3, p4, p5) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1), (p2), (p3), (p4), (p5));
	#define PTTRACE_FUNC_P6(p1, p2, p3, p4, p5, p6) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1), (p2), (p3), (p4), (p5), (p6));
	#define PTTRACE_FUNC_P7(p1, p2, p3, p4, p5, p6, p7) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1), (p2), (p3), (p4), (p5), (p6), (p7));
	#define PTTRACE_FUNC_P8(p1, p2, p3, p4, p5, p6, p7, p8) pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj; _trace_obj.func(__FUNCTION__, (p1), (p2), (p3), (p4), (p5), (p6), (p7), (p8));

	#define PTTRACE_FUNC_POS pt::FuncTrace<pt::TraceOut<pt::FileTrace> > _trace_obj(__FUNCTION__, __FILE__,__LINE__);
	#define PTTRACE(a) pt::Trace<pt::TraceOut<pt::FileTrace> > _trace_obj(a)
	#define PTTRACEOUT pt::TraceOut<pt::FileTrace>()
#else
	#ifdef COUT_TRACE
		#define PTTRACE_LINE (PTTRACEOUT << __FILE__ << ":" << __LINE__);
		#define PTTRACE_FUNC pt::FuncTrace<pt::TraceOut<pt::CoutTrace> > _trace_obj(__FUNCTION__);
		#define PTTRACE_FUNC_POS pt::FuncTrace<pt::TraceOut<pt::CoutTrace> > _trace_obj(__FUNCTION__, __FILE__,__LINE__);
		#define PTTRACE_FUNC_P1 PTTRACE_FUNC
		#define PTTRACE_FUNC_P2 PTTRACE_FUNC
		#define PTTRACE_FUNC_P3 PTTRACE_FUNC
		#define PTTRACE_FUNC_P4 PTTRACE_FUNC
		#define PTTRACE_FUNC_P5 PTTRACE_FUNC
		#define PTTRACE_FUNC_P6 PTTRACE_FUNC
		#define PTTRACE(a) pt::Trace<pt::TraceOut<pt::CoutTrace> > _trace_obj(a)
		#define PTTRACEOUT pt::TraceOut<pt::CoutTrace>()
	#else
		#define PTTRACE_LINE 
		#define PTTRACE_FUNC 
		#define PTTRACE_FUNC_P1(p1) 
		#define PTTRACE_FUNC_P2(p1, p2) 
		#define PTTRACE_FUNC_P3(p1, p2, p3) 
		#define PTTRACE_FUNC_P4(p1, p2, p3, p4)
		#define PTTRACE_FUNC_P5(p1, p2, p3, p4, p5) 
		#define PTTRACE_FUNC_P6(p1, p2, p3, p4, p5, p6) 
		#define PTTRACE_FUNC_P7(p1, p2, p3, p4, p5, p6, p7) 
		#define PTTRACE_FUNC_P8(p1, p2, p3, p4, p5, p6, p7, p8)
		#define PTTRACE_FUNC_POS 
		#define PTTRACE(a)
		#define PTTRACEOUT pt::DummyTraceOut<pt::DummyTrace>()
	#endif
#endif

#endif