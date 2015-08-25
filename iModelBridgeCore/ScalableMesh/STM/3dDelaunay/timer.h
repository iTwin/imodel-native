#pragma once
#include <windows.h>
#include <amp.h>

#ifdef DHDEBUG
//#define ENABLEAUTOTIMER
#endif
using namespace concurrency;

class Timer
    {
    private:
        LARGE_INTEGER m_Start;
        LARGE_INTEGER m_Stop;
        LARGE_INTEGER m_Freq;
        __int64 m_Overhead;
        bool m_started;
    public:
        Timer ()
            {
            m_started = false;
            QueryPerformanceFrequency (&m_Freq);
            start ();
            stop ();
            m_Overhead = m_Stop.QuadPart - m_Start.QuadPart;
            }

        void start ()
            {
            if (m_started)
                return;
            m_started = true;
            QueryPerformanceCounter (&m_Start);
            }

        void stop ()
            {
            if (!m_started)
                return;
            m_started = false;
            QueryPerformanceCounter (&m_Stop);
            }

        // return seconds
        double read ()
            {
            return double (m_Stop.QuadPart - m_Start.QuadPart - m_Overhead) / m_Freq.QuadPart;
            }
        bool HasStarted ()
            {
            return m_started;
            }
    };

class gpu_timer : private Timer
    {
    public:
        gpu_timer () : _accl (accelerator::default_accelerator)
            {
            }
        explicit gpu_timer (accelerator& accl) : _accl (accl)
            {
            }

        inline void start ()
            {
            _accl.default_view.wait ();
            Timer::start ();
            }

        inline void stop ()
            {
            _accl.default_view.wait ();
            Timer::stop ();
            }

        inline double read ()
            {
            return Timer::read ();
            }

    private:
        accelerator _accl;
    };

#ifdef ENABLEAUTOTIMER
#include <iomanip>
struct AutoTimerCounter
    {
    combinable<double> took;

    AutoTimerCounter ()
        {
//        took = 0;
        }
    void Print (const char* message)
        {
        double total = 0;
        took.combine_each ([&](double v)
            {
            total += v;
            });
        std::cout << std::setprecision (8) << message << " elapsed_time=" << total << "(sec.)\n";
        }
    };

struct AutoTimer : private Timer
    {
    AutoTimerCounter& m_counter;

    AutoTimer (AutoTimerCounter& counter, bool autoStart = true) : m_counter (counter)
        {
        if (autoStart)
            start ();
        }

    ~AutoTimer ()
        {
        Stop ();
//        long took = (double)(read () * 10000000);
//        
//        atomic_add<long> (m_counter.took, took);
////        m_counter.took += read ();
        }

    void Start ()
        {
        start ();
        }
    void Stop ()
        {
        if (HasStarted ())
            {
            stop ();
            m_counter.took.local () += read ();
            }
        }
    };
#else
struct AutoTimerCounter
    {
    void Print (const char* message)
        {
        }
    };

struct AutoTimer : private Timer
    {
    AutoTimer (AutoTimerCounter& counter, bool autoStart = true)
        {
        }

    ~AutoTimer ()
        {
        }

    void Start ()
        {
        }
    void Stop ()
        {
        }
    };

#endif