//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVEShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVEShape
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


// The class declaration must be the last include file.
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>   // Coordinate systems
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DPolygon.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HGF2DHoledShape.h>
#include <Imagepp/all/h/HGF2DComplexShape.h>

HPM_REGISTER_CLASS(HVEShape, HGFGraphicObject)



//-----------------------------------------------------------------------------
// Constructor that gives an empty shape.
//-----------------------------------------------------------------------------
HVEShape::HVEShape()
    : HGFGraphicObject()
    {
    m_pShape = new HVE2DVoidShape(GetCoordSys());

    HVESHAPE_SYNCH_DEBUG_CODE
    }

//-----------------------------------------------------------------------------
// Constructor that gives an empty shape.
//-----------------------------------------------------------------------------
HVEShape::HVEShape(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    : HGFGraphicObject(pi_pCoordSys)
    {
    m_pShape = new HVE2DVoidShape(pi_pCoordSys);

    HVESHAPE_SYNCH_DEBUG_CODE
    }





//-----------------------------------------------------------------------------
// Constructor from a fence
//-----------------------------------------------------------------------------
HVEShape::HVEShape(const HGF2DShape& pi_rShape,
                   const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HGFGraphicObject(pi_rpCoordSys)
    {
    m_pShape = HVE2DShape::fCreateShapeFromLightShape(pi_rShape, pi_rpCoordSys);
    }
#if (0)
    // Check if fence is a rectangle
    if (pi_rShape.IsRectangle())
        {
        m_pShape = new HVE2DRectangle(static_cast<const HGF2DRectangle&>(pi_rShape), pi_rpCoordSys);
        }
    else if (pi_rShape.IsPolygon())
        {
        // Create polygon of segments from fence
        m_pShape = new HVE2DPolygonOfSegments(static_cast<const HGF2DPolygonFence&>(pi_rShape), pi_rpCoordSys);
        }
    // Check if fence is holed fence
    else if (pi_rShape.IsHoled())
        {
        // Cast as holed fence
        const HGF2DHoledShape& rHoledShape = static_cast<const HGF2DHoledShape & >(pi_rShape);

#if (0)
        m_pShape = new HVE2DHoledShape(rHoledShape);
#else

        // Check if holed is defined
        if (rHoledShape.IsDefined())
            {
            const HGF2DSimpleShape& rBase = rHoledFence.GetBase();

            // Create recipient
            HAutoPtr<HVE2DHoledShape> pHoledShape(new HVE2DHoledShape(pi_rpCoordSys));

            // Check nature of base fence
            if (rBase.IsRectangle())
                {
                // Cast as rectangle
                const HGF2DRectangle& rRectBase = static_cast<const HGF2DRectangle&>(rBase);

                // Create base shape as rectangle
                HVE2DRectangle BaseShape(rRectBase, pi_rpCoordSys);

                // Set base shape of holed
                pHoledShape->SetBaseShape(BaseShape);
                }
            else
                {
                HASSERT(rBase.IsPolygon());

                // Cast as polygon
                const HGF2DPolygonOfSegments& rPolygonBase = static_cast<const HGF2DPolygonOfSegments&>(rBase);

                // Create base shape as polygon
                HVE2DPolygonOfSegments BaseShape(rPolygonBase, pi_rpCoordSys);

                // Set base shape of holed
                pHoledShape->SetBaseShape(BaseShape);
                }

            // Obtain list of holes
            const HGF2DHoledShape::HoleList& rHoleList = rHoledFence.GetHoleList();

            // Create iterator on list
            HGF2DHoledShape::HoleList::const_iterator rHoleListItr = rHoleList.begin();

            // For every hole in holed fence
            for( ; rHoleListItr != rHoleList.end() ; ++rHoleListItr)
                {
                // Check nature of hole
                if ((*rHoleListItr)->IsRectangle())
                    {
                    HVE2DRectangle Hole(*(static_cast<HGF2DRectangle *>(*rHoleListItr)),
                                        pi_rpCoordSys);

                    pHoledShape->AddHole(Hole);
                    }
                else
                    {
                    // The hole must be a hole
                    HASSERT((*rHoleListItr)->IsPolygon());

                    // Create polygon which will be hole
                    HVE2DPolygonOfSegments Hole(*(static_cast<HGF2DPolygonOfSegments *>(*rHoleListItr)),
                                                pi_rpCoordSys);

                    pHoledShape->AddHole(Hole);
                    }
                }

            // Assign result to member
            m_pShape = pHoledShape.release();
#endif
            }
        else
            {
            // The holed is not defined ... set to void shape
            m_pShape = new HVE2DVoidShape(pi_rpCoordSys);
            }
        }
    else if(pi_rShape.IsComplex())
        {
        // The given fence is complex ... cast
        const HGF2DComplexShape& rComplexShape = static_cast<const HGF2DComplexShape&>(pi_rShape);

        // Allocate a complex shape
        HAutoPtr<HVE2DComplexShape> pNewComplexShape(new HVE2DComplexShape(pi_rpCoordSys));

        // Obtain list of fence in complex fence
        const HGF2DComplexShape::FenceList& rFenceList = rComplexFence.GetShapeList();

        // Create iterator on list
        HGF2DComplexFence<double>::FenceList::const_iterator FenceListItr = rFenceList.begin();

        // For every fence in complex fence
        for ( ; FenceListItr != rFenceList.end() ; ++FenceListItr)
            {
            // Check if component fence is polygon
            if ((*FenceListItr)->IsRectangle())
                {
                // Component is a rectangle fence ... cast
                const HGF2DRectangleFence<double>& rRectangleFence = static_cast<const HGF2DRectangleFence<double>&>(**FenceListItr);

                // Create a rectangle
                HVE2DRectangle ComponentShape(rRectangleFence, pi_rpCoordSys);

                // Add component shape to complex shape
                pNewComplexShape->AddShape(ComponentShape);

                }
            else if ((*FenceListItr)->IsPolygon())
                {
                // Component is a polygon fence ... cast
                const HGF2DPolygonFence<double>& rPolygonFence = static_cast<const HGF2DPolygonFence<double>&>(**FenceListItr);

                // Create a polygon of segments
                HVE2DPolygonOfSegments ComponentShape(rPolygonFence, pi_rpCoordSys);

                // Add component shape to complex shape
                pNewComplexShape->AddShape(ComponentShape);

                }
            else if ((*FenceListItr)->IsHoled())
                {
                // Fence is holed fence ... cast
                const HGF2DHoledFence<double>& rHoledFence = static_cast<const HGF2DHoledFence<double>&>(**FenceListItr);

                // Check if holed is defined
                if (rHoledFence.IsDefined())
                    {
                    // Obtain base fence
                    const HGF2DShape& rBase = rHoledFence.GetBaseShape();

                    // Create shape from base fence
                    HVE2DHoledShape HoledShape(pi_rpCoordSys);

                    // Check nature of base fence
                    if (rBase.IsRectangle())
                        {
                        // Cast as rectangle
                        const HGF2DRectangleFence<double>& rRectBase = static_cast<const HGF2DRectangleFence<double> &>(rBase);

                        // Create base shape as rectangle
                        HVE2DRectangle BaseShape(rRectBase, pi_rpCoordSys);

                        // Set base shape of holed
                        HoledShape.SetBaseShape(BaseShape);
                        }
                    else
                        {
                        HASSERT(rBase.IsPolygon());

                        // Cast as polygon
                        const HGF2DPolygonFence<double>& rPolygonBase = static_cast<const HGF2DPolygonFence<double>&>(rBase);

                        // Create base shape as polygon
                        HVE2DPolygonOfSegments BaseShape(rPolygonBase, pi_rpCoordSys);

                        // Set base shape of holed
                        HoledShape.SetBaseShape(BaseShape);
                        }

                    // Obtain list of holes
                    const HGF2DHoledFence<double>::HoleList& rHoleList = rHoledFence.GetHoleList();

                    // Create iterator on list
                    HGF2DHoledFence<double>::HoleList::const_iterator rHoleListItr = rHoleList.begin();

                    // For every hole in holed fence
                    for( ; rHoleListItr != rHoleList.end() ; ++rHoleListItr)
                        {
                        // Check nature of hole
                        if ((*rHoleListItr)->IsRectangle())
                            {
                            HVE2DRectangle Hole(*(static_cast<const HGF2DRectangleFence<double> *>(*rHoleListItr)),
                                                pi_rpCoordSys);

                            HoledShape.AddHole(Hole);
                            }
                        else
                            {
                            // The hole must be a hole
                            HASSERT((*rHoleListItr)->IsPolygon());

                            // Create polygon which will be hole
                            HVE2DPolygonOfSegments Hole(*(static_cast<const HGF2DPolygonFence<double> *>(*rHoleListItr)),
                                                        pi_rpCoordSys);

                            HoledShape.AddHole(Hole);
                            }
                        }

                    // Add holed to complex
                    pNewComplexShape->AddShape(HoledShape);
                    }
                else
                    {
                    // Holed is not defined ... no need to add
                    }
                }
            // No other fence possible
            HDEBUGCODE( else {
                HASSERT(0);
                });
            }
        // Assign result to member
        m_pShape = pNewComplexShape.release();

        }
    // Any other type should not exist
    HDEBUGCODE(else {
        HASSERT(0);
        });

    HVESHAPE_SYNCH_DEBUG_CODE
    }

#endif

    
//-----------------------------------------------------------------------------
// constructor.  It duplicates another shape
//-----------------------------------------------------------------------------
HVEShape::HVEShape(const HVE2DShape& pi_rObj)
    : HGFGraphicObject(pi_rObj)
    {
    m_pShape = (HVE2DShape*)pi_rObj.Clone();

    HVESHAPE_SYNCH_DEBUG_CODE
    }

//-----------------------------------------------------------------------------
// Constructor that gives a shape imitating an extent
//-----------------------------------------------------------------------------
HVEShape::HVEShape(const HGF2DExtent& pi_rExtent)
    : HGFGraphicObject(pi_rExtent.GetCoordSys())
    {
    // The extent must be defined
//    HPRECONDITION(pi_rExtent.IsDefined());

    if (!pi_rExtent.IsDefined())
        {
        m_pShape = new HVE2DVoidShape(pi_rExtent.GetCoordSys());
        }
    else
        {
        // Check that the extent values are not ridiculous
        double XMin = pi_rExtent.GetXMin();
        double XMax = pi_rExtent.GetXMax();
        double YMin = pi_rExtent.GetYMin();
        double YMax = pi_rExtent.GetYMax();

        // Check if extremeties are way out of bound
        if ((XMin < -DBL_MAX / 2) && (YMin < -DBL_MAX / 2) && (YMax > DBL_MAX / 2) && (XMax > DBL_MAX / 2))
            m_pShape = new HVE2DUniverse(pi_rExtent.GetCoordSys());
        else
            m_pShape = new HVE2DRectangle(pi_rExtent);
        }

    HVESHAPE_SYNCH_DEBUG_CODE
    }


//-----------------------------------------------------------------------------
// Constructs a shape representing a rectangle using the specified points
// and coordinate system.
//-----------------------------------------------------------------------------
HVEShape::HVEShape(double pi_x1, double pi_y1,
                   double pi_x2, double pi_y2,
                   const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    : HGFGraphicObject(pi_pCoordSys)
    {
    m_pShape = new HVE2DRectangle(pi_x1, pi_y1, pi_x2, pi_y2, pi_pCoordSys);

    HVESHAPE_SYNCH_DEBUG_CODE
    }


//-----------------------------------------------------------------------------
// Constructs a shape from a double array coming from the HMR file shape tag
//-----------------------------------------------------------------------------
HVEShape::HVEShape(size_t*                      po_pBufferLength,
                   double*                      pi_pBuffer,
                   const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    : HGFGraphicObject(pi_pCoordSys)
    {
    // Create a poly segment with given points
    HFCPtr<HVE2DPolySegment> pNewPolySegment = new HVE2DPolySegment(*po_pBufferLength, pi_pBuffer, pi_pCoordSys);

    // An auto contiguous polysegment is invalid
    HASSERT(!pNewPolySegment->IsAutoContiguous());

    // Check if the polysegment auto crosses
    if (pNewPolySegment->AutoCrosses())
        {
        m_pShape = HVE2DPolygonOfSegments::CreateShapeFromAutoCrossingPolySegment(*pNewPolySegment);
        }
    else
        {
        /*----------------------------------------------------------------------------
            The polysegment does not auto cross ... the polysegment should result
            in a valid polygon of segment that does not auto cross.
        ----------------------------------------------------------------------------*/
        m_pShape = new HVE2DPolygonOfSegments (*po_pBufferLength,
                                               pi_pBuffer,
                                               pi_pCoordSys);

        // If the polysegment is of rectangular shape, we prefer that it
        // be represented as a rectangle object (for performance reason)
        if (((HVE2DPolygonOfSegments*)m_pShape)->RepresentsARectangle())
            {
            HVE2DShape* pTempShape = m_pShape;

            m_pShape = ((HVE2DPolygonOfSegments*)pTempShape)->GenerateCorrespondingRectangle();

            delete pTempShape;
            }
        }

    HVESHAPE_SYNCH_DEBUG_CODE
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another shape object.
//-----------------------------------------------------------------------------
HVEShape::HVEShape(const HVEShape& pi_rObj)
    : HGFGraphicObject(pi_rObj)
    {
    m_pShape = (HVE2DShape*)pi_rObj.m_pShape->Clone();

    HVESHAPE_SYNCH_DEBUG_CODE
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HVEShape::~HVEShape()
    {
    delete m_pShape;
    }


//-----------------------------------------------------------------------------
// Tells if the shape is rectangular or not.
//-----------------------------------------------------------------------------
bool HVEShape::IsRectangle () const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return (m_pShape->GetShapeType() == HVE2DRectangle::CLASS_ID);
    }


//-----------------------------------------------------------------------------
// Move the shape
//-----------------------------------------------------------------------------
void HVEShape::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    m_pShape->Move(pi_rDisplacement);

    HVESHAPE_SYNCH_DEBUG_CODE
    }

//-----------------------------------------------------------------------------
// Scale
// Scales the shape by specified scale around the the specified location
//-----------------------------------------------------------------------------
void HVEShape::Scale(double pi_ScaleFactor,
                     const HGF2DLocation& pi_rOrigin)
    {
    // The given scale factor must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // Call the scaling with two scales
    Scale(pi_ScaleFactor, pi_ScaleFactor, pi_rOrigin);

    HVESHAPE_SYNCH_DEBUG_CODE
    }

//-----------------------------------------------------------------------------
// Scale
// Scales the shape by specified scale around the the specified location
//-----------------------------------------------------------------------------
void HVEShape::Scale(double pi_ScaleFactorX,
                     double pi_ScaleFactorY,
                     const HGF2DLocation& pi_rOrigin)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    if (m_pShape->IsCompatibleWith(HVE2DPolygonOfSegments::CLASS_ID))
        {
        // The shape is a polygon of segment ... can be rotated
        ((HVE2DPolygonOfSegments*)m_pShape)->Scale(pi_ScaleFactorX, pi_ScaleFactorY, pi_rOrigin);
        }
    else if (m_pShape->IsCompatibleWith(HVE2DVoidShape::CLASS_ID))
        {
        // The shape is a void ... nothing to do
        }
    else if (m_pShape->IsCompatibleWith(HVE2DUniverse::CLASS_ID))
        {
        // The shape is a universe ... nothing to do
        }
    else
        {
        // Some unknown type or a type which does not support rotation

        // We must create a transformation
        HGF2DStretch Stretch;

        // Obtain origin in current coordinate system
        HGF2DLocation Origin(pi_rOrigin, GetCoordSys());

        // Set transfo to given desired rotation
        Stretch.AddAnisotropicScaling(1 / pi_ScaleFactorX,
                                      1 / pi_ScaleFactorY,
                                      Origin.GetX(),
                                      Origin.GetY());

        // Create a new coordinate system linked to current using rotation model
        HFCPtr<HGF2DCoordSys> pNewCoordSys(new HGF2DCoordSys(Stretch, GetCoordSys()));

        // Allocate a copy of shape in new coordinate system
        HVE2DShape* pOldShape = m_pShape;

        m_pShape = static_cast<HVE2DShape*>(m_pShape->AllocateCopyInCoordSys(pNewCoordSys));

        // destroy old shape
        delete pOldShape;

        // Set coordinate system to current ... this results in the transformation
        // of our shape object
        m_pShape->SetCoordSys(GetCoordSys());

        // The previous command should result in destruction of temporary coordinate system
        }

    HVESHAPE_SYNCH_DEBUG_CODE
    }

//-----------------------------------------------------------------------------
// Rotate the shape
//-----------------------------------------------------------------------------
void HVEShape::Rotate(double pi_Angle, const HGF2DLocation& pi_rOrigin)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    if (m_pShape->IsCompatibleWith(HVE2DPolygonOfSegments::CLASS_ID))
        {
        // The shape is a polygon of segment ... can be rotated
        ((HVE2DPolygonOfSegments*)m_pShape)->Rotate(pi_Angle, pi_rOrigin);
        }
    else if (m_pShape->IsCompatibleWith(HVE2DVoidShape::CLASS_ID))
        {
        // The shape is a void shape  ... nothing to do
        }
    else if (m_pShape->IsCompatibleWith(HVE2DUniverse::CLASS_ID))
        {
        // The shape is a universe ... nothing to do
        }
    else
        {
        // Some unknown type or a type which does not support rotation

        // We must create a transformation
        HGF2DSimilitude Rotation;

        // Obtain origin in current coordinate system
        HGF2DLocation Origin(pi_rOrigin, GetCoordSys());

        // Set transfo to given desired rotation
        Rotation.AddRotation(pi_Angle, Origin.GetX(), Origin.GetY());

        // Create a new coordinate system linked to current using rotation model
        HFCPtr<HGF2DCoordSys> pNewCoordSys(new HGF2DCoordSys(Rotation, GetCoordSys()));

        // Allocate a copy of shape in new coordinate system
        HVE2DShape* pOldShape = m_pShape;

        m_pShape = static_cast<HVE2DShape*>(m_pShape->AllocateCopyInCoordSys(pNewCoordSys));

        // destroy old shape
        delete pOldShape;

        // Set coordinate system to current ... this results in the transformation
        // of our shape object
        m_pShape->SetCoordSys(GetCoordSys());

        // The previous command should result in destruction of temporary coordinate system
        }

    HVESHAPE_SYNCH_DEBUG_CODE
    }


//-----------------------------------------------------------------------------
// Transform the shape
//-----------------------------------------------------------------------------
void HVEShape::TransformDirect(HGF2DTransfoModel const& pi_rTransfo)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    HFCPtr<HGF2DCoordSys> pCoordSys(GetCoordSys());

    HFCPtr<HGF2DCoordSys> pTransformCoordSys(new HGF2DCoordSys(pi_rTransfo, pCoordSys));
    ChangeCoordSys(pTransformCoordSys);
    SetCoordSys(pCoordSys);
    }

//-----------------------------------------------------------------------------
// Transform the shape
//-----------------------------------------------------------------------------
void HVEShape::TransformInverse(HGF2DTransfoModel const& pi_rTransfo)
    {
    HFCPtr<HGF2DTransfoModel> pInverse = pi_rTransfo.Clone();
    pInverse->Reverse();

    TransformDirect(*pInverse);
    }

//-----------------------------------------------------------------------------
// Transport the shape from specified source to specified destination.
// This operation actually moves the shape in space.
// The coordinate system of the result shape is always the destination
// coordinate systems.
//-----------------------------------------------------------------------------
void HVEShape::Transport(HFCPtr<HGF2DCoordSys> const& fromCoordinateSystem, HFCPtr<HGF2DCoordSys> const& toCoordinateSystem)
    {
    ChangeCoordSys(fromCoordinateSystem);
    SetCoordSys(toCoordinateSystem);
    }

//-----------------------------------------------------------------------------
// Locate a point relative to the shape
//-----------------------------------------------------------------------------
HGFGraphicObject::Location HVEShape::Locate(const HGF2DLocation& pi_rPoint) const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return m_pShape->Locate(pi_rPoint);
    }


//-----------------------------------------------------------------------------
// Retrieve the shape's fence
//-----------------------------------------------------------------------------
HGF2DShape* HVEShape::GetLightShape() const
    {
    return m_pShape->GetLightShape();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
void HVEShape::SetStrokeTolerance(HFCPtr<HGFTolerance> & pi_Tolerance)
    {
    m_pShape->SetStrokeTolerance(pi_Tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGFTolerance> HVEShape::GetStrokeTolerance() const
    {
    return m_pShape->GetStrokeTolerance();
    }

//-----------------------------------------------------------------------------
// Sets the shape's coordinate system
//-----------------------------------------------------------------------------
void HVEShape::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    {
    // Set ancester coord sys
    HGFGraphicObject::SetCoordSysImplementation(pi_pCoordSys);

    // Set coord sys of shape
    m_pShape->SetCoordSys(GetCoordSys());

    HVESHAPE_SYNCH_DEBUG_CODE
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void HVEShape::PrintState (ostream& po_rOutput) const
    {
    //TODO:...
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.Marchand  01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef HVESHAPE_DEBUG_CODE

HFCPtr<HGF2DCoordSys> HVEShape::s_debug_refCoordSys = NULL;

void HVEShape::debug_SynchExtent() const
    {
    // Init to NULL and undefined.
    m_debug_shape = 0;
    m_debug_extent = HGF2DExtent();

    if(m_pShape != NULL)
        {
        // Use the top most reference of the current coordSys if we don't have one.
        if(s_debug_refCoordSys == 0)
            {
            s_debug_refCoordSys = GetCoordSys();

            while(s_debug_refCoordSys->GetReference() != NULL)
                {
                s_debug_refCoordSys = s_debug_refCoordSys->GetReference();
                }
            }

        // Make sure we have a relation to the ref coordSys.
        if(m_pShape->GetCoordSys()->GetTransfoModelTo(s_debug_refCoordSys) != NULL)
            {
            // Keep a copy of the shape and compute the shape extent.
            m_debug_shape = (HVE2DShape*)m_pShape->AllocateCopyInCoordSys(s_debug_refCoordSys);
            m_debug_extent = m_debug_shape->GetExtent();
            }
        }
    }
#endif



