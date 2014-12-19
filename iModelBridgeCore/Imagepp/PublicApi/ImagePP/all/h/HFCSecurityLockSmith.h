//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSecurityLockSmith.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCSecurityLockSmith
//-----------------------------------------------------------------------------

#pragma once


class HFCSecurityKey;
class HFCSecurityLock;

/**

    This class encapsulates the concept of a security locksmith. The
    security locksmith is used to create keys from lock, lock from keys, or
    key-lock pairs. The class is part of a system of security lock
    implemented also in the classes HFCSecurityKey and HFCSecurityLock.

    All three classes form a set enabling implementation of a security
    system. All three of the above mentionned classes are abstract. There is
    no default security system. A triplet of classes descending from these
    classes must be implemented to create a security system. There will
    exist a tight dependency between each of the three classes. The key can
    unlock a lock, the Lock can be unlocked by a key. The locksmith can
    create a lock from a key, or a key from a lock, and possibly create both
    fitting key and lock, which go together.

    To have an efficient security system, it is necessary that the classes
    be implemented with non-inlined function, and that the locksmith not be
    included in the same library as the other. The locksmith is usually
    required for creation of keys and/or locks.

    @see HFCSecurityLock
    @see HFCSecurityKey

*/

class HFCSecurityLockSmith
    {

public:

    HDECLARE_BASECLASS_ID(1614);

    //:> Primary methods
    HFCSecurityLockSmith();
    HFCSecurityLockSmith(const HFCSecurityLockSmith&   pi_rObject);
    virtual                 ~HFCSecurityLockSmith();

    HFCSecurityLockSmith&   operator=(const HFCSecurityLockSmith& pi_rObj);

    //:> Creation operation
    virtual HFCSecurityKey*  CreateKey(const HFCSecurityLock& pi_rLock) const = 0;
    virtual HFCSecurityLock* CreateLock(const HFCSecurityKey& pi_rKey) const = 0;
    virtual bool            CreateKeyAndLock(HFCSecurityKey** po_ppKey,
                                              HFCSecurityLock** po_ppLock) const = 0;

protected:

private:

    };


#include "HFCSecurityLockSmith.hpp"
