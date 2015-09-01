//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DUniverse.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DUniverse
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DUniverse.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>

#include <Imagepp/all/h/HGF2DSegment.h>
#include <Imagepp/all/h/HGF2DPolySegment.h>
#include <Imagepp/all/h/HGF2DHoledShape.h>


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HGF2DUniverse::HGF2DUniverse()
    : HGF2DSimpleShape()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DUniverse object.
//-----------------------------------------------------------------------------
HGF2DUniverse::HGF2DUniverse(const HGF2DUniverse& pi_rObj)
    : HGF2DSimpleShape(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HGF2DUniverse::~HGF2DUniverse()
    {
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HFCPtr<HGF2DLinear> HGF2DUniverse::GetLinear() const
    {
    // Allocate linear
    HGF2DPolySegment*    pMyLinear = new HGF2DPolySegment();

    // Add required parts
    pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (-DBL_MAX)));
    pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (DBL_MAX)));
    pMyLinear->AppendPoint(HGF2DPosition((DBL_MAX), (DBL_MAX)));
    pMyLinear->AppendPoint(HGF2DPosition((DBL_MAX), (-DBL_MAX)));
    pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (-DBL_MAX)));

    return pMyLinear;
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HFCPtr<HGF2DLinear> HGF2DUniverse::GetLinear(HGF2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // Allocate linear
    HGF2DPolySegment*    pMyLinear = new HGF2DPolySegment();

    if (pi_DirectionDesired == HGF2DSimpleShape::CW)
        {
        // Create CW linear
        pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (-DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((DBL_MAX), (DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((DBL_MAX), (-DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (-DBL_MAX)));
        }
    else
        {
        pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (-DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((DBL_MAX), (-DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((DBL_MAX), (DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (DBL_MAX)));
        pMyLinear->AppendPoint(HGF2DPosition((-DBL_MAX), (-DBL_MAX)));
        }

    return pMyLinear;
    }



//-----------------------------------------------------------------------------
// DifferentiateShape
// This method create a new shape as the difference between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HGF2DShape* HGF2DUniverse::DifferentiateShape(const HGF2DShape& pi_rShape) const
    {
    HGF2DShape* pResultShape;
    // Check if the given shape is also universe
    if (pi_rShape.GetShapeType() == HGF2DUniverse::CLASS_ID)
        {
        // Both shapes are universe ... nothing left
        pResultShape = new HGF2DVoidShape();
        }
    else
        {
        // Check if it is a simple shape
        if (pi_rShape.IsSimple())
            {
            // The result is a holed shape
            HGF2DHoledShape* pResultHoled = new HGF2DHoledShape(*this);

            pResultHoled->AddHole((*(HGF2DSimpleShape*)(&pi_rShape)));

            pResultShape = pResultHoled;
            }
        else
            {
            // Tell other shape to perform the process
            pResultShape = pi_rShape.DifferentiateFromShape(*this);
            }
        }

    return(pResultShape);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DUniverse::CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const
    {
    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_IN;

    if (pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID &&
        ((HGF2DShape*)(&pi_rVector))->GetShapeType() == HGF2DUniverse::CLASS_ID)
        {
        ThePosition = HGF2DShape::S_ON;
        }

    return (ThePosition);
    }


//-----------------------------------------------------------------------------
// @bsimethod                                                   2014/06
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HGF2DUniverse::AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const
    {
    return (new HGF2DUniverse(*this));
    }

//-----------------------------------------------------------------------------
// @bsimethod                                                   2014/06
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HGF2DUniverse::AllocTransformInverse(const HGF2DTransfoModel& pi_rModel) const
    {
    return (new HGF2DUniverse(*this));
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DUniverse::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HGF2DSimpleShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HGF2DUniverse" << endl;
    HDUMP0("Object is a HGF2DUniverse\n");

#endif
    }
