/*--------------------------------------------------------------------------*/ 
/*  BoundingBox.h															*/ 
/*	Axis Aligned Bounding Rect class definition								*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK | All Rights Reserved				*/ 
/*																			*/ 
/*	Adapted from OSG::Timer class											*/ 
/*  Last Updated 12 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 

#ifndef POINTOOLS_TIMER
#define POINTOOLS_TIMER 1

#include <pt/classes.h>


#if defined(_MSC_VER)
    namespace pt {
        typedef int64_t Timer_t;
    }
#elif defined(__linux) || defined(__FreeBSD__) || defined(__CYGWIN__)|| defined(__MINGW32__)
    namespace pt {
        typedef unsigned long long Timer_t;
    }
#elif defined(__sgi)
    namespace pt {
        typedef unsigned long long Timer_t;
    }
#elif defined(unix)
    namespace pt {
        typedef unsigned long long Timer_t;
    }
#elif defined __APPLE__ || defined macintosh
    namespace pt {
        typedef double Timer_t;
    }
#else
    #include <ctime>
    namespace pt {
        typedef std::clock_t Timer_t;
    }
#endif

namespace pt {

/** A high resolution, low latency time stamper.*/ 
class CCLASSES_API Timer {

    public:

        Timer();
        ~Timer() {}

        static const Timer* instance();


    #if defined __DARWIN_OSX__  || defined macintosh
        // PJA MAC OSX - inline Tick() pollutes namespace so badly 
        // we cant compile, due to Carbon.h ...
            Timer_t tick() const;
    #else
        inline Timer_t tick() const;
    #endif
        
        inline double delta_s( Timer_t t1, Timer_t t2 ) const { return (double)(t2 - t1)*_secsPerTick; }
        inline double delta_m( Timer_t t1, Timer_t t2 ) const { return delta_s(t1,t2)*1e3; }
        inline double delta_u( Timer_t t1, Timer_t t2 ) const { return delta_s(t1,t2)*1e6; }
        inline double delta_n( Timer_t t1, Timer_t t2 ) const { return delta_s(t1,t2)*1e9; }
        
        inline double getSecondsPerTick() const { return _secsPerTick; }

    protected :

        double                          _secsPerTick;
        bool                            _useStandardClock;
       
#       ifdef __sgi
        unsigned long*                  _clockAddress_32;
        unsigned long long*             _clockAddress_64;
        int                             _cycleCntrSize;
    // for SGI machines with 32 bit clocks.
        mutable unsigned long           _lastClockValue;
        mutable unsigned long long      _rollOver;
#       endif
        
};

}

#if defined(_MSC_VER)

    #include <time.h>
    #pragma optimize("",off)

    namespace pt{

#ifndef _M_X64
        inline Timer_t Timer::tick( void ) const
        {
            if (_useStandardClock) return clock();

            volatile Timer_t ts;
            volatile unsigned int HighPart;
            volatile unsigned int LowPart;
            _asm
            {
                xor eax, eax        //  Used when QueryPerformanceCounter()
                xor edx, edx        //  not supported or minimal overhead
                _emit 0x0f          //  desired
                _emit 0x31          //
                mov HighPart,edx
                mov LowPart,eax
            }
            //ts = LowPart | HighPart >> 32;
            *((unsigned int*)&ts) = LowPart;
            *((unsigned int*)&ts+1) = HighPart;
            return ts;
        }
#else
		inline Timer_t Timer::tick( void ) const
		{
			return clock();
		}
#endif
    }
    #pragma optimize("",on)

#elif defined(__MINGW32__)

    #include <sys/time.h>

    #define CLK(x)      __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x))
    namespace pt{

    inline Timer_t Timer::tick() const
    {
        if (_useStandardClock)
            return clock();
        else
        {
            Timer_t x;CLK(x);return x;
        }
    }

    }

#elif defined(__linux) || defined(__FreeBSD__) || defined(__CYGWIN__)

    #include <sys/time.h>

#ifdef __ia64
    #define CLK(x)        ((x)=0)
#else
    #define CLK(x)      __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x))
#endif

    namespace pt{

        inline Timer_t Timer::tick() const
        {
            if (_useStandardClock)
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                return ((osg::Timer_t)tv.tv_sec)*1000000+(osg::Timer_t)tv.tv_usec;
            }
            else
            {
                Timer_t x;CLK(x);return x;
            }
        }

    }
  
#elif defined(__sgi)

    #include <sys/types.h>
    #include <sys/time.h>

    namespace pt{

        inline  Timer_t Timer::tick() const
        {
            if (_useStandardClock)
            {
                struct timeval tv;
                gettimeofday(&tv, NULL);
                return ((osg::Timer_t)tv.tv_sec)*1000000+(osg::Timer_t)tv.tv_usec;
            }
            else
            {
                if ( _clockAddress_64 )
                    return *_clockAddress_64;
                else
                {
                    unsigned long clockValue = *_clockAddress_32;
                    if( _lastClockValue > clockValue )
                    {
                        # ifdef __GNUC__
                        _rollOver += 0x100000000LL;
                        #else
                        _rollOver += 0x100000000L;
                        #endif
                    }
                    _lastClockValue = clockValue;
                    return _rollOver + clockValue;
                }
            }
        }
    }
    
#elif defined(unix)

    #include <sys/time.h>

    namespace pt{
        inline  Timer_t Timer::tick() const
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return ((osg::Timer_t)tv.tv_sec)*1000000+(osg::Timer_t)tv.tv_usec;
        }
    }

#elif !defined (__DARWIN_OSX__) && !defined (macintosh)

    // no choice, always use std::clock()
    namespace osg{

        inline  Timer_t Timer::tick( void ) const { return std::clock(); }
    }

#endif

// note, MacOSX compiled in the Timer.cpp.

#endif
