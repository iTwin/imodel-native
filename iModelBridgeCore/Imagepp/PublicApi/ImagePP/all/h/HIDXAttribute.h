//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXAttribute.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HIDXAttribute
//-----------------------------------------------------------------------------
// General class for indexable attributes.
//
// NOVTABLE used: NEVER instantiate directly.
//-----------------------------------------------------------------------------

#pragma once



class HNOVTABLEINIT HIDXAttribute
    {
public:

    HIDXAttribute() {};

    virtual         ~HIDXAttribute() {};


private:

    // Disabled.
    HIDXAttribute(const HIDXAttribute& pi_rObj);
    HIDXAttribute&  operator=(const HIDXAttribute& pi_rObj);

    };


