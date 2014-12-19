//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHomeMadeLockSmith.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HFCHomeMadeLockSmith::HFCHomeMadeLockSmith()
    : HFCSecurityLockSmith()
    {
    // We should never stay empty!
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HFCHomeMadeLockSmith::HFCHomeMadeLockSmith(WString pi_Seed)
    : HFCSecurityLockSmith()
    {
    m_Seed = pi_Seed;

    CalculateLockRepresentation();
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HFCHomeMadeLockSmith object.
//-----------------------------------------------------------------------------
inline HFCHomeMadeLockSmith::HFCHomeMadeLockSmith(const HFCHomeMadeLockSmith& pi_rObj)
    : HFCSecurityLockSmith(pi_rObj)
    {
    m_Seed = pi_rObj.m_Seed;
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  .
//-----------------------------------------------------------------------------
inline HFCHomeMadeLockSmith& HFCHomeMadeLockSmith::operator=(const HFCHomeMadeLockSmith& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCSecurityLockSmith::operator=(pi_rObj);

        m_Seed = pi_rObj.m_Seed;
        }

    // Return reference to self
    return (*this);
    }




















