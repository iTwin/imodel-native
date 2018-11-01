//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HArrayAutoPtr.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HArrayAutoPtr.
//
// Template HArrayAutoPtr holds onto a pointer obtained via new and deletes that
// object when it itself is destroyed (such as when leaving block scope).
//
// It can be used to make calls to new() exception safe.
//*****************************************************************************
#pragma once

#include "HTypes.h"

BEGIN_IMAGEPP_NAMESPACE

#define HARRAYAUTOPTR(Type, Pointer) (HArrayAutoPtr<Type> (Pointer).get())

template <class P>
class HNOVTABLEINIT HArrayAutoPtr
    {
public:
    // Constructor and destructor.
#ifdef __HMRCBUG_EXPLICITPREFIX_NOT_SUP
    HArrayAutoPtr(P* pi_Ptr = 0);
#else
    explicit          HArrayAutoPtr(P* pi_Ptr = 0);
#endif
    HArrayAutoPtr(HArrayAutoPtr<P>& pi_rObj);
    ~HArrayAutoPtr();

    // Operators.
    HArrayAutoPtr<P>& operator=(HArrayAutoPtr<P>& pi_rObj);
    HArrayAutoPtr<P>& operator=(P* pi_Ptr);
    P&                operator*() const;
    P*                operator->() const;
    bool             operator==(const P* pi_pObj) const;
    bool             operator==(const HArrayAutoPtr<P>& pi_rObj);
    bool             operator!=(const P* pi_pObj) const;
    bool             operator!=(const HArrayAutoPtr<P>& pi_rObj);
    operator P* () const;

    // Pointer manipulation methods.
    P*                get() const;
    P*                release();
    P*                reset(P* pi_Ptr = 0);

private:
    // Member.
    P* m_Ptr;
    };

//-------------------------------------------------------------------------------------------
// Class HMallocAutoPtr.
//
// Template HMallocAutoPtr holds onto a pointer obtained via new and deletes that
// object when it itself is destroyed (such as when leaving block scope).

template <class P>
class HNOVTABLEINIT HMallocAutoPtr
    {
public:
    // Constructor and destructor.
#ifdef __HMRCBUG_EXPLICITPREFIX_NOT_SUP
    HMallocAutoPtr(size_t pi_NbItems = 0);
#else
    explicit          HMallocAutoPtr(size_t pi_NbItems = 0);
#endif
    HMallocAutoPtr(HMallocAutoPtr<P>& pi_rObj);
    ~HMallocAutoPtr();

    // Allocation
    P*              AllocItems (size_t pi_NbItems);
    P*              ReallocItems (size_t pi_NewNbItems);

    // Operators.
    P&                operator*() const;
    bool             operator==(const P* pi_pObj) const;
    bool             operator==(const HMallocAutoPtr<P>& pi_rObj);
    bool             operator!=(const P* pi_pObj) const;
    bool             operator!=(const HMallocAutoPtr<P>& pi_rObj);
    operator P* () const;

    // Pointer manipulation methods.
    P*                get() const;

private:
    // Member.
    P* m_Ptr;

    // Method
    // These methods are here voluntary, to be sure to not mixed malloc/new pointer
    HMallocAutoPtr<P>& operator=(HMallocAutoPtr<P>& pi_rObj);
    HMallocAutoPtr<P>& operator=(P* pi_Ptr);

    P*                release();
    P*                reset(P* pi_Ptr = 0);
    };

END_IMAGEPP_NAMESPACE
#include "HArrayAutoPtr.hpp"
