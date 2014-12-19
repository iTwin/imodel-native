//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCSecurityLockSmith.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inlines methods for class HFCSecurityLockSmith
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Default constructor that creates a locksmith.
-----------------------------------------------------------------------------*/
inline HFCSecurityLockSmith::HFCSecurityLockSmith()
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor.  It duplicates another HFCSecurityLockSmith object.

 @param pi_rObj Constant reference to another locksmith that will "train"
                the new one to make it create the same kind of locks and keys.
-----------------------------------------------------------------------------*/
inline HFCSecurityLockSmith::HFCSecurityLockSmith(const HFCSecurityLockSmith& pi_rObj)
    {
    }

/**----------------------------------------------------------------------------
 The destructor.
-----------------------------------------------------------------------------*/
inline HFCSecurityLockSmith::~HFCSecurityLockSmith()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator.  It duplicates another HFCSecurityLockSmith object.

 @param pi_rObj Constant reference to another locksmith that will "train"
                the new one to make it create the same kind of locks and keys.
-----------------------------------------------------------------------------*/
inline HFCSecurityLockSmith& HFCSecurityLockSmith::operator=(const HFCSecurityLockSmith& pi_rObj)
    {
    // Return reference to self
    return (*this);
    }


/**----------------------------------------------------------------------------
 Creates a key permitting to unlock the provided lock. If the key could
 not be created then 0 is returned.

 @param pi_rLock Constant reference to lock to obtain key for.

 @return Returns a dynamically allocated key. This key must be destroyed
         when no more useful.
-----------------------------------------------------------------------------*/
//HFCSecurityKey* HFCSecurityLockSmith::CreateKey(const HFCSecurityLock& pi_rLock) const;
//:> Pure virtual method.

/**----------------------------------------------------------------------------
 Creates a lock that can be unloced the provided key.

 @param pi_rKey Constant reference to key to obtain lock for.

 @return Returns a dynamically allocated lock. This lock must be destroyed when
         no more useful. If the lock could not be created then 0 is returned.
-----------------------------------------------------------------------------*/
//HFCSecurityLock* HFCSecurityLockSmith::CreateLock(const HFCSecurityKey& pi_rKey) const;
//:> Pure virtual method.

/**----------------------------------------------------------------------------
 Creates a lock / key pair that fit with each other.

 @param po_ppKey    A pointer to a pointer that will be set to a newly allocated
                    key. The key must be destroyed when no more useful.
 @param po_ppLock   A pointer to a pointer that will be set to a newly allocated
                    lock. The lock must be deleted when no more useful.

 @return Returns true if key and lock could be created.
-----------------------------------------------------------------------------*/
//bool HFCSecurityLockSmith::CreateKeyAndLock(HFCSecurityKey** po_ppKey, HFCSecurityLock** po_ppLock) const;
//:> Pure virtual method.
