//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSecurityKey.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCSecurityKey
//-----------------------------------------------------------------------------

#pragma once

class HFCSecurityLock;

/**

    This class encapsulates the concept of a security key. The security key
    is used to unlock security lock implemented in the class
    HFCSecurityLock. To create keys or locks, a class HFCSecurityLockSmith
    may be necessary.

    All three classes form a set enabling implementation of a security
    system. All three of the above mentioned classes are abstract. There is
    no default security system. A triplet of classes descending from these
    classes must be implemented to create a security system. There is a
    tight dependency between each of the three classes. The key can unlock a
    lock; the lock can be unlocked by a key. The locksmith can create a lock
    from a key, or a key from a lock, and possibly create both fitting key
    and lock that go together.

    To have an efficient security system, it is necessary that the classes
    be implemented with non-inline function, and that the locksmith not be
    included in the same library as the other. The locksmith is usually
    required for creation of keys and/or locks.

    @see HFCSecurityLock
    @see HFCSecurityLockSmith
*/

class HFCSecurityKey
    {

public:

    HDECLARE_BASECLASS_ID(1612);

    //:> Primary methods
    HFCSecurityKey();
    HFCSecurityKey(const HFCSecurityKey& pi_rObject);
    virtual            ~HFCSecurityKey();

    HFCSecurityKey&    operator=(const HFCSecurityKey& pi_rObj);

    //:> Operation
    bool              Unlocks(HFCSecurityLock& pi_rLock) const;
    };

#include "HFCSecurityKey.hpp"

