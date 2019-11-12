//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HVEVSRelativeAttribute
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HVEVSRelativeAttribute::HVEVSRelativeAttribute(const HFCPtr<HVEShape>& pi_rpShape)
    : m_pShape(pi_rpShape)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline HVEVSRelativeAttribute::~HVEVSRelativeAttribute()
    {
    }


//-----------------------------------------------------------------------------
// Set a new shape
//-----------------------------------------------------------------------------
inline void HVEVSRelativeAttribute::SetShape(const HFCPtr<HVEShape>& pi_rpShape)
    {
    m_pShape = pi_rpShape;
    }


//-----------------------------------------------------------------------------
// Retrieve the shape
//-----------------------------------------------------------------------------
inline HFCPtr<HVEShape>& HVEVSRelativeAttribute::GetShape() const
    {
    return m_pShape;
    }


//-----------------------------------------------------------------------------
// Clear the cached shape
//-----------------------------------------------------------------------------
inline void HVEVSRelativeAttribute::ClearShape()
    {
    m_pShape = 0;
    }
END_IMAGEPP_NAMESPACE
