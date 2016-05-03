/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
//#include <stdlib.h>
#include "PointoolsVortexAPIInternal.h"


#include <pt/Timer.h>

using namespace pt;

// follows are the constructors of the Timer class, once version
// for each OS combination.  The order is WIN32, FreeBSD, Linux, IRIX,
// and the rest of the world.
//
// all the rest of the timer methods are implemented within the header.


const Timer* Timer::instance()
{
    static Timer s_timer;
    return &s_timer;
}

#ifdef _WIN32

    #include <fcntl.h>
    #include <windows.h>
    #include <winbase.h>

	double _tempSecsPerClick=0.0;

    Timer::Timer()
    {
        _useStandardClock = false;
        
        if (_useStandardClock)
        {
            _secsPerTick = (1.0 / (double) CLOCKS_PER_SEC);            
        }
        else
        {
            // use a global here to ensure that the Sleep(..) for 1 sec
            // is not incurred more than once per app execution.
            if (_tempSecsPerClick==0.0)
            {
                Timer_t start_time = tick();
                Sleep (100);
                Timer_t end_time = tick();

                _tempSecsPerClick = 0.1/(double)(end_time-start_time);
            }
            _secsPerTick = _tempSecsPerClick;
        }
    }

#elif defined(__FreeBSD__)

    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <sys/types.h>
    Timer::Timer()
    {
        _useStandardClock = false;
        
        if (_useStandardClock)
        {
            _secsPerTick = 1e-6; // gettimeofday()'s precision.
        }
        else
        {
            int cpuspeed;
            size_t len;

            len = sizeof(cpuspeed);
            if (sysctlbyname("machdep.tsc_freq", &cpuspeed, &len, NULL, NULL) == -1)
            {
                _useStandardClock = true;
            perror("sysctlbyname(machdep.tsc_freq)");
                return;
            }
            _secsPerTick = 1.0/cpuspeed;
        }
    }
    
#elif defined(__linux) 

    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/types.h>

    Timer::Timer()
    {
#ifdef __ia64
        _useStandardClock = true;
#else
        _useStandardClock = false;
#endif
        
        if (_useStandardClock)
        {
            _secsPerTick = 1e-6; // gettimeofday()'s precision.
        }
        else
        {
            char buff[128];
            FILE *fp = fopen( "/proc/cpuinfo", "r" );

            double cpu_mhz=0.0f;

            while( fgets( buff, sizeof( buff ), fp ) > 0 )
            {
                if( !strncmp( buff, "cpu MHz", strlen( "cpu MHz" )))
            {
                char *ptr = buff;

                while( ptr && *ptr != ':' ) ptr++;
                if( ptr ) 
                {
                  ptr++;
                  sscanf( ptr, "%lf", &cpu_mhz );
                }
                break;
            }
            }
            fclose( fp );

            if (cpu_mhz==0.0f)
            {
                // error - no cpu_mhz found.
                Timer_t start_time = tick();
                sleep (1);
                Timer_t end_time = tick();
                _secsPerTick = 1.0/(double)(end_time-start_time);
            }
            else
            {
                _secsPerTick = 1e-6/cpu_mhz;
            }
        }        
    }

#elif defined(__sgi)

    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/syssgi.h>
    #include <sys/mman.h>

    Timer::Timer( void )
    {
        _useStandardClock = false; // default to false.

        if (!_useStandardClock)
        {
            __psunsigned_t phys_addr, raddr;
            unsigned int cycleval;
            volatile unsigned long long *iotimer_addr;
            int fd, poffmask;

            poffmask     = getpagesize() - 1;
            phys_addr    = syssgi( SGI_QUERY_CYCLECNTR, &cycleval );
            raddr        = phys_addr & ~poffmask;

            _clockAddress_32 = 0;
            _clockAddress_64 = 0;
            _rollOver = 0;
            _lastClockValue = 0;

            if( (fd = open( "/dev/mmem", O_RDONLY )) < 0 )
            {
                perror( "/dev/mmem" );
                _useStandardClock=true;
                return;
            }

            iotimer_addr = (volatile unsigned long long *)mmap(
                (void *)0L,
                (size_t)poffmask,
                (int)PROT_READ,
                (int)MAP_PRIVATE, fd, (off_t)raddr);

            iotimer_addr = (unsigned long long *)(
                (__psunsigned_t)iotimer_addr + (phys_addr & poffmask)
                );

            _cycleCntrSize = syssgi( SGI_CYCLECNTR_SIZE );

            // Warning:  this casts away the volatile; not good
            if( _cycleCntrSize > 32 )
                _clockAddress_32 = 0,
                _clockAddress_64 = (unsigned long long *) iotimer_addr;
            else
                _clockAddress_32 = (unsigned long *) iotimer_addr,
                _clockAddress_64 = 0;

            _secsPerTick = (double)(cycleval)* 1e-12;
            
#if 0 // Obsolete
            // this is to force the use of the standard clock in
            // instances which the realtime clock is of such a small
            // size that it will loop too rapidly for proper realtime work.
            // this happens on the O2 for instance.
            if (_cycleCntrSize<=32) _useStandardClock=true;
#endif // Obsolete
            
        }

        if (_useStandardClock)
        {
            _secsPerTick = 1e-6; // gettimeofday()'s precision.
        }

    }

#elif defined (__DARWIN_OSX__)  || defined (macintosh)

    #if defined (__DARWIN_OSX__)
        #include <Carbon/Carbon.h>         // do I really have to link against the Carbon framework just for this?
    #else    
        #include <MacTypes.h>
        #include <Timer.h>
    #endif


    Timer::Timer( void )
    {
        _useStandardClock = false;
        _secsPerTick = 1e-6; // Carbon timer's precision.

    }

    Timer_t Timer::tick(void) const
    {
        UnsignedWide usecs;
        Microseconds(&usecs);

        return (usecs.hi * 4294967296.0) + usecs.lo;
    }

#elif defined(unix)

    Timer::Timer( void )
    {
        _useStandardClock = true;
        _secsPerTick = 1e-6; // gettimeofday()'s precision.
    }

#else 

    // handle the rest of the OS world by just using the std::clock,

    Timer::Timer( void )
    {
        _useStandardClock = true;
        _secsPerTick = (1.0 / (double) CLOCKS_PER_SEC);
    }

#endif
