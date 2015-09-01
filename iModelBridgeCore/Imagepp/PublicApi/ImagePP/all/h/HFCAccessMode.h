//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCAccessMode.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Type : HFCAccessMode
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
/**

This extremely simple class defines objects representing access modes.
These access modes could be applied to a file, a socket, or any stream
of data: they describe how a given resource may be accessed or created.

This class is simplemented as a structure of boolean data members that
can be set and read directly (no method is defined for this).   A few
methods are provided to help comparison of access modes.

NOTE: This class has no constructor and no destructor.

 */

struct HNOVTABLEINIT HFCAccessMode
    {
    bool m_HasReadAccess;
    bool m_HasWriteAccess;
    bool m_HasCreateAccess;
    bool m_HasReadShare;
    bool m_HasWriteShare;
    bool m_OpenAlways;

    bool IsIncluded(HFCAccessMode pi_AccessMode) const;
    bool operator==(const HFCAccessMode& pi_rObj) const;
    bool operator!=(const HFCAccessMode& pi_rObj) const;
    HFCAccessMode operator|(const HFCAccessMode& pi_rObj) const;
    HFCAccessMode& operator|=(const HFCAccessMode& pi_rObj);
    };

const HFCAccessMode HFC_NO_ACCESS            = {false, false, false, false, false, false};
const HFCAccessMode HFC_SHARE_READ_ONLY      = {false, false, false, true,  false, false};
const HFCAccessMode HFC_SHARE_WRITE_ONLY     = {false, false, false, false, true,  false};
const HFCAccessMode HFC_SHARE_READ_WRITE     = {false, false, false, true,  true,  false};

const HFCAccessMode HFC_READ_ONLY            = {true,  false, false, false, false, false};
const HFCAccessMode HFC_WRITE_ONLY           = {false, true,  false, false, false, false};
const HFCAccessMode HFC_CREATE_ONLY          = {false, false, true,  false, false, false};
const HFCAccessMode HFC_OPEN_ALWAYS          = {false, false, false, false, false, true};
const HFCAccessMode HFC_READ_WRITE           = {true,  true,  false, false, false, false};
const HFCAccessMode HFC_READ_WRITE_OPEN      = {true,  true,  false, false, false, false};
const HFCAccessMode HFC_READ_WRITE_CREATE    = {true,  true,  true,  false, false, false};
const HFCAccessMode HFC_WRITE_AND_CREATE     = {false, true,  true,  false, false, false};
const HFCAccessMode HFC_READ_CREATE          = {true,  false, true,  false, false, false};

END_IMAGEPP_NAMESPACE

#include "HFCAccessMode.hpp"

