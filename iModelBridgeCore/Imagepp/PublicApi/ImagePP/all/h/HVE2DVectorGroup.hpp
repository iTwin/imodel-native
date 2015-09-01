//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DVectorGroup.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DVectorGroup::HVE2DVectorGroup()
    : HVE2DVector(),
      m_ExtentUpToDate(false)

    {
    }

//-----------------------------------------------------------------------------
// Constructor with coordinate syste,
//-----------------------------------------------------------------------------
inline HVE2DVectorGroup::HVE2DVectorGroup(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DVector(pi_rpCoordSys),
      m_ExtentUpToDate(false)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DVectorGroup::~HVE2DVectorGroup()
    {
    MakeEmpty();
    }

//-----------------------------------------------------------------------------
// GetMainVectorType
// Returns the vector type
//-----------------------------------------------------------------------------
inline HVE2DVectorTypeId HVE2DVectorGroup::GetMainVectorType() const
    {
    return (HVE2DVectorGroup::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// IsNull
// Indicates if the group is null (empty)
//-----------------------------------------------------------------------------
inline bool HVE2DVectorGroup::IsNull() const
    {
    return (m_VectorList.empty());
    }


//-----------------------------------------------------------------------------
// Clone
// Returns a new occurent of group
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DVectorGroup::Clone() const
    {
    return(new HVE2DVectorGroup(*this));
    }

END_IMAGEPP_NAMESPACE
