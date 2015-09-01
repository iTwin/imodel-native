//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFMessages.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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