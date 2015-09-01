//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXIndexable.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HIDXIndexable
//-----------------------------------------------------------------------------
// An object that can be put in an index.
//
// NEVER add a virtual method : NOVTABLE used.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

#include "HIDXAttribute.h"
BEGIN_IMAGEPP_NAMESPACE

////////////////////////
// HIDXIndexable
////////////////////////

template <class T> class HNOVTABLEINIT HIDXIndexable : public HFCShareableObject<HIDXIndexable<T> >
    {
public:

    explicit        HIDXIndexable(const T pi_Object);
    ~HIDXIndexable();

    // Give new attribute to the indexable (possessed by indexable)
    void            AddAttribute(void const* pi_pIndex, HIDXAttribute* pi_pNewAttribute);

    // Retrieve an attribute
    HIDXAttribute*  GetAttribute(void const* pi_pIndex) const;
    void            RemoveAttribute(void const* pi_pIndex);

    bool           IndexesSameObjectAs(const HIDXIndexable<T>& pi_rOtherIndexable) const;

    // Retrieve the object
    const T         GetObject() const;

    // Define a type for a list of indexables.
    typedef list < HFCPtr< HIDXIndexable<T> >, allocator< HFCPtr< HIDXIndexable<T> > > > List;

private:

    // Copy ctor and assignment are disabled
    HIDXIndexable(const HIDXIndexable<T>& pi_rObj);
    HIDXIndexable<T>& operator=(const HIDXIndexable<T>& pi_rObj);

    typedef map < void const*, HIDXAttribute*, less<void const*>, allocator<HIDXAttribute*> >
    AttributeMap;

    // The list of attributes for the indexable object
    AttributeMap    m_Attributes;

    T               m_Object;
    };

END_IMAGEPP_NAMESPACE

#include "HIDXIndexable.hpp"


