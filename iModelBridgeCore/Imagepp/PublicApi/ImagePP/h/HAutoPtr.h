//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HAutoPtr.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HAutoPtr.
//
// Template HAutoPtr holds onto a pointer obtained via new and deletes that
// object when it itself is destroyed (such as when leaving block scope).
//
// It can be used to make calls to new() exception safe.
//*****************************************************************************
#pragma once

#include "HTypes.h"

BEGIN_IMAGEPP_NAMESPACE

template <class P>
class HNOVTABLEINIT HAutoPtr
    {
public:
    // Constructor and destructor.
#ifdef __HMRCBUG_EXPLICITPREFIX_NOT_SUP
    HAutoPtr(P* pi_Ptr = 0);
#else
    explicit     HAutoPtr(P* pi_Ptr = 0);
#endif
    HAutoPtr(HAutoPtr<P>& pi_rObj);
    ~HAutoPtr();

    // Operators.
    HAutoPtr<P>& operator=(HAutoPtr<P>& pi_rObj);
    HAutoPtr<P>& operator=(P* pi_Ptr);
    P&           operator*() const;
    P*           operator->() const;
    bool        operator==(const P* pi_pObj) const;
    bool        operator==(const HAutoPtr<P>& pi_rObj);
    bool        operator!=(const P* pi_pObj) const;
    bool        operator!=(const HAutoPtr<P>& pi_rObj);
    operator P* () const;

    // Pointer manipulation methods.
    P*           get() const;
    P*           release();
    P*           reset(P* pi_Ptr = 0);

private:
    // Member.
    P* m_Ptr;
    };

END_IMAGEPP_NAMESPACE
#include "HAutoPtr.hpp"
