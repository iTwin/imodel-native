/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/MSSmartPtr.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
\addtogroup ScopeManagement
Scope management classes use C++ constructor and destructor
calls at scope entry and exit time to guarantee corresponding
setup and teardown operations.
* @bsiclass
+===============+===============+===============+===============+===============+======*/

/*=================================================================================**//**
\ingroup ScopeManagement
* Templatized class to create a shallow copy of a variable that is automatically restored to it's original
* value when the AutoRestore instance goes out of scope.
*
* Use:
*    AutoRestore<DPoint3d> savePoint (&point);      // Will be restored when out of scope
*    ...
*    point.x = 0;
*    ...
*
* When "savePoint" goes out of scope, "point" will be restored to its original value
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <class T> class AutoRestore
    {
    private:
        T   m_original;     // value of the original variable
        T*  m_addr;         // address of the original variable so that we can restore it later

    public:
    AutoRestore (T* original)
        {
        m_original = *original;
        m_addr     = original;
        }

    AutoRestore (T* original, T newVal)     // save original and then set to new value
        {
        m_addr     = original;
        m_original = *original;
        *original  = newVal;
        }

    ~AutoRestore ()
        {
        *m_addr = m_original;
        }
    T GetOriginal() {return m_original;}
    };


END_BENTLEY_DGN_NAMESPACE

