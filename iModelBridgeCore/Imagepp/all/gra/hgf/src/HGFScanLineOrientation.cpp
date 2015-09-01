//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFScanLineOrientation.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFScanLineOrientation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFScanLineOrientation.h>
#include <Imagepp/all/h/HGF2DAffine.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HGFScanLineOrientation::HGFScanLineOrientation(HGFSLO pi_ScanLineOrientation)
    {
    // Actually can we only have a SLO TransfoModel generator from SLO 4 => SLO [0,7]

    HPRECONDITION(pi_ScanLineOrientation == HGF_UPPER_LEFT_HORIZONTAL);

    m_ScanLineOrigin = pi_ScanLineOrientation;
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HGFScanLineOrientation::HGFScanLineOrientation(const HGFScanLineOrientation& pi_rObj)
    {
    DeepCopy(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Equal operator
//-----------------------------------------------------------------------------
HGFScanLineOrientation& HGFScanLineOrientation::operator=
(const HGFScanLineOrientation& pi_rObj)
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
// CreateScanLineOrentationModel
//-----------------------------------------------------------------------------

HFCPtr<HGF2DTransfoModel> HGFScanLineOrientation::GetTransfoModelFrom(HGFSLO         pi_ScanLineOrientation,
                                                                      uint32_t       pi_Width,
                                                                      uint32_t       pi_Height,
                                                                      HGF2DPosition& pi_Origin)
    {
    HPRECONDITION(pi_Width > 0);
    HPRECONDITION(pi_Height > 0);
    HPRECONDITION(m_ScanLineOrigin == HGF_UPPER_LEFT_HORIZONTAL);

    HFCPtr<HGF2DAffine> pTransfoModel = new HGF2DAffine();

    // construct a transformation model according the right scan line orentation
    switch (pi_ScanLineOrientation)
        {

        case HGF_UPPER_LEFT_VERTICAL:       // Origin : Upper Left      Line Orentation : Vertical
            pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
            //pTransfoModel->AddAnisotropicScaling(1,-1);
            pTransfoModel->AddVerticalFlip(pi_Height);
            break;

        case HGF_UPPER_RIGHT_VERTICAL:      // Origin : Upper Right     Line Orentation : Vertical
            pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
            break;

        case HGF_LOWER_LEFT_VERTICAL:       // Origin : Lower Left      Line Orentation : Vertical
            pTransfoModel->AddRotation(-PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
            break;

        case HGF_LOWER_RIGHT_VERTICAL:      // Origin : Lower Right     Line Orentation : Vertical
            //pTransfoModel->AddAnisotropicScaling(-1,1);
            pTransfoModel->AddHorizontalFlip(pi_Width);
            pTransfoModel->AddRotation(PI/2.0, pi_Origin.GetX(), pi_Origin.GetY());
            break;

        case HGF_UPPER_LEFT_HORIZONTAL:     // Origin : Upper Left      Line Orentation : Horizontal
            // We are not supposed to do anything here because the SLO 4 is
            // the standard scan line orientation and do not need any transformation to be viewed
            // correctly.
            break;

        case HGF_UPPER_RIGHT_HORIZONTAL:    // Origin : Upper Right     Line Orentation : Horizontal
            //pTransfoModel->AddAnisotropicScaling(-1,1);
            pTransfoModel->AddHorizontalFlip(pi_Width);
            break;

        case HGF_LOWER_LEFT_HORIZONTAL:     // Origin : Lower Left      Line Orentation : Horizontal
            //pTransfoModel->AddAnisotropicScaling(1,-1);
            pTransfoModel->AddVerticalFlip(pi_Height);
            break;

        case HGF_LOWER_RIGHT_HORIZONTAL:    // Origin : Lower Right     Line Orentation : Horizontal
            pTransfoModel->AddRotation(PI, pi_Origin.GetX(), pi_Origin.GetY());
            break;

        default :          // Invalid Scan Line Orientation. In release Keep the  SLO 4
            HASSERT(false);
            break;
        }
    return ((HFCPtr<HGF2DTransfoModel>&)pTransfoModel);
    }

//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HGFScanLineOrientation::DeepCopy(const HGFScanLineOrientation& pi_rObj)
    {
    m_ScanLineOrigin = pi_rObj.m_ScanLineOrigin;
    }

//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HGFScanLineOrientation::DeepDelete()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetScanLineOrigin
//-----------------------------------------------------------------------------
HGFSLO HGFScanLineOrientation::GetScanLineOrigin() const
    {
    return m_ScanLineOrigin;
    }
