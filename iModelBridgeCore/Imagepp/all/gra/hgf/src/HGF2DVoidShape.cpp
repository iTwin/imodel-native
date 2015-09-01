//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DVoidShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DVoidShape
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DVoidShape.h>
#include <Imagepp/all/h/HGFScanLines.h>


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOf
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DVoidShape::CalculateSpatialPositionOf(const HGF2DVector& pi_rVector) const
    {
    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    if (pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID &&
        ((HGF2DShape*)(&pi_rVector))->GetShapeType() == HGF2DVoidShape::CLASS_ID)
        {
        ThePosition = HGF2DShape::S_ON;
        }

    return (ThePosition);
    }

//-----------------------------------------------------------------------------
// CreateScanLines
// This method creates the scan lines for the present shape
//-----------------------------------------------------------------------------
void HGF2DVoidShape::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    // Normally their is nothing to do here, to be more precise,
    // we are never supposed to get here. At least reset all ScanLines
    // limits and members to better reflect a void shape and
    // improve the debugging experience ;-)

    HASSERT(false);
    pio_rScanlines.ResetLimits();
    }


//-----------------------------------------------------------------------------
// @bsimethod                                                   2014/06
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HGF2DVoidShape::AllocTransformDirect(const HGF2DTransfoModel& pi_rModel) const
    {
    return (new HGF2DVoidShape(*this));
    }

//-----------------------------------------------------------------------------
// @bsimethod                                                   2014/06
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HGF2DVoidShape::AllocTransformInverse(const HGF2DTransfoModel& pi_rModel) const
    {
    return (new HGF2DVoidShape(*this));
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DVoidShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HGF2DSimpleShape::PrintState(po_rOutput);

    HDUMP0("Object is a HGF2DVoidShape\n");
    po_rOutput << "Object is a HGF2DVoidShape" << endl;

#endif
    }



