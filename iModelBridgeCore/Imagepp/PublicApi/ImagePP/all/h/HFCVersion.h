//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCVersion.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    HFCVersion(const WString&   pi_rName);
    HFCVersion(const WString&   pi_rName,
               const WString&   pi_rInfo,
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

    const WString&  GetName() const;
    const WString&  GetInfo() const;
    size_t          GetNumberCount() const;
    uint32_t        GetNumber(size_t pi_Index) const;


protected:
    //--------------------------------------
    // Members
    //--------------------------------------

    WString     m_Name;
    WString     m_Info;
    size_t      m_NumberCount;
    HArrayAutoPtr<uint32_t>
    m_pNumbers;

    };

END_IMAGEPP_NAMESPACE
#include "HFCVersion.hpp"

