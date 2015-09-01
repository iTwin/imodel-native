//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFMappedSurface.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFMappedSurface
//-----------------------------------------------------------------------------
// This class encapsulates the functionalities of the page
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>

#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HGFMappedSurface::HGFMappedSurface(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor,
                                   const HFCPtr<HGF2DCoordSys>&         pi_rpRefCoordSys)
    : HRASurface(pi_rpDescriptor)
    {
    InitObject(pi_rpRefCoordSys);
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HGFMappedSurface::HGFMappedSurface(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor,
                                   const HGF2DExtent&                   pi_rExtent)
    : HRASurface(pi_rpDescriptor)
    {
    InitObject(pi_rExtent.GetCoordSys());

    FitToExtent(pi_rExtent);
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HGFMappedSurface::~HGFMappedSurface()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// GetExtent
//-----------------------------------------------------------------------------
HGF2DExtent HGFMappedSurface::GetExtent() const
    {
    HFCPtr<HGSSurfaceDescriptor> pDescriptor = GetSurfaceDescriptor();

    HGF2DExtent Extent( 0,
                        0,
                        m_Width,
                        m_Height,
                        GetCoordSys());

    return Extent;
    }

//-----------------------------------------------------------------------------
// Translate
//-----------------------------------------------------------------------------
void HGFMappedSurface::Translate(const HGF2DDisplacement& pi_rDisplacement)
    {
    // calculate a new transfo model
    HGF2DTranslation Translation(pi_rDisplacement);
    HFCPtr<HGF2DTransfoModel> pTransfo(Translation.ComposeInverseWithDirectOf(*(GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()))));

    // create a new coordsys
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(*pTransfo, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }


//-----------------------------------------------------------------------------
// TranslateFromRef
//-----------------------------------------------------------------------------
void HGFMappedSurface::TranslateFromRef(const HGF2DDisplacement& pi_rDisplacement)
    {
    // calculate a new transfo model
    HGF2DTranslation Translation(pi_rDisplacement);

    // create a new coordsys
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(Translation, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }

//-----------------------------------------------------------------------------
// Rotate
//-----------------------------------------------------------------------------
void HGFMappedSurface::Rotate(double pi_rRotation)
    {
    // For now, use an affine because of a bug when composing similitudes...
    HGF2DAffine Similitude(HGF2DDisplacement(0.0, 0.0), pi_rRotation, 1.0, 1.0, 0.0);

    HFCPtr<HGF2DTransfoModel> pTransfo(Similitude.ComposeInverseWithDirectOf(*(GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()))));

    // create a new coordsys
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(*pTransfo, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }


//-----------------------------------------------------------------------------
// RotateFromRef
//-----------------------------------------------------------------------------
void HGFMappedSurface::RotateFromRef(double pi_rRotation)
    {

    // calculate a new transfo model
    HGF2DSimilitude Similitude(HGF2DDisplacement(0, 0), 
                               pi_rRotation,
                               1.0);

    // create a new coordsys
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(Similitude, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }

//-----------------------------------------------------------------------------
// Scale
//-----------------------------------------------------------------------------
void HGFMappedSurface::Scale(double pi_ScaleX, double pi_ScaleY)
    {
    // calculate a new transfo model
    HGF2DStretch Stretch(HGF2DDisplacement(0.0, 0.0), 1.0 / pi_ScaleX, 1.0 / pi_ScaleY);

    HFCPtr<HGF2DTransfoModel> pTransfo(Stretch.ComposeInverseWithDirectOf(*(GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()))));

    // create a new coordsys
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(*pTransfo, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }


//-----------------------------------------------------------------------------
// ScaleFromRef
//-----------------------------------------------------------------------------
void HGFMappedSurface::ScaleFromRef(double pi_ScaleX, double pi_ScaleY)
    {
    // calculate a new transfo model
    HGF2DStretch Stretch(HGF2DDisplacement(0.0, 0.0), 1.0 / pi_ScaleX, 1.0 / pi_ScaleY);

    // create a new coordsys
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(Stretch, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }

//-----------------------------------------------------------------------------
// SetTransfoModelToRef
//-----------------------------------------------------------------------------
void HGFMappedSurface::SetTransfoModelToRef(const HGF2DTransfoModel& pi_rTransfoModel)
    {
    // create a new coordsys
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(pi_rTransfoModel, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }

//-----------------------------------------------------------------------------
// SetTransfoModelToSurface
//-----------------------------------------------------------------------------
void HGFMappedSurface::SetTransfoModelToSurface(const HGF2DTransfoModel& pi_rTransfoModel)
    {
    // create a new coordsys
    HGF2DTranslation Translation(HGF2DDisplacement(m_OffsetX, m_OffsetY));
    HFCPtr<HGF2DTransfoModel> pModel = Translation.ComposeInverseWithDirectOf(pi_rTransfoModel);
    m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(*pModel, GetCoordSys()->GetReference()));

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }

//-----------------------------------------------------------------------------
// FitToExtent
//-----------------------------------------------------------------------------
void HGFMappedSurface::FitToExtent(const HGF2DExtent& pi_rExtent)
    {
    // The provided extent must be defined
    HPRECONDITION(pi_rExtent.IsDefined());

    // In addition the extent width and height may not be 0.0 (exact compare)
    HPRECONDITION(pi_rExtent.GetWidth() != 0.0);
    HPRECONDITION(pi_rExtent.GetHeight() != 0.0);

    // obtain the best scale factor for the page
    HGF2DExtent Extent(pi_rExtent);

    // have a translation to go to the origin of the extent
    HGF2DTranslation Translation(HGF2DDisplacement(Extent.GetXMin(), Extent.GetYMin()));

    // have a stretch factor to fit the extent in the device
    HFCPtr<HGSSurfaceDescriptor> pDescriptor(GetSurfaceDescriptor());
    double XScalingToFitPage = (double)m_Width * GetSurfaceDescriptor()->GetWidthToHeightUnitRatio() / Extent.GetWidth();
    double YScalingToFitPage = (double)m_Height / (double)Extent.GetHeight();
    double ScaleFactor;
    if(XScalingToFitPage < YScalingToFitPage)
        ScaleFactor = XScalingToFitPage / GetSurfaceDescriptor()->GetWidthToHeightUnitRatio();
    else
        ScaleFactor = YScalingToFitPage;

    HGF2DStretch Stretch(HGF2DDisplacement((-1.0 * (m_Width / ScaleFactor - Extent.GetWidth()) / 2), 
                                           (-1.0 * (m_Height / ScaleFactor - Extent.GetHeight()) / 2)),
                         1.0 / ScaleFactor,
                         1.0 / ScaleFactor);

    // compose the two transformations
    HFCPtr<HGF2DTransfoModel> pTransfo(Stretch.ComposeInverseWithDirectOf(Translation));
    HFCPtr<HGF2DTransfoModel> pTransfoFinal;

    // find the transfo between the extent coordsys and the page coordsys
    if (GetCoordSys()->GetReference() != NULL)
        {
        HFCPtr<HGF2DTransfoModel> pTransfoExtentToPage(pi_rExtent.GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()));

        // compose the final transformation
        pTransfoFinal = pTransfo->ComposeInverseWithDirectOf(*pTransfoExtentToPage);

        // create the new coordsys
        m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(*pTransfoFinal, GetCoordSys()->GetReference()));
        }
    else
        {
        pTransfoFinal = pTransfo;

        // create the new coordsys
        m_pCoordSysContainer->SetCoordSys(new HGF2DCoordSys(*pTransfoFinal, GetCoordSys()));
        }

    // test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();

    UpdateSurfaceCoordSys();
    }

//-----------------------------------------------------------------------------
// InitObject
//-----------------------------------------------------------------------------
void HGFMappedSurface::InitObject(const HFCPtr<HGF2DCoordSys>& pi_rpRefCoordSys)
    {
    // A coordinate system must be provided
    HPRECONDITION(pi_rpRefCoordSys != 0);

    // offsets
    m_Width = GetSurfaceDescriptor()->GetWidth();
    m_Height = GetSurfaceDescriptor()->GetHeight();
    m_OffsetX = 0;
    m_OffsetY = 0;

    // Create the coordsys for the page and put in in a container
    m_pCoordSysContainer = new HGFCoordSysContainer(pi_rpRefCoordSys);

    // Create the device context coordsys (it can be offseted from the page coordsys)
    UpdateSurfaceCoordSys();

    // Test the SLO of the coordsys with the one of the page
    UpdateCoordSysWithSLO();
    }

//-----------------------------------------------------------------------------
// DeepDelete
//-----------------------------------------------------------------------------
void HGFMappedSurface::DeepDelete()
    {
    }

//-----------------------------------------------------------------------------
// UpdateCoordSysWithSLO
//-----------------------------------------------------------------------------
void HGFMappedSurface::UpdateCoordSysWithSLO()
    {
    // HLXHK SLO4 supported only
    m_pCoordSysForSLO = GetCoordSys();
    }

//-----------------------------------------------------------------------------
// UpdateSurfaceCoordSys
//-----------------------------------------------------------------------------
void HGFMappedSurface::UpdateSurfaceCoordSys()
    {
    HFCPtr<HGSSurfaceDescriptor> pDescriptor(GetSurfaceDescriptor());

    if ((m_OffsetX != 0) || (m_OffsetY != 0))
        {
        // create a translation
        HGF2DTranslation Translation(HGF2DDisplacement(-1.0 * m_OffsetX, -1.0 * m_OffsetY));

        m_pSurfaceCoordSys   = new HGF2DCoordSys(Translation, GetCoordSys());
        }
    else
        {
        m_pSurfaceCoordSys = GetCoordSys();
        }
    }


//-----------------------------------------------------------------------------
// SetOffsets
//-----------------------------------------------------------------------------
void HGFMappedSurface::SetOffsets(uint32_t pi_OffsetX, uint32_t pi_OffsetY)
    {
    m_OffsetX = pi_OffsetX;
    m_OffsetY = pi_OffsetY;

    GetSurfaceDescriptor()->SetDimensions(m_Width + m_OffsetX, m_Height + m_OffsetY);

    // update the coordsys of the surface from the logical coordsys
    UpdateSurfaceCoordSys();
    }
