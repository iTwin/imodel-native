//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCExclusiveKey.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCExclusiveKey
//-----------------------------------------------------------------------------
#pragma once

// Define the default HFCMonitor that works on HFCExclusiveKeys
#include <ImagePP/all/h/HFCMonitor.h>

BEGIN_IMAGEPP_NAMESPACE
/**

    This class is used to create exclusive keys, which are objects that can
    have an ownership relation to only one thread at a time.  These objects
    are used for synchronization purposes in multithreaded environments;
    they are used to protect some resources against conflicting accesses
    from different threads.  This object does not however work for
    multi-process situations.

    Its use is simple: an exclusive key is created for a particular shared
    resource.  No physical link is established between the resource and the
    key; the relation is a "logical" one that remains in the mind of the
    programmer.  A thread that takes ownership of the key (by calling the
    method Claim) is telling to other threads that the related resource is
    now protected.  Any thread that would like to use that resource must
    first take ownership of the key.  If a thread already owns the key, the
    execution of another thread that claims the key is paused until the key
    is released (until the other thread calls the method ReleaseKey).  This
    way, the resource is managed for mutual exclusive access.

    The exclusive key is reentrant by the thread that has successfully
    claimed the key.

    @h3 Notes

    System resources required by instances of this class are allocated
    at first use, which is not the same moment than at construction time.
    The first time the key is claimed, the internal handle is created.

    @b[Important}: To have a better handling of the exclusive keys in exception
    prone code, it is strongly recommended to use the HFCMonitor class one
    the key.  This object makes sure that exclusive keys are released, even
    if an exception occurs.  This avoids having to catch every exception
    that may happen just to release manually the key.

*/

class HFCExclusiveKey
    {
public:

    //:> Primary methods

    HFCExclusiveKey();
    virtual     ~HFCExclusiveKey();

    //:> Key operation

    void        ClaimKey();
    void        ReleaseKey();


private:

    // Disabled methods

    HFCExclusiveKey(const HFCExclusiveKey&);
    HFCExclusiveKey& operator=(const HFCExclusiveKey&);


    BeMutex m_Key;
    };

typedef HFCGenericMonitor<HFCExclusiveKey> HFCMonitor;

END_IMAGEPP_NAMESPACE

#include "HFCExclusiveKey.hpp"
