/*----------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/ReleaseMarshaller.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include "Bentley.h"
#include "RefCounted.h"
#include "bvector.h"
#include <windows.h>

BEGIN_BENTLEY_NAMESPACE

//  The ReleaseMarshaller is used when implementing .net wrappers classes for native classes.
//  Our wrapper classes often contain a pointer to a IRefCounted class. When that is the
//  case, the wrapper class usually needs to be IDisposable and have a Finalizer.
//  Declare a ReleaseMarshaller* as a member, initialize it in the constructor of your
//  class, and then implement the Dispose method and Finalizer as follows:
//
// Dispose method:
// Light::~Light()
//    {
//    if ( (NULL == m_marshaller) || (NULL == m_native) )
//        return;
//
//    m_native->Release();
//    m_native = NULL;
//    }
// 
// Finalizer:
// Light::!Light()
//    {
//    if ( (NULL == m_marshaller) || (NULL == m_native) )
//        return;
//
//    m_marshaller->QueueEntry (m_native);
//    m_native = NULL;
//    }
//
// 
//  The ReleaseMarshaller works by keeping a queue of IRefCounted*'s waiting to be
//  Released. When another instance is contructed that uses the ReleaseMarshaller
//  mechanism, all the queued up instances are released, in the thread where they
//  were created (rather than the Finalizer thread).

typedef bvector<BentleyApi::IRefCounted*>   ReleaseMarshallerQueue_T;
typedef ReleaseMarshallerQueue_T::iterator  ReleaseMarshallerQueueIterator_T;

class ReleaseMarshaller
{
private:
    static DWORD                s_threadLocalStorageIndex; 
    static CRITICAL_SECTION     s_criticalSection;
    static bool                 s_csInitialized;

    ReleaseMarshallerQueue_T*   m_queue;

    void ReleaseQueueContents(ReleaseMarshallerQueue_T& queue);
    void ReleaseAll ();


public:
    BENTLEYDLL_EXPORT static void InitializeForSystem ();
    BENTLEYDLL_EXPORT static void InitializeForThread();
    BENTLEYDLL_EXPORT static void UninitializeForThread();
    BENTLEYDLL_EXPORT static void Shutdown ();

    BENTLEYDLL_EXPORT static ReleaseMarshaller* GetMarshaller();

    BENTLEYDLL_EXPORT void QueueEntry (IRefCounted*entry);
};

END_BENTLEY_NAMESPACE



