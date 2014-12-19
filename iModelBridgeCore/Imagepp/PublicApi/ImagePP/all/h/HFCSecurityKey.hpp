//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSecurityKey.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCSecurityKey
//-----------------------------------------------------------------------------


#include "HFCSecurityLock.h"

/**----------------------------------------------------------------------------
 Default Constructor
-----------------------------------------------------------------------------*/
inline HFCSecurityKey::HFCSecurityKey()
    {
    }


/**----------------------------------------------------------------------------
 Copy constructor.  It duplicates another HFCSecurityKey object.

 @param pi_rObj A constant reference to a key from which to create a duplicate.
-----------------------------------------------------------------------------*/
inline HFCSecurityKey::HFCSecurityKey(const HFCSecurityKey& pi_rObj)
    {
    }

/**----------------------------------------------------------------------------
 The destructor.
-----------------------------------------------------------------------------*/
inline HFCSecurityKey::~HFCSecurityKey()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator.  Self becomes identical to the provided one.

 @param pi_rObj A constant reference to a key from which to create a duplicate.
-----------------------------------------------------------------------------*/
inline HFCSecurityKey& HFCSecurityKey::operator=(const HFCSecurityKey& pi_rObj)
    {
    // Return reference to self
    return (*this);
    }


/**----------------------------------------------------------------------------
 Attempts to use this key on a lock to open it.

 @param pi_rLock Constant reference to lock to unlock.

 @return Returns true if the key unlocks the HFCSecurityLock, or
         false otherwise.
-----------------------------------------------------------------------------*/
inline bool HFCSecurityKey::Unlocks(HFCSecurityLock& pi_rLock) const
    {
    return(pi_rLock.UnlockWith(*this));
    }

















