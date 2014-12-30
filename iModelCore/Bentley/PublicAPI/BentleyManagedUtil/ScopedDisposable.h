/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BentleyManagedUtil/ScopedDisposable.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#if !defined (_MANAGED)
#error This header is intended to be included only in a managed region of a /CLR compiland.
#endif

#include <gcroot.h>

#pragma unmanaged
#include <BaseTsd.h>            // Windows Platform SDK header file, defines INT_PTR
#pragma managed

namespace Bentley { namespace Interop {

class ScopedDisposable
    {
    private: gcroot<System::IDisposable *>   m_disposableObject;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    BernMcCarty   12/04
    +---------------+---------------+---------------+---------------+---------------+------*/
    public: ScopedDisposable (System::IDisposable *disposableObject)
        {
        m_disposableObject = disposableObject;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    BernMcCarty   12/04
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~ScopedDisposable()
        {
        if (NULL != m_disposableObject)
            {
            m_disposableObject.Dispose();
            m_disposableObject = NULL;
            }
        }

    };

}

}

