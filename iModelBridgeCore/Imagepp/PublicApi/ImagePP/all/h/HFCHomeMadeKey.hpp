//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCHomeMadeKey.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HFCHomeMadeKey::HFCHomeMadeKey()
    : HFCSecurityKey()
    {
    // We should never stay with an empty representation!
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HFCHomeMadeKey::HFCHomeMadeKey(WString pi_KeyRepresentation)
    : HFCSecurityKey()
    {
    m_Representation = pi_KeyRepresentation;
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HFCHomeMadeKey object.
//-----------------------------------------------------------------------------
inline HFCHomeMadeKey::HFCHomeMadeKey(const HFCHomeMadeKey& pi_rObj)
    : HFCSecurityKey(pi_rObj)
    {
    m_Representation = pi_rObj.m_Representation;
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.
//-----------------------------------------------------------------------------
inline HFCHomeMadeKey& HFCHomeMadeKey::operator=(const HFCHomeMadeKey& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HFCSecurityKey::operator=(pi_rObj);

        m_Representation = pi_rObj.m_Representation;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// Return our representation (used in the lock class to unlock it)
//-----------------------------------------------------------------------------
inline WString HFCHomeMadeKey::GetRepresentation() const
    {
    return m_Representation;
    }
