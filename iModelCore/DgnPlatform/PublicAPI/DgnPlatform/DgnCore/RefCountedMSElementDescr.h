/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/RefCountedMSElementDescr.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include "MSElementDescr.h"

//=======================================================================================
//!
//! A "smart pointer" to an MSElementDescr. Works just like RefCountedPointer
//!   (aka Boost intrusive_ptr)
//!
//! @bsiclass                                                     Sam.Wilson      01/2005
//=======================================================================================
class  RefCountedElemDescrP
{
private:
    MSElementDescrP m_element;

    void Release () {RELEASE_AND_CLEAR (m_element);}
    void AddRef (MSElementDescrP e)
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
    RefCountedElemDescrP (MSElementDescrP e, bool addInitialRef = true)
        {
        if (addInitialRef)
            AddRef (e);
        else
            m_element = e;
        }
    ~RefCountedElemDescrP () {Release ();}
    MSElementDescrP get() const {return m_element;}
    MSElementDescrP operator->() const {return m_element;}
    MSElementDescrR operator*() const {return *m_element;}
};
