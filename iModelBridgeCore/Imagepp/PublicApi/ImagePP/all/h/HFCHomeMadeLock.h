//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHomeMadeLock.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCHomeMadeLock
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once


#include "HFCSecurityLock.h"

class HFCHomeMadeKey;
class HFCHomeMadeLockSmith;

class HFCHomeMadeLock : public HFCSecurityLock
    {
    friend class HFCHomeMadeLockSmith;

public:

    HDECLARE_BASECLASS_ID(1616);

    // Primary methods
    HFCHomeMadeLock();
    HFCHomeMadeLock(WString pi_LockRepresentation);
    HFCHomeMadeLock(const HFCHomeMadeLock&   pi_rObject);
    virtual             ~HFCHomeMadeLock();

    HFCHomeMadeLock&    operator=(const HFCHomeMadeLock& pi_rObj);

    // Operation
    virtual bool       UnlockWith(const HFCSecurityKey& pi_rKey);
    virtual bool       LockWith(const HFCSecurityKey& pi_rKey);

    WString             GetRepresentation() const;

protected:

    WString             GetKeyRepresentation() const;

private:

    bool               CanBeUnlockedBy(HFCHomeMadeKey& pi_rKey) const;

    // Lock
    WString     m_Representation;

    enum {
        REPRESENTATION_LENGTH = 14
        };
    };


#include "HFCHomeMadeLock.hpp"

