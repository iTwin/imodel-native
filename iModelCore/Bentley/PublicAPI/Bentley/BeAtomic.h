/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeAtomic.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>

#if !defined (_WIN32)
    #define BENTLEY_HAVE_STD_ATOMIC
#else
    #if (_MSC_VER > 1600) && !defined (_MANAGED)
        #define BENTLEY_HAVE_STD_ATOMIC
    #endif
#endif

#if defined (BENTLEY_HAVE_STD_ATOMIC)

#include <atomic>

#ifdef BeAtomic
#undef BeAtomic
#endif

template<class T> struct BeAtomic : std::atomic<T>
{
    explicit BeAtomic(T val=0) : std::atomic<T>(val){}
    T IncrementAtomicPre() {return this->fetch_add(1)+1;}
    T DecrementAtomicPost() {return this->fetch_sub(1);}
};

#else

    #define INTERLOCKED_FUNCTIONS(TYPE_SUFFIX,TYPE)\
        extern "C" TYPE __cdecl _InterlockedIncrement ## TYPE_SUFFIX (TYPE volatile*);\
        extern "C" TYPE __cdecl _InterlockedDecrement ## TYPE_SUFFIX (TYPE volatile*);\
        extern "C" TYPE __cdecl _InterlockedExchange ## TYPE_SUFFIX (TYPE volatile*, TYPE);\
        extern "C" TYPE __cdecl _InterlockedExchangeAdd ## TYPE_SUFFIX (TYPE volatile*, TYPE);\
        extern "C" TYPE __cdecl _InterlockedCompareExchange ## TYPE_SUFFIX (TYPE volatile *, TYPE, TYPE);\
        
    extern "C" void* __cdecl _InterlockedExchangePointer (void* volatile*, void*);
    extern "C" void* __cdecl _InterlockedCompareExchangePointer (void* volatile*, void*, void*);
    
    INTERLOCKED_FUNCTIONS (, long)
    INTERLOCKED_FUNCTIONS (64, __int64)

    BEGIN_BENTLEY_NAMESPACE

    // Note: the default constructor for std::atomic does not 0-initialize the value. That's why BeAtomic does not 0-initialize m_value below.

    #define BE_ATOMIC_IMPL(UTYPE,TYPE_SUFFIX,CAST_TYPE)\
        private:\
            UTYPE volatile m_value;\
            BeAtomic& operator= (BeAtomic const&);\
            BeAtomic (BeAtomic const&);\
        public:\
            explicit BeAtomic (UTYPE v=0) : m_value(v) {}\
            UTYPE operator++() volatile {return _InterlockedIncrement ## TYPE_SUFFIX ((CAST_TYPE volatile*)&m_value);}\
            UTYPE operator++(int) volatile {return _InterlockedIncrement ## TYPE_SUFFIX ((CAST_TYPE volatile*)&m_value)-1;}\
            UTYPE operator--()    volatile {return _InterlockedDecrement ## TYPE_SUFFIX ((CAST_TYPE volatile*)&m_value);}\
            UTYPE operator--(int) volatile {return _InterlockedDecrement ## TYPE_SUFFIX ((CAST_TYPE volatile*)&m_value)+1;}\
            UTYPE IncrementAtomicPre() {return ++(*this);} \
            UTYPE DecrementAtomicPost() {return (*this)--;} \
            UTYPE load() const {return _InterlockedCompareExchange ## TYPE_SUFFIX ((CAST_TYPE volatile*)&m_value,0,0);}\
            void store (UTYPE v) const {_InterlockedExchange ## TYPE_SUFFIX ((CAST_TYPE volatile*)&m_value,v);}

    //=======================================================================================
    //! std::atomic<UTYPE> work-alike for use by C++/CLI managed code
    //  @bsiclass 
    //=======================================================================================
    template<typename T> struct BeAtomic
        {
        };

    template<> struct BeAtomic<uint64_t>
        {
        BE_ATOMIC_IMPL(uint64_t,64,long long)
        };

    template<> struct BeAtomic<uint32_t>
        {
        BE_ATOMIC_IMPL(uint32_t,,long)
        };

    template<> struct BeAtomic<int>
        {
        BE_ATOMIC_IMPL(int,,long)
        };

    END_BENTLEY_NAMESPACE

#endif
