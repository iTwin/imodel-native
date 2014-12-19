//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSecurityLock.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HFCSecurityLock
//-----------------------------------------------------------------------------


/**-----------------------------------------------------------------------------
 Default constructor that creates a locked lock.
-----------------------------------------------------------------------------*/
inline HFCSecurityLock::HFCSecurityLock()
    : m_Locked(true)
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor.  It duplicates another HFCSecurityLock object, meaning
 that a lock is duplicated wiuth the same lock status and the same key
 needed to unlock it.

 @param pi_rObj  Constant reference to another lock to duplicate.
-----------------------------------------------------------------------------*/
inline HFCSecurityLock::HFCSecurityLock(const HFCSecurityLock& pi_rObj)
    : m_Locked(pi_rObj.m_Locked)
    {
    }

/**----------------------------------------------------------------------------
 The destructor.
-----------------------------------------------------------------------------*/
inline HFCSecurityLock::~HFCSecurityLock()
    {
    }

/**----------------------------------------------------------------------------
 Assignment operator.  It duplicates another HFCSecurityLock object, meaning
 that a lock is duplicated wiuth the same lock status and the same key
 needed to unlock it.

 @param pi_rObj  Constant reference to another lock to duplicate.

 @return A reference to self.
-----------------------------------------------------------------------------*/
inline HFCSecurityLock& HFCSecurityLock::operator=(const HFCSecurityLock& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Locked = pi_rObj.m_Locked;
        }

    // Return reference to self
    return (*this);
    }

/**----------------------------------------------------------------------------
 Indicates if this lock is currently locked.

 @return Returns true if the lock is currently LOCKED, and false otherwise.
-----------------------------------------------------------------------------*/
inline bool HFCSecurityLock::IsLocked() const
    {
    return(m_Locked);
    }

/**----------------------------------------------------------------------------
 Unlocks self using the provided key.

 @param pi_rKey Constant reference to key to use to unlock.

 @return true if the lock is unlocked by the provided HFCSecurityKey instance,
         or false otherwise.
-----------------------------------------------------------------------------*/
//bool HFCSecurityLock::UnlockWith(const HFCSecurityKey& pi_rKey);
//:> Pure virtual method.

/**----------------------------------------------------------------------------
 Lock self using the provided key. The lock must be currently unlocked to be used.

 @param pi_rKey Constant reference to key to use to lock.

 @return true if the lock was successfully locked by the provided HFCSecurityKey
         instance, or false otherwise.
-----------------------------------------------------------------------------*/
//bool HFCSecurityLock::LockWith(const HFCSecurityKey& pi_rKey);
//:> Pure virtual method.



















