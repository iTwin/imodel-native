/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/WorkerThread.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <Bentley/Tasks/WorkerThread.h>

USING_NAMESPACE_BENTLEY_TASKS


/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThread::WorkerThread (Utf8CP name) 
: AsyncTaskRunnerPool (1, name)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThread::~WorkerThread ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t WorkerThread::GetThreadId ()
    {
    if (m_runners.empty ())
        {
        return 0;
        }
    return m_runners.front ()->GetId ();
    }
