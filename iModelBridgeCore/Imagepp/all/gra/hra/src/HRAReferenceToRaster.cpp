//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAReferenceToRaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRAReferenceToRaster
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HRAReferenceToRasterEditor.h>
#include <Imagepp/all/h/HRAReferenceToRasterIterator.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <ImagePPInternal/gra/HRAImageNode.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>




HPM_REGISTER_CLASS(HRAReferenceToRaster, HRARaster)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRAReferenceToRaster, HRARaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRAReferenceToRaster, HGFGeometryChangedMsg, NotifyGeometryChanged)
HMG_REGISTER_MESSAGE(HRAReferenceToRaster, HRAEffectiveShapeChangedMsg, NotifyEffectiveShapeChanged)
HMG_REGISTER_MESSAGE(HRAReferenceToRaster, HRAProgressImageChangedMsg, NotifyProgressImageChanged)
HMG_END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// Default constructor.
//-----------------------------------------------------------------------------
HRAReferenceToRaster::HRAReferenceToRaster()
    : HRARaster()
    {
    m_EffectiveShapeDirty = true;

    m_Composable = false;
    }


//-----------------------------------------------------------------------------
// Constructor from source only.
//-----------------------------------------------------------------------------
HRAReferenceToRaster::HRAReferenceToRaster(const HFCPtr<HRARaster>& pi_pSource,
                                           bool                    pi_Composable)
    : HRARaster(pi_pSource->GetCoordSys()),
      m_pEffectiveShape(new HVEShape(pi_pSource->GetCoordSys())),
      m_pSource(pi_pSource)
    {
    // No geometry change
    m_CoordSysChanged     = false;
    m_ShapeChanged        = false;

    m_EffectiveShapeDirty = true;

    m_Composable = pi_Composable;

    SetDefaultShape();

    // Link ourselves to the source raster, to receive notifications
    LinkTo(m_pSource);
    }


//-----------------------------------------------------------------------------
// Constructor from source and shape.
//-----------------------------------------------------------------------------
HRAReferenceToRaster::HRAReferenceToRaster(const HFCPtr<HRARaster>& pi_pSource,
                                           const HVEShape&          pi_rShape,
                                           bool                    pi_Composable)
    : HRARaster(pi_pSource->GetCoordSys()),
      m_pEffectiveShape(new HVEShape(pi_pSource->GetCoordSys())),
      m_pSource(pi_pSource)
    {
    // No geometry change
    m_CoordSysChanged     = false;
    m_ShapeChanged        = true;

    m_EffectiveShapeDirty = true;

    m_Composable = pi_Composable;

    SetShape(pi_rShape);

    // Link ourselves to the source raster, to receive notifications
    LinkTo(m_pSource);
    }


//-----------------------------------------------------------------------------
// Constructor from source and coordinate system.
//-----------------------------------------------------------------------------
HRAReferenceToRaster::HRAReferenceToRaster(const HFCPtr<HRARaster>&     pi_pSource,
                                           const HFCPtr<HGF2DCoordSys>& pi_pCoordSys,
                                           bool                        pi_Composable)
    : HRARaster(pi_pCoordSys),
      m_pEffectiveShape(new HVEShape(pi_pCoordSys)),
      m_pSource(pi_pSource)
    {
    // Do we have a different coordinate system?
    if (pi_pCoordSys != pi_pSource->GetCoordSys())
        m_CoordSysChanged = true;
    else
        m_CoordSysChanged = false;

    m_ShapeChanged = false;

    m_EffectiveShapeDirty = true;

    m_Composable = pi_Composable;

    SetDefaultShape();

    // Link ourselves to the source raster, to receive notifications
    LinkTo(m_pSource);
    }


//-----------------------------------------------------------------------------
// Constructor from source coordinate system and shape.
//-----------------------------------------------------------------------------
HRAReferenceToRaster::HRAReferenceToRaster(const HFCPtr<HRARaster>&     pi_pSource,
                                           const HFCPtr<HGF2DCoordSys>& pi_pCoordSys,
                                           const HVEShape&              pi_rShape,
                                           bool                        pi_Composable)
    : HRARaster(pi_pCoordSys),
      m_pEffectiveShape(new HVEShape(pi_pCoordSys)),
      m_pSource(pi_pSource)
    {
    // Do we have a different coordinate system?
    if (pi_pCoordSys != pi_pSource->GetCoordSys())
        m_CoordSysChanged = true;
    else
        m_CoordSysChanged = false;

    m_ShapeChanged = true;

    m_EffectiveShapeDirty = true;

    m_Composable = pi_Composable;

    SetShape(pi_rShape);

    // Link ourselves to the source raster, to receive notifications
    LinkTo(m_pSource);
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRAReferenceToRaster::HRAReferenceToRaster(const HRAReferenceToRaster& pi_rObj)
    : HRARaster((HRARaster&)pi_rObj),
      m_pSource(pi_rObj.m_pSource)
    {
    HPRECONDITION(pi_rObj.m_pSource != 0);

    // Copy geometry status
    m_CoordSysChanged = pi_rObj.m_CoordSysChanged;
    m_ShapeChanged    = pi_rObj.m_ShapeChanged;

    m_Composable = pi_rObj.m_Composable;

    // Copy effective shape status
    m_EffectiveShapeDirty = pi_rObj.m_EffectiveShapeDirty;

    // Copy effective shape
    m_pEffectiveShape = new HVEShape(*pi_rObj.m_pEffectiveShape);

    // Link ourselves to the source raster, to receive notifications
    LinkTo(m_pSource);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAReferenceToRaster::~HRAReferenceToRaster()
    {
    if (m_pSource != 0)
        {
        // Unlink from the source raster
        UnlinkFrom(m_pSource);
        }
    }


//-----------------------------------------------------------------------------
// Assignment operation
//-----------------------------------------------------------------------------
HRAReferenceToRaster& HRAReferenceToRaster::operator=(const HRAReferenceToRaster& pi_rObj)
    {
    HPRECONDITION(pi_rObj.m_pSource != 0);
    HPRECONDITION(m_pSource != 0);

    if (&pi_rObj != this)
        {
        // Unlink from our previous source
        UnlinkFrom(m_pSource);

        // Copy the HRARaster portion
        HRARaster::operator=(pi_rObj);

        // Copy the other object's source pointer
        m_pSource = pi_rObj.m_pSource;

        // Copy geometry status
        m_CoordSysChanged = pi_rObj.m_CoordSysChanged;
        m_ShapeChanged    = pi_rObj.m_ShapeChanged;

        m_Composable = pi_rObj.m_Composable;

        // Copy effective shape status
        m_EffectiveShapeDirty = pi_rObj.m_EffectiveShapeDirty;

        // Copy cached effective shape
        if (!m_EffectiveShapeDirty)
            *m_pEffectiveShape = *pi_rObj.m_pEffectiveShape;

        // Link ourselves to the new source raster, to receive notifications
        LinkTo(m_pSource);
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// public
// CopyFrom
//-----------------------------------------------------------------------------
ImagePPStatus HRAReferenceToRaster::_CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& pi_rOptions)
    {
    HPRECONDITION(m_pSource != 0);
    HRACopyFromOptions newOptions(pi_rOptions);

    HVEShape newDestShape(GetShape());  // Always in reference's CS.

    // Take intersection of reference's shape and specified one
    if (NULL != pi_rOptions.GetEffectiveCopyRegion())
        {
        newDestShape.Intersect(*pi_rOptions.GetEffectiveCopyRegion());
        newOptions.SetEffectiveCopyRegion(&newDestShape);
        // Set shape to the source's coordinate system
        newDestShape.SetCoordSys(m_pSource->GetCoordSys());
        }

    // Extract the transformation we're applying to our source (T1)
    HFCPtr<HGF2DTransfoModel> pRefTransfo(m_pSource->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

    // Calculate the transformation between our CS and the CS
    // the object to copy uses. (T2)
    HFCPtr<HGF2DTransfoModel> pCurrentToRef(srcRaster.GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

    // Create the CS by composing everything correctly :)
    // T2 o T1 o -T2
    HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToRef->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToRef));
    HFCPtr<HGF2DCoordSys> pCSToApply(new HGF2DCoordSys(*pToApply, srcRaster.GetCoordSys()));

    //Need this to make sure we do no delete an HRaster that is not an HFCPtr.
    srcRaster.IncrementRef();
    HFCPtr<HRARaster> pRefToSrcRaster(new HRAReferenceToRaster(&srcRaster, pCSToApply));

    // Use source's CopyFrom, with new calculated shape
    ImagePPStatus status = m_pSource->CopyFrom(*pRefToSrcRaster, newOptions);

    srcRaster.DecrementRef();

    return status;

    // Our content has changed...
    // But don't propagate ContentChanged, since our source (to whom we are
    // registered) will call it when it has finished its CopyFrom().
    }

//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    HPRECONDITION(m_pSource != 0);

    HVEShape NewDestShape(GetShape());  // Always in reference's CS.

    // Take intersection of reference's shape and specified one
    const HVEShape* pShape = pi_rOptions.GetDestShape();

    if(pShape != 0)
        {
        NewDestShape.Intersect(*pShape);
        }

    // Set shape to the source's coordinate system
    NewDestShape.SetCoordSys(m_pSource->GetCoordSys());

    // Extract the transformation we're applying to our source (T1)
    HFCPtr<HGF2DTransfoModel> pRefTransfo(m_pSource->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

    // Calculate the transformation between our CS and the CS
    // the object to copy uses. (T2)
    HFCPtr<HGF2DTransfoModel> pCurrentToRef(pi_pSrcRaster->GetCoordSys()->GetTransfoModelTo(GetCoordSys()));

    // Create the CS by composing everything correctly :)
    // T2 o T1 o -T2
    HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToRef->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToRef));
    HFCPtr<HGF2DCoordSys> pCSToApply(new HGF2DCoordSys(*pToApply, pi_pSrcRaster->GetCoordSys()));

    HFCPtr<HRARaster> pRefToSrcRaster(new HRAReferenceToRaster(pi_pSrcRaster, pCSToApply));

    HRACopyFromLegacyOptions NewCopyFromOptions(pi_rOptions);
    NewCopyFromOptions.SetDestShape(&NewDestShape);

    // Use source's CopyFrom, with new calculated shape
    m_pSource->CopyFromLegacy(pRefToSrcRaster, NewCopyFromOptions);

    // Our content has changed...
    // But don't propagate ContentChanged, since our source (to whom we are
    // registered) will call it when it has finished its CopyFrom().
    }

//-----------------------------------------------------------------------------
// public
// CopyFrom
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    CopyFromLegacy(pi_pSrcRaster, HRACopyFromLegacyOptions());
    }


//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::Clear()
    {
    HRAClearOptions ClearOptions;
    Clear(ClearOptions);
    }

//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::Clear(const HRAClearOptions& pi_rOptions)
    {
    if (pi_rOptions.HasShape())
        {
        HVEShape NewDstShape(*pi_rOptions.GetShape());
        if (pi_rOptions.HasApplyRasterClipping())
            NewDstShape.Intersect(*GetEffectiveShape());

        NewDstShape.ChangeCoordSys(GetCoordSys());
        NewDstShape.SetCoordSys(m_pSource->GetCoordSys());

        HRAClearOptions Options(pi_rOptions);
        Options.SetShape(&NewDstShape);
        m_pSource->Clear(Options);
        }
    else
        m_pSource->Clear(pi_rOptions);
    }

//-----------------------------------------------------------------------------
// Create an editor
//-----------------------------------------------------------------------------
HRARasterEditor* HRAReferenceToRaster::CreateEditor(HFCAccessMode pi_Mode)
    {
    HPRECONDITION(m_pSource != 0);

    HRARasterEditor*  pEditor;

    if (m_CoordSysChanged)
        {
        // Create a HRAReferenceToRasterEditor
        pEditor = new HRAReferenceToRasterEditor(HFCPtr<HRAReferenceToRaster>(this), pi_Mode);
        }
    else
        {
        if (m_ShapeChanged)
            {
            HVEShape RefShape(GetShape());  // Always in reference's CS.

            // Set shape to the source's coordinate system
            RefShape.SetCoordSys(m_pSource->GetCoordSys());

            // Call source's CreateEditor, with reference's shape
            pEditor = m_pSource->CreateEditor(RefShape, pi_Mode);
            }
        else
            {
            // We can do without shaping
            pEditor = m_pSource->CreateEditor(pi_Mode);
            }
        }

    return pEditor;
    }


//-----------------------------------------------------------------------------
// Create a shaped editor
//-----------------------------------------------------------------------------
HRARasterEditor* HRAReferenceToRaster::CreateEditor(const HVEShape& pi_rShape,
                                                    HFCAccessMode   pi_Mode)
    {
    HPRECONDITION(m_pSource != 0);

    HRARasterEditor*  pEditor;

    if (m_CoordSysChanged)
        {
        // Create a HRAReferenceToRasterEditor
        pEditor = new HRAReferenceToRasterEditor(
            HFCPtr<HRAReferenceToRaster>(this),
            pi_rShape, pi_Mode);
        }
    else
        {
        HVEShape RefShape(GetShape());  // Always in reference's CS.

        // Take intersection of reference's shape and specified one
        RefShape.Intersect(pi_rShape);

        // Set shape to the source's coordinate system
        RefShape.SetCoordSys(m_pSource->GetCoordSys());

        // Call source's CreateEditor, with calculated shape
        pEditor = m_pSource->CreateEditor(RefShape, pi_Mode);
        }

    return pEditor;
    }



HRARasterEditor* HRAReferenceToRaster::CreateEditorUnShaped (HFCAccessMode pi_Mode)
    {
    HPRECONDITION(m_pSource != 0);

    HRARasterEditor*  pEditor;

    if (m_CoordSysChanged)
        {
        // Create a HRAReferenceToRasterEditor
        pEditor = new HRAReferenceToRasterEditor(HFCPtr<HRAReferenceToRaster>(this), pi_Mode);
        }
    else
        {
        pEditor = m_pSource->CreateEditorUnShaped(pi_Mode);
        }

    return pEditor;
    }


//-----------------------------------------------------------------------------
// Notification for shape changed
//-----------------------------------------------------------------------------
bool HRAReferenceToRaster::NotifyEffectiveShapeChanged (const HMGMessage& pi_rMessage)
    {
    // If we're still using the default shape
    if (!m_ShapeChanged)
        {
        // Reset our default shape
        SetDefaultShape();
        }

    // Can't trust our cached effective shape anymore
    m_EffectiveShapeDirty = true;

    // call the parent method
    HRARaster::InvalidateRepPalCache();

    return true;
    }


//-----------------------------------------------------------------------------
// Notification for geometry change
//-----------------------------------------------------------------------------
bool HRAReferenceToRaster::NotifyGeometryChanged (const HMGMessage& pi_rMessage)
    {
    bool PropagateMessage = false;

    // Retrieve pointer to sender object
    HFCPtr<HGF2DCoordSys> pSenderCoordSys = ((HRARaster*) pi_rMessage.GetSender())->GetCoordSys();

    if (m_CoordSysChanged)
        {
        // We have our own coordinate system. Change it to reflect
        // the source's change.

        // Set our new coordinate system. Use sender's CS as a reference,
        // keeping the transformation between our old CS and our old source's CS.
        SetCoordSys(new HGF2DCoordSys(*GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()),
                                      pSenderCoordSys));
        }
    else
        {
        if (GetCoordSys() != pSenderCoordSys)
            {
            // Set our new coordinate system as the source's
            SetCoordSys(pSenderCoordSys);
            }
        else
            {
            // The source CS has changed. But calling SetCoordSys wouldn't
            // do a thing since we're already using it. Instead, we'll
            // let the message go.
            m_EffectiveShapeDirty = true;

            if (!m_ShapeChanged)
                {
                // Reset our default shape
                SetDefaultShape();
                }

            PropagateMessage = true;
            }
        }

    return PropagateMessage;
    }


//-----------------------------------------------------------------------------
// Notification for image progression
//-----------------------------------------------------------------------------
bool HRAReferenceToRaster::NotifyProgressImageChanged (const HMGMessage& pi_rMessage)
    {
    // Take the shape in the reference's world
    HVEShape ReferenceShape(((HRAProgressImageChangedMsg&)pi_rMessage).GetShape());
    ReferenceShape.ChangeCoordSys(m_pSource->GetCoordSys());
    ReferenceShape.SetCoordSys(GetCoordSys());

    // Send a new message with that
    Propagate(HRAProgressImageChangedMsg(ReferenceShape, ((HRAProgressImageChangedMsg&)pi_rMessage).IsEnded()));

    // and stop the old one...
    return false;
    }


//-----------------------------------------------------------------------------
// Set our coordinate system
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys)
    {
    HPRECONDITION(m_pSource != 0);

    HRARaster::SetCoordSysImplementation(pi_rOldCoordSys);

    if (GetCoordSys() == m_pSource->GetCoordSys())
        m_CoordSysChanged = false;
    else
        m_CoordSysChanged = true;

    // Don't trust our cached effective shape anymore.
    m_EffectiveShapeDirty = true;

    // If we're still using the default shape
    if (!m_ShapeChanged)
        {
        // Reset our default shape
        SetDefaultShape();
        }
    }


//-----------------------------------------------------------------------------
// Move the reference
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HPRECONDITION(m_pSource != 0);

    // Model with translation.
    HGF2DTranslation NewModel (pi_rDisplacement);

    if (m_CoordSysChanged)
        {
        // We have a private coordinate system. Simply change the
        // model between it and the source's coordinate system

        SetCoordSys(new HGF2DCoordSys(*NewModel.ComposeInverseWithDirectOf (
                                          *(GetCoordSys()->GetTransfoModelTo (
                                                m_pSource->GetCoordSys()))),
                                      m_pSource->GetCoordSys()));
        }
    else
        {
        // Create our coordinate system, using the new transformation.

        SetCoordSys(new HGF2DCoordSys(NewModel, m_pSource->GetCoordSys()));

        // Don't notify, since SetCoordSys() will do it for us...
        }
    }


//-----------------------------------------------------------------------------
// Rotate the reference around a point
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::Rotate(double               pi_Angle,
                                  const HGF2DLocation& pi_rOrigin)
    {
    HPRECONDITION(m_pSource != 0);

    // Model with translation.
    HGF2DSimilitude Rotation;
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Rotation.AddRotation(pi_Angle, LogicalLocation.GetX(), LogicalLocation.GetY());

    if (m_CoordSysChanged)
        {
        // We have a private coordinate system. Simply change the
        // model between it and the source's coordinate system

        SetCoordSys(new HGF2DCoordSys(*Rotation.ComposeInverseWithDirectOf (
                                          *(GetCoordSys()->GetTransfoModelTo (
                                                m_pSource->GetCoordSys()))),
                                      m_pSource->GetCoordSys()));
        }
    else
        {
        // Create our coordinate system, using the new transformation.

        SetCoordSys(new HGF2DCoordSys(Rotation, m_pSource->GetCoordSys()));

        // Don't notify, since SetCoordSys() will do it for us...
        }
    }


//-----------------------------------------------------------------------------
// Rotate the reference around a point
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::Scale(double pi_ScaleFactorX,
                                 double pi_ScaleFactorY,
                                 const HGF2DLocation& pi_rOrigin)
    {
    HPRECONDITION(m_pSource != 0);

    // Model with translation.
    HGF2DStretch Scale;
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Scale.AddAnisotropicScaling(pi_ScaleFactorX, pi_ScaleFactorY,
                                LogicalLocation.GetX(), LogicalLocation.GetY());

    if (m_CoordSysChanged)
        {
        // We have a private coordinate system. Simply change the
        // model between it and the source's coordinate system

        SetCoordSys(new HGF2DCoordSys(*Scale.ComposeInverseWithDirectOf (
                                          *(GetCoordSys()->GetTransfoModelTo (
                                                m_pSource->GetCoordSys()))),
                                      m_pSource->GetCoordSys()));
        }
    else
        {
        // Create our coordinate system, using the new transformation.

        SetCoordSys(new HGF2DCoordSys(Scale, m_pSource->GetCoordSys()));

        // Don't notify, since SetCoordSys() will do it for us...
        }
    }


//-----------------------------------------------------------------------------
// Try to compose current reference and specified shape. Return new
// reference if necessary
//-----------------------------------------------------------------------------
HRAReferenceToRaster* HRAReferenceToRaster::TryToCompose(const HVEShape& pi_rShape)
    {
    if (m_Composable)
        {
        // We are composable. Simply intersect our shape with
        // the specified one.
        HVEShape TempShape(GetShape());
        TempShape.Intersect(pi_rShape);
        SetShape(TempShape);

        // And return ourselves
        return this;
        }
    else
        return new HRAReferenceToRaster(this, pi_rShape, true);
    }


//-----------------------------------------------------------------------------
// Try to compose current reference and specified coordinate system.
// Return new reference if necessary
//-----------------------------------------------------------------------------
HRAReferenceToRaster* HRAReferenceToRaster::TryToCompose(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    {
    if (m_Composable)
        {
        // We are composable. Use the specified coordinate system
        // Create a new coordsys


        HFCPtr<HGF2DTransfoModel> pTransfoModel(pi_pCoordSys->GetTransfoModelTo(GetCoordSys())->Clone());
        HFCPtr<HGF2DTransfoModel> pNewTransfoModel(pTransfoModel->ComposeInverseWithDirectOf(*(GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()))));
        HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys(*pNewTransfoModel, GetCoordSys()->GetReference()));
        SetCoordSys(pCoordSys);

        // And return ourselves
        return this;
        }
    else
        return new HRAReferenceToRaster(this, pi_pCoordSys, true);
    }



//-----------------------------------------------------------------------------
// Try to compose current reference and specified parameters. Return new
// reference if necessary
//-----------------------------------------------------------------------------
HRAReferenceToRaster* HRAReferenceToRaster::TryToCompose(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys,
                                                         const HVEShape&              pi_rShape)
    {
    if (m_Composable)
        {
        // We are composable. Simply intersect our shape with
        // the specified one...
        HVEShape TempShape(GetShape());
        TempShape.Intersect(pi_rShape);
        SetShape(TempShape);

        HFCPtr<HGF2DTransfoModel> pTransfoModel(pi_pCoordSys->GetTransfoModelTo(GetCoordSys())->Clone());
        HFCPtr<HGF2DTransfoModel> pNewTransfoModel(pTransfoModel->ComposeInverseWithDirectOf(*(GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()))));
        HFCPtr<HGF2DCoordSys> pCoordSys(new HGF2DCoordSys(*pNewTransfoModel, GetCoordSys()->GetReference()));
        SetCoordSys(pCoordSys);

        // And return ourselves
        return this;
        }
    else
        return new HRAReferenceToRaster(this, pi_pCoordSys, pi_rShape, true);
    }


//-----------------------------------------------------------------------------
// Public
// Passed the request to the source
//-----------------------------------------------------------------------------
bool HRAReferenceToRaster::HasLookAhead() const
    {
    HPRECONDITION(m_pSource != 0);

    return (m_pSource->HasLookAhead());
    }


//-----------------------------------------------------------------------------
// Public
// Pass the request to the source
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::SetLookAhead(const HVEShape& pi_rShape,
                                        uint32_t        pi_ConsumerID,
                                        bool           pi_Async)
    {
    HPRECONDITION(HasLookAhead());

    HFCPtr<HGF2DCoordSys> pReferenceLookAheadCS(new HGF2DCoordSys(
                                                    *GetSource()->GetCoordSys()->GetTransfoModelTo(GetCoordSys()),
                                                    pi_rShape.GetCoordSys()));

    // Take a copy of the shape and transform it to the source's position
    HVEShape TempShape(pi_rShape);
    TempShape.ChangeCoordSys(GetCoordSys());
    TempShape.SetCoordSys(m_pSource->GetCoordSys());

    // Use a destination CS that doesn't take the reference's
    // transformation into account.
    TempShape.ChangeCoordSys(pReferenceLookAheadCS);

    m_pSource->SetLookAhead(TempShape, pi_ConsumerID, pi_Async);
    }



//-----------------------------------------------------------------------------
// Print the object's state
//-----------------------------------------------------------------------------
#ifdef __HMR_PRINTSTATE
void HRAReferenceToRaster::PrintState(ostream& po_rOutput) const
    {
    // Call ancestor
    HRARaster::PrintState(po_rOutput);

    po_rOutput
            << "HRAReferenceToRaster"
            << endl;
    }
#endif


//-----------------------------------------------------------------------------
// Effective shape needs to be recalculated
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::RecalculateEffectiveShape()
    {
    m_EffectiveShapeDirty = true;
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    HRADrawOptions Options(pi_Options);

    // if there is no replacing coordsys, set ours
    if(Options.GetReplacingCoordSys() == 0)
        Options.SetReplacingCoordSys(GetCoordSys());

    // Take a copy of the shape and transform it to the source's position
    HFCPtr<HVEShape> pRegionToDraw;
    if (Options.GetShape() != 0)
        pRegionToDraw = new HVEShape(*Options.GetShape());
    else
        pRegionToDraw = new HVEShape(*GetEffectiveShape());

    pRegionToDraw->ChangeCoordSys(GetCoordSys());
    pRegionToDraw->SetCoordSys(m_pSource->GetCoordSys());

    Options.SetShape(pRegionToDraw);

    m_pSource->Draw(pio_destSurface, Options);
    }

/** -----------------------------------------------------------------------------
Set volatile layers
-----------------------------------------------------------------------------
*/
void HRAReferenceToRaster::SetContext(const HFCPtr<HMDContext>& pi_rpContext)
    {
    m_pSource->SetContext(pi_rpContext);
    }

/** -----------------------------------------------------------------------------
Get volatile layers
-----------------------------------------------------------------------------
*/
HFCPtr<HMDContext> HRAReferenceToRaster::GetContext()
    {
    return m_pSource->GetContext();
    }

/** -----------------------------------------------------------------------------
Invalidate raster
-----------------------------------------------------------------------------
*/
void HRAReferenceToRaster::InvalidateRaster()
    {
    m_pSource->InvalidateRaster();
    }

//-----------------------------------------------------------------------------
// Get the reference's average pixel size
//-----------------------------------------------------------------------------
HGF2DExtent HRAReferenceToRaster::GetAveragePixelSize () const
    {
    HPRECONDITION(m_pSource != 0);

#if (0)
    // Ask source...
    HGF2DExtent TempExtent(m_pSource->GetAveragePixelSize());

    // Make sure it's in the source's coordinate system
    TempExtent.ChangeCoordSys(m_pSource->GetCoordSys());

    // Put it in ours
    TempExtent.SetCoordSys(GetCoordSys());

#endif
    // Ask source...
    HGF2DExtent AverageSourcePixelSize(m_pSource->GetAveragePixelSize());

    double AverageX = 0.0;
    double AverageY = 0.0;

    if (AverageSourcePixelSize.IsDefined())
        {
        // Obtain the extent of the whole source whatever its type
        HGF2DExtent SourceImageExtent = m_pSource->GetExtent();
        HASSERT(SourceImageExtent.IsDefined());

        // Change this extent into the minimum pixel size coordsys.
        // This last coordsys should be the coordsys of the physical coordinate system
        // or pseudo physical coordinate system of the image that contains the smallest pixel
        // This operation does increase the size of the image extent but this approximation should
        // be sufficient and the size increase is nullified in later division.
        SourceImageExtent.ChangeCoordSys(AverageSourcePixelSize.GetCoordSys());

        // Extract the approximative width and height of image in the physical
        // coordinate system of the smallest image
        double ApproxImageWidth = SourceImageExtent.GetWidth();
        double ApproxImageHeight = SourceImageExtent.GetHeight();

        // Create four location to express the corners of this extent
        HGF2DLocation SourceImageLowerLeftCorner(SourceImageExtent.GetXMin(), SourceImageExtent.GetYMin(), SourceImageExtent.GetCoordSys());
        HGF2DLocation SourceImageUpperLeftCorner(SourceImageExtent.GetXMin(), SourceImageExtent.GetYMax(), SourceImageExtent.GetCoordSys());
        HGF2DLocation SourceImageUpperRightCorner(SourceImageExtent.GetXMax(), SourceImageExtent.GetYMax(), SourceImageExtent.GetCoordSys());
        HGF2DLocation SourceImageLowerRightCorner(SourceImageExtent.GetXMax(), SourceImageExtent.GetYMin(), SourceImageExtent.GetCoordSys());

        // Convert our coordinate throught the reference transformation
        SourceImageLowerLeftCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageUpperLeftCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageUpperRightCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageLowerRightCorner.ChangeCoordSys(m_pSource->GetCoordSys());

        SourceImageLowerLeftCorner.SetCoordSys(GetCoordSys());
        SourceImageUpperLeftCorner.SetCoordSys(GetCoordSys());
        SourceImageUpperRightCorner.SetCoordSys(GetCoordSys());
        SourceImageLowerRightCorner.SetCoordSys(GetCoordSys());

        // Obtain the size of distances in this coordinate system
        double XLower = (SourceImageLowerLeftCorner - SourceImageLowerRightCorner).CalculateLength();
        double XUpper = (SourceImageUpperLeftCorner - SourceImageUpperRightCorner).CalculateLength();
        double YLeft  = (SourceImageLowerLeftCorner - SourceImageUpperLeftCorner).CalculateLength();
        double YRight = (SourceImageLowerRightCorner - SourceImageUpperRightCorner).CalculateLength();

        AverageX = (XLower + XUpper) / 2.0;
        AverageY = (YLeft + YRight) / 2.0;

        // These averages are the average sizes in the current logical for the whole image
        // we divide for the size
        AverageX = AverageX / ApproxImageWidth;
        AverageY = AverageY / ApproxImageHeight;

        // We multiply by the original size of pixel
        AverageX = AverageX * AverageSourcePixelSize.GetWidth();
        AverageY = AverageY * AverageSourcePixelSize.GetHeight();
        }

    // We create the extent representing the average pixel size
    return(HGF2DExtent(0, 0, AverageX, AverageY, GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// Get the reference's pixel size range
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    HPRECONDITION(m_pSource != 0);

    // Ask source...
    HGF2DExtent SourceMinimum;
    HGF2DExtent SourceMaximum;
    m_pSource->GetPixelSizeRange(SourceMinimum, SourceMaximum);

    // Obtain the extent of the whole source whatever its type
    HGF2DExtent SourceImageExtent = m_pSource->GetExtent();

    double MinimumAverageX = 0.0;
    double MinimumAverageY = 0.0;
    double MaximumAverageX = 0.0;
    double MaximumAverageY = 0.0;

    double MinimumCenterX (0.0);
    double MinimumCenterY (0.0);
    double MaximumCenterX (0.0);
    double MaximumCenterY (0.0);


    if (SourceImageExtent.IsDefined() && SourceMinimum.IsDefined() && SourceMaximum.IsDefined())
        {
        // Change this extent into the minimum pixel size coordsys.
        // This last coordsys should be the coordsys of the physical coordinate system
        // or pseudo physical coordinate system of the image that contains the smallest pixel
        // This operation does increase the size of the image extent but this approximation should
        // be sufficient and the size increase is nullified in later division.
        SourceImageExtent.ChangeCoordSys(SourceMinimum.GetCoordSys());

        // Extract the approximative width and height of image in the physical
        // coordinate system of the smallest image
        double ApproxImageWidth = SourceImageExtent.GetWidth();
        double ApproxImageHeight = SourceImageExtent.GetHeight();

        // Create four location to express the corners of this extent
        HGF2DLocation SourceImageLowerLeftCorner(SourceImageExtent.GetXMin(), SourceImageExtent.GetYMin(), SourceImageExtent.GetCoordSys());
        HGF2DLocation SourceImageUpperLeftCorner(SourceImageExtent.GetXMin(), SourceImageExtent.GetYMax(), SourceImageExtent.GetCoordSys());
        HGF2DLocation SourceImageUpperRightCorner(SourceImageExtent.GetXMax(), SourceImageExtent.GetYMax(), SourceImageExtent.GetCoordSys());
        HGF2DLocation SourceImageLowerRightCorner(SourceImageExtent.GetXMax(), SourceImageExtent.GetYMin(), SourceImageExtent.GetCoordSys());
        HGF2DLocation SourceImageCenter((SourceImageExtent.GetOrigin().GetX() + SourceImageExtent.GetWidth()  / 2.0),
                                        (SourceImageExtent.GetOrigin().GetY() + SourceImageExtent.GetHeight() / 2.0),
                                        SourceImageExtent.GetCoordSys());

        // Convert our coordinate throught the reference transformation
        SourceImageLowerLeftCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageUpperLeftCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageUpperRightCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageLowerRightCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageCenter.ChangeCoordSys(m_pSource->GetCoordSys());

        SourceImageLowerLeftCorner.SetCoordSys(GetCoordSys());
        SourceImageUpperLeftCorner.SetCoordSys(GetCoordSys());
        SourceImageUpperRightCorner.SetCoordSys(GetCoordSys());
        SourceImageLowerRightCorner.SetCoordSys(GetCoordSys());
        SourceImageCenter.SetCoordSys(GetCoordSys());

        // Obtain the size of distances in this coordinate system
        double XLower = (SourceImageLowerLeftCorner - SourceImageLowerRightCorner).CalculateLength();
        double XUpper = (SourceImageUpperLeftCorner - SourceImageUpperRightCorner).CalculateLength();
        double YLeft  = (SourceImageLowerLeftCorner - SourceImageUpperLeftCorner).CalculateLength();
        double YRight = (SourceImageLowerRightCorner - SourceImageUpperRightCorner).CalculateLength();

        MinimumAverageX = (XLower + XUpper) / 2.0;
        MinimumAverageY = (YLeft + YRight) / 2.0;

        // These averages are the average sizes in the current logical for the whole image
        // we divide for the size
        MinimumAverageX = MinimumAverageX / ApproxImageWidth;
        MinimumAverageY = MinimumAverageY / ApproxImageHeight;

        // We multiply by the original size of pixel
        MinimumAverageX = MinimumAverageX * SourceMinimum.GetWidth();
        MinimumAverageY = MinimumAverageY * SourceMinimum.GetHeight();
        MinimumCenterX  = SourceImageCenter.GetX();
        MinimumCenterY  = SourceImageCenter.GetY();


        // Do the equivalent process for the maximum pixel size
        // Obtain the extent of the whole source whatever its type
        SourceImageExtent = m_pSource->GetExtent();

        // Change this extent into the minimum pixel size coordsys.
        // This last coordsys should be the coordsys of the physical coordinate system
        // or pseudo physical coordinate system of the image that contains the smallest pixel
        // This operation does increase the size of the image extent but this approximation should
        // be sufficient and the size increase is nullified in later division.
        SourceImageExtent.ChangeCoordSys(SourceMaximum.GetCoordSys());

        // Extract the approximative width and height of image in the physical
        // coordinate system of the smallest image
        ApproxImageWidth = SourceImageExtent.GetWidth();
        ApproxImageHeight = SourceImageExtent.GetHeight();

        // Create four location to express the corners of this extent
        SourceImageLowerLeftCorner = HGF2DLocation(SourceImageExtent.GetXMin(), SourceImageExtent.GetYMin(), SourceImageExtent.GetCoordSys());
        SourceImageUpperLeftCorner = HGF2DLocation(SourceImageExtent.GetXMin(), SourceImageExtent.GetYMax(), SourceImageExtent.GetCoordSys());
        SourceImageUpperRightCorner = HGF2DLocation(SourceImageExtent.GetXMax(), SourceImageExtent.GetYMax(), SourceImageExtent.GetCoordSys());
        SourceImageLowerRightCorner = HGF2DLocation(SourceImageExtent.GetXMax(), SourceImageExtent.GetYMin(), SourceImageExtent.GetCoordSys());
        SourceImageCenter           = HGF2DLocation((SourceImageExtent.GetOrigin().GetX() + SourceImageExtent.GetWidth()  / 2.0),
                                                    (SourceImageExtent.GetOrigin().GetY() + SourceImageExtent.GetHeight() / 2.0),
                                                    SourceImageExtent.GetCoordSys());

        // Convert our coordinate throught the reference transformation
        SourceImageLowerLeftCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageUpperLeftCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageUpperRightCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageLowerRightCorner.ChangeCoordSys(m_pSource->GetCoordSys());
        SourceImageCenter.ChangeCoordSys(m_pSource->GetCoordSys());

        SourceImageLowerLeftCorner.SetCoordSys(GetCoordSys());
        SourceImageUpperLeftCorner.SetCoordSys(GetCoordSys());
        SourceImageUpperRightCorner.SetCoordSys(GetCoordSys());
        SourceImageLowerRightCorner.SetCoordSys(GetCoordSys());
        SourceImageCenter.SetCoordSys(GetCoordSys());

        // Obtain the size of distances in this coordinate system
        XLower = (SourceImageLowerLeftCorner - SourceImageLowerRightCorner).CalculateLength();
        XUpper = (SourceImageUpperLeftCorner - SourceImageUpperRightCorner).CalculateLength();
        YLeft  = (SourceImageLowerLeftCorner - SourceImageUpperLeftCorner).CalculateLength();
        YRight = (SourceImageLowerRightCorner - SourceImageUpperRightCorner).CalculateLength();

        MaximumAverageX = (XLower + XUpper) / 2.0;
        MaximumAverageY = (YLeft + YRight) / 2.0;

        // These averages are the average sizes in the current logical for the whole image
        // we divide for the size
        MaximumAverageX = MaximumAverageX / ApproxImageWidth;
        MaximumAverageY = MaximumAverageY / ApproxImageHeight;

        // We multiply by the original size of pixel
        MaximumAverageX = MaximumAverageX * SourceMaximum.GetWidth();
        MaximumAverageY = MaximumAverageY * SourceMaximum.GetHeight();

        MaximumCenterX  = SourceImageCenter.GetX();
        MaximumCenterY  = SourceImageCenter.GetY();

        }

    // We create the extent representing the minimum pixel size
    po_rMinimum = HGF2DExtent(MinimumCenterX - MinimumAverageX / 2.0,
                              MinimumCenterY - MinimumAverageY / 2.0,
                              MinimumCenterX + MinimumAverageX / 2.0,
                              MinimumCenterY + MinimumAverageY / 2.0,
                              GetCoordSys());

    // We create the extent representing the minimum pixel size
    po_rMaximum = HGF2DExtent(MaximumCenterX - MaximumAverageX / 2.0,
                              MaximumCenterY - MaximumAverageY / 2.0,
                              MaximumCenterX + MaximumAverageX / 2.0,
                              MaximumCenterY + MaximumAverageY / 2.0,
                              GetCoordSys());
    }



//-----------------------------------------------------------------------------
// IsStoredRaster: depends on the source image.
//-----------------------------------------------------------------------------
bool HRAReferenceToRaster::IsStoredRaster () const
    {
    HPRECONDITION(m_pSource != 0);

    return m_pSource->IsStoredRaster();
    }


//-----------------------------------------------------------------------------
// CreateIterator
//-----------------------------------------------------------------------------
HRARasterIterator* HRAReferenceToRaster::CreateIterator(const HRAIteratorOptions& pi_rOptions) const
    {
    HPRECONDITION(IsStoredRaster());

    HRARasterIterator*  pIterator;

    if (m_CoordSysChanged)
        {
        // Return an iterator on the reference
        pIterator = new HRAReferenceToRasterIterator(
            HFCPtr<HRAReferenceToRaster>((HRAReferenceToRaster*)this),
            pi_rOptions);
        }
    else
        {
        // We can use the source's iterator

        if (m_ShapeChanged || pi_rOptions.IsShaped())
            {
            // Calculate intersection of specified region and our effective shape
            HFCPtr<HVEShape> pRefShape(pi_rOptions.CalculateClippedRegion(HFCPtr<HRARaster>((HRARaster*)this)));
            pRefShape->ChangeCoordSys(GetCoordSys());

            // Set it to the source's coordinate system
            pRefShape->SetCoordSys(m_pSource->GetCoordSys());

            // Simply return the source's iterator, with the reference's shape
            pIterator = m_pSource->CreateIterator(HRAIteratorOptions(pRefShape, pi_rOptions.GetPhysicalCoordSys(), false));
            }
        else
            {
            // Simply return the source's iterator
            pIterator = m_pSource->CreateIterator(HRAIteratorOptions(pi_rOptions.GetPhysicalCoordSys()));
            }
        }

    return pIterator;
    }


//-----------------------------------------------------------------------------
// ComputeHistogram
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                            bool                pi_ForceRecompute)
    {
    HPRECONDITION(IsStoredRaster());
    HPRECONDITION(pio_pOptions != 0);

    m_pSource->ComputeHistogram(pio_pOptions, pi_ForceRecompute);
    }


//-----------------------------------------------------------------------------
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRAReferenceToRaster::GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(IsStoredRaster());

    HPRECONDITION(pio_pRepPalParms != 0);

    return m_pSource->GetRepresentativePalette(pio_pRepPalParms);
    }

//-----------------------------------------------------------------------------
// Tell if reference has a single pixel type.
//-----------------------------------------------------------------------------
bool HRAReferenceToRaster::HasSinglePixelType() const
    {
    HPRECONDITION(m_pSource != 0);

    // Ask source...
    return m_pSource->HasSinglePixelType();
    }


//-----------------------------------------------------------------------------
// Return reference's pixel type.
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRAReferenceToRaster::GetPixelType() const
    {
    HPRECONDITION(m_pSource != 0);

    // Ask source...
    return m_pSource->GetPixelType();
    }


//-----------------------------------------------------------------------------
// Return reference's source raster object.
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster>& HRAReferenceToRaster::GetSource() const
    {
    HPRECONDITION(m_pSource != 0);

    return m_pSource;
    }


//-----------------------------------------------------------------------------
// Return reference's extent.
//-----------------------------------------------------------------------------
HGF2DExtent HRAReferenceToRaster::GetExtent() const
    {
    HPRECONDITION(m_pSource != 0);

    // Take extent of our effective shape
    return GetEffectiveShape()->GetExtent();
    }


//-----------------------------------------------------------------------------
// Set the reference's shape
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::SetShape(const HVEShape& pi_rShape)
    {
    // From now on, the geometry is changed.
    m_ShapeChanged = true;

    // Take copy of received shape
    HVEShape TempShape(pi_rShape);

    // Make sure it's in our coordinate system
    TempShape.ChangeCoordSys(GetCoordSys());

    // Our effective shape isn't valid now
    m_EffectiveShapeDirty = true;

    // Call ancestor...
    HRARaster::SetShape(TempShape);
    }


//-----------------------------------------------------------------------------
// Get the effective shape
//-----------------------------------------------------------------------------
HFCPtr<HVEShape> HRAReferenceToRaster::GetEffectiveShape () const
    {
    HPRECONDITION(m_pSource != 0);

    // Recalculate if necessary
    if (m_EffectiveShapeDirty)
        {
        // Take source's effective shape
        *m_pEffectiveShape = *(m_pSource->GetEffectiveShape());

        // Make sure it's in the source's coordinate system
        m_pEffectiveShape->ChangeCoordSys(m_pSource->GetCoordSys());

        // Translate it into our coordinate system
        m_pEffectiveShape->SetCoordSys(GetCoordSys());

        // Intersect it with our shape if necessary
        if (m_ShapeChanged)
            m_pEffectiveShape->Intersect(GetShape());

        // Now we're clean...
        m_EffectiveShapeDirty = false;
        }

    return m_pEffectiveShape;
    }


//-----------------------------------------------------------------------------
// Tell if reference's source has pixels using the specified channel
//-----------------------------------------------------------------------------
bool HRAReferenceToRaster::ContainsPixelsWithChannel(
    HRPChannelType::ChannelRole pi_Role,
    Byte                      pi_Id) const
    {
    HPRECONDITION(m_pSource != 0);

    return m_pSource->ContainsPixelsWithChannel(pi_Role, pi_Id);
    }


//-----------------------------------------------------------------------------
// Set reference's shape to default value
//-----------------------------------------------------------------------------
void HRAReferenceToRaster::SetDefaultShape()
    {
    HPRECONDITION(m_pSource != 0);

    // Take our source's effective shape
    HVEShape TempShape(*m_pSource->GetEffectiveShape());

    // Make sure it's in the source's coordinate system
    TempShape.ChangeCoordSys(m_pSource->GetCoordSys());

    // Put it in our coordinate system
    TempShape.SetCoordSys(GetCoordSys());

    // Call ancestor...
    HRARaster::SetShapeImpl(TempShape);
    }


//-----------------------------------------------------------------------------
// Return a new copy of self
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRAReferenceToRaster::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HRAReferenceToRaster(*this);
    }
//-----------------------------------------------------------------------------
// Return a new copy of self
//-----------------------------------------------------------------------------
HPMPersistentObject* HRAReferenceToRaster::Clone () const
    {
    return new HRAReferenceToRaster(*this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAReferenceToRaster::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    // Do not override the replacing
    if (NULL != options.GetReplacingCoordSys().GetPtr())
        return GetSource()->BuildCopyToContext(imageNode, options);

    HFCPtr<HGF2DTransfoModel> pDiffModel(GetCoordSys()->GetTransfoModelTo(GetSource()->GetCoordSys()));
    HFCPtr<HGF2DTransfoModel> pSimplModel(pDiffModel->CreateSimplifiedModel());
    if (NULL != pSimplModel.GetPtr())
        pDiffModel = pSimplModel;

    // If the diffModel is identity, nothing to do
    if (pDiffModel->IsCompatibleWith(HGF2DIdentityId))
        return GetSource()->BuildCopyToContext(imageNode, options);

    // Here we know that the HRAReferenceToRaster induce a transformation
    HRACopyToOptions newOptions(options);
    newOptions.SetReplacingCoordSys(GetCoordSys());
    return GetSource()->BuildCopyToContext(imageNode, newOptions);

    }
