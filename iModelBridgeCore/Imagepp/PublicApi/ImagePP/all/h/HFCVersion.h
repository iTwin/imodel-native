//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCVersion
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
class HFCVersion
    {
public:
    //--------------------------------------
    // Basic methods
    //--------------------------------------

    // Construction/Destruction
    HFCVersion();
    HFCVersion(const Utf8String&   pi_rName);
    HFCVersion(const Utf8String&   pi_rName,
               const Utf8String&   pi_rInfo,
               size_t           pi_Count,
               ...);
    HFCVersion(const HFCVersion& pi_rObj);
    virtual         ~HFCVersion();

    // Copy operator
    HFCVersion&     operator= (const HFCVersion& pi_rObj);


    //--------------------------------------
    // Comparison
    //--------------------------------------

    bool           operator==(const HFCVersion& pi_rObj) const;
    bool           operator!=(const HFCVersion& pi_rObj) const;
    bool           operator< (const HFCVersion& pi_rObj) const;
    bool           operator<=(const HFCVersion& pi_rObj) const;
    bool           operator> (const HFCVersion& pi_rObj) const;
    bool           operator>=(const HFCVersion& pi_rObj) const;


    //--------------------------------------
    // Query
    //--------------------------------------

    const Utf8String&  GetName() const;
    const Utf8String&  GetInfo() const;
    size_t          GetNumberCount() const;
    uint32_t        GetNumber(size_t pi_Index) const;


protected:
    //--------------------------------------
    // Members
    //--------------------------------------

    Utf8String     m_Name;
    Utf8String     m_Info;
    size_t      m_NumberCount;
    HArrayAutoPtr<uint32_t>
    m_pNumbers;

    };

END_IMAGEPP_NAMESPACE
#include "HFCVersion.hpp"

