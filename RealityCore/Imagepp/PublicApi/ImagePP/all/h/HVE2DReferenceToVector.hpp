//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HVE2DReferenceToVector
//-----------------------------------------------------------------------------



BEGIN_IMAGEPP_NAMESPACE
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


END_IMAGEPP_NAMESPACE
