//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DReferenceToVector.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HVE2DReferenceToVector
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// GetSource
// Return reference source vector object.
//-----------------------------------------------------------------------------
inline const HVE2DVector* HVE2DReferenceToVector::GetSource() const
    {
    HPRECONDITION(m_pSource != 0);

    return(m_pSource);
    }


//-----------------------------------------------------------------------------
// Clone
// Return a new copy of self
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DReferenceToVector::Clone () const
    {
    return new HVE2DReferenceToVector(*this);
    }


