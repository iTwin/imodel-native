//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF message classes
//-----------------------------------------------------------------------------
// Inline methods for Message classes used in HGF.
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

///////////////////////////
// HGFGeometryChangedMsg
///////////////////////////

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGFGeometryChangedMsg::HGFGeometryChangedMsg()
    : HMGAsynchronousMessage()
    {
    }


//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HGFGeometryChangedMsg::HGFGeometryChangedMsg(const HGFGeometryChangedMsg& pi_rObj)
    : HMGAsynchronousMessage(pi_rObj)
    {
    }

END_IMAGEPP_NAMESPACE