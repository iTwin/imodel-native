//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHomeMadeLock.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HFCHomeMadeLock::HFCHomeMadeLock()
    : HFCSecurityLock()
    {
    // We should never stay with an empty representation!
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HFCHomeMadeLock::HFCHomeMadeLock(WString pi_LockRepresentation)
    : HFCSecurityLock()
    {
    HASSERT(pi_LockRepresentation.size() >= REPRESENTATION_LENGTH);

    m_Representation = pi_LockRepresentation;
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HFCSecurityLock object.
//-----------------------------------------------------------------------------
inline HFCHomeMadeLock::HFCHomeMadeLock(const HFCHomeMadeLock& pi_rObj)
    : HFCSecurityLock(pi_rObj)
    {
    m_Representation = pi_rObj.m_Representation;
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another lock object.
//-----------------------------------------------------------------------------
inline HFCHomeMadeLock& HFCHomeMadeLock::operator=(const HFCHomeMadeLock& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCSecurityLock::operator=(pi_rObj);

        m_Representation = pi_rObj.m_Representation;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// Return the lock's representation
//-----------------------------------------------------------------------------
inline WString HFCHomeMadeLock::GetRepresentation() const
    {
    return m_Representation;
    }


















