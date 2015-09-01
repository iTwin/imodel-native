//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCBinStreamLockManager.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HFCBinStreamLockManager
//-----------------------------------------------------------------------------

#pragma once

#include "HFCBinStream.h"

// Define the default HFCMonitor that works on HFCBinStreamLockManager
#include <ImagePP/all/h/HFCMonitor.h>

BEGIN_IMAGEPP_NAMESPACE

/** ---------------------------------------------------------------------------
    This class implents a intelligent file locking algorithm. It can perform a
    virtual lock if the pi_pBinstream parameter is null. And a reference count
    enable the user to perform many lock one over the others on the same file.
    But the users must always uses the same instance of this class.
    This class can be uses with a HFCGenericMonitor too. It is helpfull to
    perform automatic unlock of the file.

    @see HFCLockMonitor
// ------------------------------------------------------------------------- */
class HFCBinStreamLockManager
    {
public:
    HFCBinStreamLockManager (HFCBinStream* pi_pBinStream,
                             uint64_t pi_Pos,
                             uint64_t pi_Size,
                             bool pi_Share);

    ~HFCBinStreamLockManager ();

    void Lock();
    void UnLock();

    //:> Those methods are used for compatibility with the HFCGenericMonitor
    //:> templated class.
    void ClaimKey();
    void ReleaseKey();

    bool IsLocked();
    Byte GetRefCount() const;

    //:> This method will set the pointer on the sister file in the method
    //:> OpenFile of the object HRFSisterFileSharing.
    void SetSisterFileStream(HFCBinStream* pi_pBinStream);


private:
    //:> Attributes
    HFCBinStream* m_pBinStream;
    uint64_t m_Position;
    uint64_t m_Size;
    bool   m_Share;
    Byte   m_RefCount;

    //:> Disabled methods
    HFCBinStreamLockManager();
    HFCBinStreamLockManager(const HFCBinStreamLockManager&);
    HFCBinStreamLockManager& operator= (const HFCBinStreamLockManager&);


    };

typedef HFCGenericMonitor<HFCBinStreamLockManager> HFCLockMonitor;

END_IMAGEPP_NAMESPACE


#include "HFCBinStreamLockManager.hpp"

