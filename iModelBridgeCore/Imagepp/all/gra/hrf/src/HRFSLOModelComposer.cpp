//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSLOModelComposer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFSLOModelComposer
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFSLOModelComposer.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DPosition.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

HRFSLOModelComposer::HRFSLOModelComposer(HRFScanlineOrientation pi_ScanlineOrientation)
    {
    m_ScanLineOrigin = pi_ScanlineOrientation;
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------

HRFSLOModelComposer::HRFSLOModelComposer(const HRFSLOModelComposer& pi_rObj)
    {
    DeepCopy(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Equal operator
//-----------------------------------------------------------------------------

HRFSLOModelComposer& HRFSLOModelComposer::operator=
(const HRFSLOModelComposer& pi_rObj)
    {
    if(this != &pi_rObj)
        {
        DeepDelete();

        DeepCopy(pi_rObj);
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// GetTransfoModelFrom
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFSLOModelComposer::GetTransfoModelFrom(HRFScanlineOrientation  pi_ScanlineOrientation,
                                                                   uint32_t pi_Width,
                                                                   uint32_t pi_Height,
                                                                   HGF2DPosition& pi_Origin)
    {
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);

    HFCPtr<HGF2DAffine> pModelToSLO4 = 0;
    HFCPtr<HGF2DAffine> pModelFromSLO4 = 0;
    if (m_ScanLineOrigin != HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
        {
        pModelToSLO4 = GetTransfoModelToSLO4(m_ScanLineOrigin,
                                             pi_Width,
                                             pi_Height,
                                             pi_Origin);

        // if Scanline is vertical, flip the width and height
        if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL ||
            pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL ||
            pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL ||
            pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            uint32_t Temp = pi_Width;
            pi_Width = pi_Height;
            pi_Height = Temp;
            }

        // Set new origin
        pi_Origin.SetX(pModelToSLO4->GetTranslation().GetDeltaX());
        pi_Origin.SetY(pModelToSLO4->GetTranslation().GetDeltaY());
        }

    pModelFromSLO4 = GetTransfoModelFromSLO4(pi_ScanlineOrientation,
                                             pi_Width,
                                             pi_Height,
                                             pi_Origin);
    HFCPtr<HGF2DTransfoModel> pModel;
    if (pModelToSLO4 != 0)
        pModel = pModelFromSLO4->ComposeInverseWithDirectOf(*pModelToSLO4);
    else
        pModel = pModelFromSLO4;

    HFCPtr<HGF2DTransfoModel> pModelSimplify = pModel->CreateSimplifiedModel();
    if (pModelSimplify != 0)
        pModel = pModelSimplify;

    return pModel;
    }

//-----------------------------------------------------------------------------
// public
// CreateScanlineOrentationModel
//-----------------------------------------------------------------------------

HFCPtr<HGF2DAffine> HRFSLOModelComposer::GetIntergraphTransfoModelFrom
(HRFScanlineOrientation  pi_ScanlineOrientation,
 uint32_t pi_Width,
 uint32_t pi_Height,
 HGF2DPosition& pi_Origin)
    {
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);
    HPRECONDITION(m_ScanLineOrigin == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL);

    HFCPtr<HGF2DAffine> pTransfoModel = new HGF2DAffine();

    // Construct a transformation model according the right scan line orentation
    if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)       // Origin : Upper Left      Line Orentation : Vertical
        {
        pTransfoModel->AddHorizontalFlip(pi_Origin.GetX());
        pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)     // Origin : Upper Right     Line Orentation : Vertical
        {
        pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(0.0, -(double)pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)      // Origin : Lower Left      Line Orentation : Vertical
        {
        pTransfoModel->AddRotation(-PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, 0.0));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)     // Origin : Lower Right     Line Orentation : Vertical
        {
        pTransfoModel->AddHorizontalFlip(pi_Origin.GetX());
        pTransfoModel->AddRotation(-PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, -(double)pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)    // Origin : Upper Left      Line Orentation : Horizontal
        {
        // We are not supposed to do anything here because the SLO 4 is
        // the standard scan line orientation and do not need any transformation to be viewed
        // correctly.
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)   // Origin : Upper Right     Line Orentation : Horizontal
        {
        pTransfoModel->AddHorizontalFlip(pi_Origin.GetX());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, 0.0));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)    // Origin : Lower Left      Line Orentation : Horizontal
        {
        pTransfoModel->AddVerticalFlip(pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(0.0, -(double)pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)   // Origin : Lower Right     Line Orentation : Horizontal
        {
        pTransfoModel->AddRotation(PI, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, -(double)pi_Height));
        }
    else
        // Origin : Invalid         Line Orentation : Invalid
        // Invalid Scan Line Orientation. In release Keep the  SLO 4
        HASSERT(false);

    return (pTransfoModel);
    }

//-----------------------------------------------------------------------------
// private
// GetTransfoModelToSLO4
//-----------------------------------------------------------------------------
HFCPtr<HGF2DAffine> HRFSLOModelComposer::GetTransfoModelToSLO4(HRFScanlineOrientation   pi_ScanlineOrientation,
                                                               uint32_t                 pi_Width,
                                                               uint32_t                 pi_Height,
                                                               HGF2DPosition&           pi_Origin) const
    {
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);

    HFCPtr<HGF2DAffine> pTransfoModel = new HGF2DAffine();

    // Construct a transformation model according the right scan line orentation
    if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)       // Origin : Upper Left      Line Orentation : Vertical
        {
        pTransfoModel->AddVerticalFlip(pi_Origin.GetY());
        pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)     // Origin : Upper Right     Line Orentation : Vertical
        {
        pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, 0.0));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)      // Origin : Lower Left      Line Orentation : Vertical
        {
        pTransfoModel->AddRotation(-PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(0.0, pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)     // Origin : Lower Right     Line Orentation : Vertical
        {
        pTransfoModel->AddHorizontalFlip(pi_Origin.GetX());
        pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)    // Origin : Upper Left      Line Orentation : Horizontal
        {
        // We are not supposed to do anything here because the SLO 4 is
        // the standard scan line orientation and do not need any transformation to be viewed
        // correctly.
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)   // Origin : Upper Right     Line Orentation : Horizontal
        {
        pTransfoModel->AddHorizontalFlip(pi_Origin.GetX());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, 0.0));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)    // Origin : Lower Left      Line Orentation : Horizontal
        {
        pTransfoModel->AddVerticalFlip(pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(0.0, pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)   // Origin : Lower Right     Line Orentation : Horizontal
        {
        pTransfoModel->AddRotation(PI, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(pi_Width, pi_Height));
        }
    else
        // Origin : Invalid         Line Orentation : Invalid
        // Invalid Scan Line Orientation. In release Keep the  SLO 4
        HASSERT(false);

    return (pTransfoModel);
    }

//-----------------------------------------------------------------------------
// private
// GetTransfoModelFromSLO4
//-----------------------------------------------------------------------------
HFCPtr<HGF2DAffine> HRFSLOModelComposer::GetTransfoModelFromSLO4(HRFScanlineOrientation pi_ScanlineOrientation,
                                                                 uint32_t               pi_Width,
                                                                 uint32_t               pi_Height,
                                                                 HGF2DPosition&         pi_Origin) const
    {
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);

    HFCPtr<HGF2DAffine> pTransfoModel = new HGF2DAffine();

    // Construct a transformation model according the right scan line orentation
    if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)       // Origin : Upper Left      Line Orentation : Vertical
        {
        pTransfoModel->AddVerticalFlip(pi_Origin.GetY());
        pTransfoModel->AddRotation(-PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)     // Origin : Upper Right     Line Orentation : Vertical
        {
        pTransfoModel->AddRotation(-PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(-(double)pi_Width, 0.0));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)      // Origin : Lower Left      Line Orentation : Vertical
        {
        pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(0.0, -(double)pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)     // Origin : Lower Right     Line Orentation : Vertical
        {
        pTransfoModel->AddHorizontalFlip(pi_Origin.GetX());
        pTransfoModel->AddRotation(-PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(-(double)pi_Width, -(double)pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)    // Origin : Upper Left      Line Orentation : Horizontal
        {
        // We are not supposed to do anything here because the SLO 4 is
        // the standard scan line orientation and do not need any transformation to be viewed
        // correctly.
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)   // Origin : Upper Right     Line Orentation : Horizontal
        {
        pTransfoModel->AddHorizontalFlip(pi_Origin.GetX());
        pTransfoModel->AddTranslation(HGF2DDisplacement(-(double)pi_Width, 0.0));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)    // Origin : Lower Left      Line Orentation : Horizontal
        {
        pTransfoModel->AddVerticalFlip(pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(0.0, -(double)pi_Height));
        }
    else if (pi_ScanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)   // Origin : Lower Right     Line Orentation : Horizontal
        {
        pTransfoModel->AddRotation(PI, pi_Origin.GetX(), pi_Origin.GetY());
        pTransfoModel->AddTranslation(HGF2DDisplacement(-(double)pi_Width, -(double)pi_Height));
        }
    else
        // Origin : Invalid         Line Orentation : Invalid
        // Invalid Scan Line Orientation. In release Keep the  SLO 4
        HASSERT(false);

    return (pTransfoModel);
    }

//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------

void HRFSLOModelComposer::DeepCopy(const HRFSLOModelComposer& pi_rObj)
    {
    m_ScanLineOrigin = pi_rObj.m_ScanLineOrigin;
    }

//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------

void HRFSLOModelComposer::DeepDelete()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetScanLineOrigin
//-----------------------------------------------------------------------------

HRFScanlineOrientation HRFSLOModelComposer::GetScanLineOrigin() const
    {
    return m_ScanLineOrigin;
    }
