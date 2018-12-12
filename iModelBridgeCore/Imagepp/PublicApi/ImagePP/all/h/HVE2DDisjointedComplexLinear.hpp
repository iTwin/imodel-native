//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DDisjointedComplexLinear.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default constructor for a complex linear. This constructor creates an empty
    complex linear that does not contain any linear components. The interpretation
    coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
inline HVE2DDisjointedComplexLinear::HVE2DDisjointedComplexLinear()
    : HVE2DComplexLinear()
    {
    }

/** -----------------------------------------------------------------------------
    Default constructor for a complex linear. This constructor creates an empty
    complex linear that does not contain any linear components. The interpretation
    coordinate system is the one provided.

    @param pi_rpCoordSys Reference to smart pointer to interpretation coordinate system.
                         This pointer may not be null.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HVE2DComplexLinear        MyComplex1(pMyWorld);
    @end

    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline HVE2DDisjointedComplexLinear::HVE2DDisjointedComplexLinear(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DComplexLinear(pi_rpCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DDisjointedComplexLinear::~HVE2DDisjointedComplexLinear()
    {
    }

//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the complex linear
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DDisjointedComplexLinear::Clone() const
    {
    return (new HVE2DDisjointedComplexLinear(*this));
    }

END_IMAGEPP_NAMESPACE
