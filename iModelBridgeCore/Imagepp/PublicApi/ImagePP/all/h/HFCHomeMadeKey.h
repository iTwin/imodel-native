//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHomeMadeKey.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCHomeMadeKey
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HFCSecurityKey.h"


class HFCHomeMadeKey : public HFCSecurityKey
    {
    friend class HFCHomeMadeLockSmith;
    friend class HFCHomeMadeLock;

public:

    HDECLARE_BASECLASS_ID(1615);

    // Primary methods
    HFCHomeMadeKey();
    HFCHomeMadeKey(WString pi_KeyRepresentation);
    HFCHomeMadeKey(const HFCHomeMadeKey& pi_rObject);
    virtual         ~HFCHomeMadeKey();

    HFCHomeMadeKey& operator=(const HFCHomeMadeKey& pi_rObj);

    bool           operator==(const HFCHomeMadeKey& pi_rObj) const;

    WString         GetRepresentation() const;


private:

    // The key.
    WString          m_Representation;
    };


#include "HFCHomeMadeKey.hpp"

