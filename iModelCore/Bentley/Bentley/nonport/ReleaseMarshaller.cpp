/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/ReleaseMarshaller.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
    #include <windows.h>

#include <Bentley/Bentley.h>
#include <Bentley/ReleaseMarshaller.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ReleaseMarshaller::InitializeForSystem ()
    {
    if (s_csInitialized)
        return;

    InitializeCriticalSectionAndSpinCount (&s_criticalSection, 4000);
    s_threadLocalStorageIndex = TlsAlloc();

    s_csInitialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ReleaseMarshaller::InitializeForThread()
    {
    InitializeForSystem ();

    ReleaseMarshaller* marshaller = new ReleaseMarshaller();

    TlsSetValue(s_threadLocalStorageIndex, marshaller);
    marshaller->m_queue = new ReleaseMarshallerQueue_T();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ReleaseMarshaller* ReleaseMarshaller::GetMarshaller()
    {
    ReleaseMarshaller* marshaller = (ReleaseMarshaller*) TlsGetValue (s_threadLocalStorageIndex);
    if (NULL == marshaller)
        {
        InitializeForThread ();
        marshaller = (ReleaseMarshaller*)TlsGetValue(s_threadLocalStorageIndex);
        }

    if (marshaller->m_queue->size() != 0)
        marshaller->ReleaseAll ();

    return marshaller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ReleaseMarshaller::UninitializeForThread()
    {
    ReleaseMarshaller* marshaller = GetMarshaller();
    if (NULL == marshaller)
        return;

#if defined (CANT_DO_PURE_NATIVE)
    System::GC::Collect();
    System::GC::WaitForPendingFinalizers();
#endif

    TlsSetValue(s_threadLocalStorageIndex, NULL);

    EnterCriticalSection (&s_criticalSection);
    ReleaseMarshallerQueue_T* current = marshaller->m_queue;
    marshaller->m_queue = NULL;
    LeaveCriticalSection(&s_criticalSection);

    marshaller->ReleaseQueueContents(*current);
    delete current;

    delete marshaller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ReleaseMarshaller::ReleaseQueueContents(ReleaseMarshallerQueue_T& queue)
    {
    for (ReleaseMarshallerQueueIterator_T iterator = queue.begin(); iterator != queue.end(); iterator++)
        {
        BentleyApi::IRefCounted* counted = *iterator;
        if (NULL != counted)
            counted->Release();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ReleaseMarshaller::ReleaseAll ()
    {
    if (m_queue->size() == 0)
        return;

    ReleaseMarshallerQueue_T* newQueue = new ReleaseMarshallerQueue_T();

    EnterCriticalSection(&s_criticalSection);
    ReleaseMarshallerQueue_T* current = m_queue;
    m_queue = newQueue;
    LeaveCriticalSection(&s_criticalSection);

    ReleaseQueueContents(*current);
    delete current;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            ReleaseMarshaller::QueueEntry (IRefCounted*entry)
    {
    if (NULL == entry)
        return;

    EnterCriticalSection(&s_criticalSection);
    m_queue->push_back(entry);
    LeaveCriticalSection(&s_criticalSection);
    }

DWORD                       ReleaseMarshaller::s_threadLocalStorageIndex;
CRITICAL_SECTION            ReleaseMarshaller::s_criticalSection;
bool                        ReleaseMarshaller::s_csInitialized;

#else
    #error This file is only valid for Windows desktop!
#endif
