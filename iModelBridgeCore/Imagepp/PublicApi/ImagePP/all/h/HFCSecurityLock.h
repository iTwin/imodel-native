//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSecurityLock.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCSecurityLock
//-----------------------------------------------------------------------------

#pragma once


class HFCSecurityKey;

/**

    This class encapsulates the concept of a security lock. The security
    lock is used to lock a system, module, functionality or whatever, the
    application wishes to lock from usage. A security lock can be unlocked
    by a fitting key implemented by a HFCSecurityKey. To create keys or
    locks, a class HFCSecurityLockSmith may be necessary.

    All three classes form a set enabling implementation of a security
    system. All three of the above mentioned classes are abstract. There is
    no default security system. A triplet of classes descending from these
    classes must be implemented to create a security system. There will
    exist a tight dependency between each of the three classes. The key can
    unlock a lock; the lock can be unlocked by a key. The locksmith can
    create a lock from a key, or a key from a lock, and possibly create both
    fitting key and lock that go together.

    To have an efficient security system, it is necessary that the classes
    be implemented with non-inline function, and that the locksmith not be
    included in the same library as the other. The locksmith is usually
    required for creation of keys and/or locks.

    @see HFCSecurityKey
    @see HFCSecurityLockSmith

*/

class HFCSecurityLock
    {

public:

    HDECLARE_BASECLASS_ID(1613);

    //:> Primary methods
    HFCSecurityLock();
    HFCSecurityLock(const HFCSecurityLock& pi_rObject);
    virtual             ~HFCSecurityLock();

    HFCSecurityLock&    operator=(const HFCSecurityLock& pi_rObj);

    //:> Status
    bool               IsLocked() const;

    //:> Operation
    virtual bool       UnlockWith(const HFCSecurityKey& pi_rKey) = 0;
    virtual bool       LockWith(const HFCSecurityKey& pi_rKey) = 0;

protected:

    bool               m_Locked;

    };

#include "HFCSecurityLock.hpp"

