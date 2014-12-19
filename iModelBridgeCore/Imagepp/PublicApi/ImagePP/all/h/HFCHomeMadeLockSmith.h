//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHomeMadeLockSmith.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCHomeMadeLockSmith
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once


#include "HFCSecurityLockSmith.h"

class HFCHomeMadeKey;
class HFCHomeMadeLock;

class HFCHomeMadeLockSmith : public HFCSecurityLockSmith
    {

public:

    HDECLARE_BASECLASS_ID(1617);

    // Primary methods
    HFCHomeMadeLockSmith();
    HFCHomeMadeLockSmith(WString pi_Seed);
    HFCHomeMadeLockSmith(const HFCHomeMadeLockSmith&   pi_rObject);
    virtual                 ~HFCHomeMadeLockSmith();

    HFCHomeMadeLockSmith&   operator=(const HFCHomeMadeLockSmith& pi_rObj);

    // Creation operation
    virtual HFCSecurityKey*  CreateKey(const HFCSecurityLock& pi_rLock) const;
    virtual HFCSecurityLock* CreateLock(const HFCSecurityKey& pi_rKey) const;
    virtual bool            CreateKeyAndLock(HFCSecurityKey** po_ppKey,
                                              HFCSecurityLock** po_ppLock) const;

protected:

private:

    // Encode the seed into a lock rep.
    void            CalculateLockRepresentation();

    // The seed string from which we create the lock and key representations
    WString         m_Seed;

    // The lock representation for our seed.
    WString         m_LockRepresentation;

    enum {
        REPRESENTATION_LENGTH = 14
        };
    };


#include "HFCHomeMadeLockSmith.hpp"

