//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DUniverse.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DUniverse
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>

HPM_REGISTER_CLASS(HVE2DUniverse, HVE2DSimpleShape)


#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HVE2DUniverse::HVE2DUniverse()
    : HVE2DSimpleShape()
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HVE2DUniverse::HVE2DUniverse(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HVE2DUniverse::HVE2DUniverse(const HGF2DUniverse& pi_rLightShape,
                             const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys)
    {
    // The given light shape does not serve a lot of purpose.
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DUniverse object.
//-----------------------------------------------------------------------------
HVE2DUniverse::HVE2DUniverse(const HVE2DUniverse& pi_rObj)
    : HVE2DSimpleShape(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HVE2DUniverse::~HVE2DUniverse()
    {
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DUniverse::GetLinear() const
    {
    // Allocate linear
    HVE2DComplexLinear    MyComplexLinear(GetCoordSys());

    // Add required parts
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation((-DBL_MAX), (-DBL_MAX), GetCoordSys()),
                                              HGF2DLocation((-DBL_MAX), DBL_MAX, GetCoordSys())));
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation((-DBL_MAX), DBL_MAX, GetCoordSys()),
                                              HGF2DLocation(DBL_MAX, DBL_MAX, GetCoordSys())));
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation(DBL_MAX, DBL_MAX, GetCoordSys()),
                                              HGF2DLocation(DBL_MAX, (-DBL_MAX), GetCoordSys())));
    MyComplexLinear.AppendLinear(HVE2DSegment(HGF2DLocation(DBL_MAX, (-DBL_MAX), GetCoordSys()),
                                              HGF2DLocation((-DBL_MAX), (-DBL_MAX), GetCoordSys())));

    return(MyComplexLinear);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DUniverse::GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // Allocate linear
    HVE2DComplexLinear    MyComplexLinear(GetCoordSys());

    if (pi_DirectionDesired == HVE2DSimpleShape::CW)
        {
        // Create CW linear
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint((-DBL_MAX), (-DBL_MAX));
        MySegment.SetRawEndPoint((-DBL_MAX), DBL_MAX);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint((-DBL_MAX), DBL_MAX);
        MySegment.SetRawEndPoint(DBL_MAX, DBL_MAX);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, DBL_MAX);
        MySegment.SetRawEndPoint(DBL_MAX, (-DBL_MAX));
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, (-DBL_MAX));
        MySegment.SetRawEndPoint((-DBL_MAX), (-DBL_MAX));
        MyComplexLinear.AppendLinear(MySegment);
        }
    else
        {
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint((-DBL_MAX), (-DBL_MAX));
        MySegment.SetRawEndPoint(DBL_MAX, (-DBL_MAX));
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, (-DBL_MAX));
        MySegment.SetRawEndPoint(DBL_MAX, DBL_MAX);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, DBL_MAX);
        MySegment.SetRawEndPoint((-DBL_MAX), DBL_MAX);
        MyComplexLinear.AppendLinear(MySegment);
        MySegment.SetRawStartPoint((-DBL_MAX), DBL_MAX);
        MySegment.SetRawEndPoint((-DBL_MAX), (-DBL_MAX));
        MyComplexLinear.AppendLinear(MySegment);
        }

    return(MyComplexLinear);
    }

//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns a linear descibing the path of the rectangle
//-----------------------------------------------------------------------------
HVE2DComplexLinear* HVE2DUniverse::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // Allocate linear
    HVE2DComplexLinear*    pMyComplexLinear = new HVE2DComplexLinear(GetCoordSys());

    if (pi_DirectionDesired == HVE2DSimpleShape::CW)
        {
        // Create CW linear
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint((-DBL_MAX), (-DBL_MAX));
        MySegment.SetRawEndPoint((-DBL_MAX), DBL_MAX);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint((-DBL_MAX), DBL_MAX);
        MySegment.SetRawEndPoint(DBL_MAX, DBL_MAX);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, DBL_MAX);
        MySegment.SetRawEndPoint(DBL_MAX, (-DBL_MAX));
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, (-DBL_MAX));
        MySegment.SetRawEndPoint((-DBL_MAX), (-DBL_MAX));
        pMyComplexLinear->AppendLinear(MySegment);
        }
    else
        {
        HVE2DSegment    MySegment(GetCoordSys());
        MySegment.SetRawStartPoint((-DBL_MAX), (-DBL_MAX));
        MySegment.SetRawEndPoint(DBL_MAX, (-DBL_MAX));
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, (-DBL_MAX));
        MySegment.SetRawEndPoint(DBL_MAX, DBL_MAX);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint(DBL_MAX, DBL_MAX);
        MySegment.SetRawEndPoint((-DBL_MAX), DBL_MAX);
        pMyComplexLinear->AppendLinear(MySegment);
        MySegment.SetRawStartPoint((-DBL_MAX), DBL_MAX);
        MySegment.SetRawEndPoint((-DBL_MAX), (-DBL_MAX));
        pMyComplexLinear->AppendLinear(MySegment);
        }

    return(pMyComplexLinear);
    }


//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method create a new shape as the difference between self and given.
// The two shapes must be expressed in the same coordinate system
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DUniverse::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape* pResultShape;
    // Check if the given shape is also universe
    if (pi_rShape.GetShapeType() == HVE2DUniverse::CLASS_ID)
        {
        // Both shapes are universe ... nothing left
        pResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        // Check if it is a simple shape
        if (pi_rShape.IsSimple())
            {
            // The result is a holed shape
            HVE2DHoledShape* pResultHoled = new HVE2DHoledShape(*this);

            pResultHoled->AddHole((*(HVE2DSimpleShape*)(&pi_rShape)));

            pResultShape = pResultHoled;
            }
        else
            {
            // Tell other shape to perform the process
            pResultShape = pi_rShape.DifferentiateFromShapeSCS(*this);
            }
        }

    return(pResultShape);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DUniverse::CalculateSpatialPositionOf(const HVE2DVector& pi_rVector) const
    {
    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_IN;

    if (pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID &&
        ((HVE2DShape*)(&pi_rVector))->GetShapeType() == HVE2DUniverse::CLASS_ID)
        {
        ThePosition = HVE2DShape::S_ON;
        }

    return (ThePosition);
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DUniverse::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HVE2DSimpleShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HVE2DUniverse" << endl;
    HDUMP0("Object is a HVE2DUniverse\n");

#endif
    }


//-----------------------------------------------------------------------------
// GetLightShape
// Allocates a light shape 
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DUniverse::GetLightShape() const
{
    return(new HGF2DUniverse());
}
