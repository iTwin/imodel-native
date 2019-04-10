//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCExclusiveKey.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCExclusiveKey
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

/**----------------------------------------------------------------------------
 Constructor for this class. It creates an exclusive key: just after
 creation the key does not have any owner.

 @i{Note}: no copy constructor and no assignment operator are defined for
           this class.
-----------------------------------------------------------------------------*/
inline HFCExclusiveKey::HFCExclusiveKey()
    {
    }

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
inline HFCExclusiveKey::~HFCExclusiveKey()
    {
    }

/**----------------------------------------------------------------------------
 Gives the ownership of the key to current thread.  If the key is already
 owned, the current thread is suspended until its owner releases the
 key.

 If the current thread already owns the key, this method simply
 returns.

 @see HFCExclusiveKey::ReleaseKey
-----------------------------------------------------------------------------*/
inline void HFCExclusiveKey::ClaimKey()
    {
    m_Key.Enter();
    }

/**----------------------------------------------------------------------------
 Releases ownership for this key.  After this, the key is available for
 ownership by any thread.  One of the threads, if any, waiting to gain
 ownership of the key will get it.

 @see HFCExclusiveKey::Claim
-----------------------------------------------------------------------------*/
inline void HFCExclusiveKey::ReleaseKey()
    {
    m_Key.Leave();
    }

END_IMAGEPP_NAMESPACE