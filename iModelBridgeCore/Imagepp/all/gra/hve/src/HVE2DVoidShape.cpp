//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DVoidShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DVoidShape
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DVoidShape.h>
#include <Imagepp/all/h/HGFScanLines.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>

HPM_REGISTER_CLASS(HVE2DVoidShape, HVE2DSimpleShape)

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HVE2DVoidShape::HVE2DVoidShape()
    : HVE2DSimpleShape()
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HVE2DVoidShape::HVE2DVoidShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HVE2DVoidShape::HVE2DVoidShape(const HGF2DVoidShape& pi_rLightShape,
                               const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys)
    {
    // Nothing to do with the light shape provided.
    }



//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DVoidShape object.
//-----------------------------------------------------------------------------
HVE2DVoidShape::HVE2DVoidShape(const HVE2DVoidShape& pi_rObj)
    : HVE2DSimpleShape(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HVE2DVoidShape::~HVE2DVoidShape()
    {
    }

//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DVoidShape::CalculateSpatialPositionOf(const HVE2DVector& pi_rVector) const
    {
    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    if (pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID &&
        ((HVE2DShape*)(&pi_rVector))->GetShapeType() == HVE2DVoidShape::CLASS_ID)
        {
        ThePosition = HVE2DShape::S_ON;
        }

    return (ThePosition);
    }

//-----------------------------------------------------------------------------
// CreateScanLines
// This method creates the scan lines for the present shape
//-----------------------------------------------------------------------------
void HVE2DVoidShape::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    // Normally their is nothing to do here, to be more precise,
    // we are never supposed to get here. At least reset all ScanLines
    // limits and members to better reflect a void shape and
    // improve the debugging experience ;-)

    HASSERT(false);
    pio_rScanlines.ResetLimits();
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DVoidShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HVE2DSimpleShape::PrintState(po_rOutput);

    HDUMP0("Object is a HVE2DVoidShape\n");
    po_rOutput << "Object is a HVE2DVoidShape" << endl;

#endif
    }


//-----------------------------------------------------------------------------
// GetLightShape
// Allocates a light shape 
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DVoidShape::GetLightShape() const
{
    return(new HGF2DVoidShape());
}
