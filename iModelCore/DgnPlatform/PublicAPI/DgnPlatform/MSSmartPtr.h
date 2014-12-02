/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/MSSmartPtr.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/


#include "DgnCore/ElementRef.h"
#include "DgnCore/MSElementDescr.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
\addtogroup ScopeManagement
Scope management classes use C++ constructor and destructor
calls at scope entry and exit time to guarantee corresponding
setup and teardown operations.
* @bsiclass
+===============+===============+===============+===============+===============+======*/

/*=================================================================================**//**
\ingroup ScopeManagement
Templatized "Smart pointer".
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <class _Ty, class _Base>
class MstnSmartP : protected _Base
{
protected:
    _Ty   m_ptr;

    MstnSmartP (MstnSmartP const&);            // no copying!
    MstnSmartP& operator= (MstnSmartP const&); // no assignment!

public:
    MstnSmartP ()               {m_ptr = NULL;}
    MstnSmartP (_Ty from)       {m_ptr = from;}
    ~MstnSmartP ()              {Free();}

    _Ty SetPtr (_Ty p)
        {
        if (p != m_ptr)
            Free();
        return (m_ptr=p);
        }

    void Free()                 {if (m_ptr) this->_Free (&m_ptr);}
    _Ty* GetPtr ()              {return &m_ptr;}
    _Ty  GetPtrVal()            {return m_ptr;}
    _Ty operator ->() const     {return m_ptr;};
    operator _Ty() const        {return m_ptr;}
    bool operator!() const      {return (m_ptr == NULL);}
    MstnSmartP& operator= (_Ty p)  {SetPtr (p); return *this;}
};



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

/*=================================================================================**//**
*
* A smart pointer that holds a copy of the pointer that you pass to it. When you
* copy a CopiedPtr, the copy makes its own copy of the pointer.
*
* NB: If the actual pointer points to an object of a derived class, then CopiedPtr will
*       not make a copy of the derived class but will make a new instance of the base class.
*       If you want to use polymorphic pointers, write a "ClonedPtr" template that calls
*       a virtual "Clone" method instead of copy constructors.
*
* @bsiinterface                                 Sam.Wilson                      07/2009
+===============+===============+===============+===============+===============+======*/
template <typename T>
class           CopiedPtr
{
private:
    T*          m_ptr;

    void        FreePtr ()
        {
        if (NULL != m_ptr)
            {
            delete m_ptr;
            m_ptr = NULL;
            }
        }

    void        CopyPtr (T const* ptr)
        {
        if (ptr != NULL)
            m_ptr = new T (*ptr);
        else
            m_ptr = NULL;
        }

public:
    CopiedPtr () : m_ptr(NULL) {;}
    CopiedPtr (T const* p) {CopyPtr (p);}
    ~CopiedPtr () {FreePtr();}
    CopiedPtr (CopiedPtr const& rhs) {CopyPtr(rhs.m_ptr);}
    CopiedPtr& operator=(CopiedPtr const& rhs) {FreePtr(); CopyPtr(rhs.m_ptr); return *this;}

    T& operator*()  const throw()               {return *m_ptr;}
    T* operator->() const throw()               {return m_ptr;}
    T* GetPtr()     const throw()               {return m_ptr;}
    void SetPtr (T const* p)                    {CopyPtr(p);}

}; // CopiedPtr

END_BENTLEY_DGNPLATFORM_NAMESPACE

