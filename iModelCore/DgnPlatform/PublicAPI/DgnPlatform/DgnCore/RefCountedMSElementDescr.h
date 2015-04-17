/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/RefCountedMSElementDescr.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "DgnElementDescr.h"

//=======================================================================================
//!
//! A "smart pointer" to an DgnElementDescr. Works just like RefCountedPointer
//!   (aka Boost intrusive_ptr)
//!
//! @bsiclass                                                     Sam.Wilson      01/2005
//=======================================================================================
class  RefCountedElemDescrP
{
private:
    DgnElementDescrP m_element;

    void Release () {RELEASE_AND_CLEAR (m_element);}
    void AddRef (DgnElementDescrP e)
        {
        m_element = e;
        if (NULL != m_element)
            m_element->AddRef ();
        }

public:
    RefCountedElemDescrP () : m_element (NULL) {}
    RefCountedElemDescrP& operator= (RefCountedElemDescrP const& cc)
        {
        Release ();
        AddRef (cc.m_element);
        return *this;
        }
    RefCountedElemDescrP (RefCountedElemDescrP const& cc) {AddRef (cc.m_element);}
    RefCountedElemDescrP (DgnElementDescrP e, bool addInitialRef = true)
        {
        if (addInitialRef)
            AddRef (e);
        else
            m_element = e;
        }
    ~RefCountedElemDescrP () {Release ();}
    DgnElementDescrP get() const {return m_element;}
    DgnElementDescrP operator->() const {return m_element;}
    DgnElementDescrR operator*() const {return *m_element;}
};
