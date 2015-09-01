//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAReferenceToStoredRaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRAReferenceToRaster
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRAReferenceToStoredRaster.h>
#include <Imagepp/all/h/HRAStoredRaster.h>
#include <Imagepp/all/h/HMGMessage.h>


HPM_REGISTER_CLASS(HRAReferenceToStoredRaster, HRAReferenceToRaster)


// Default constructor.
//-----------------------------------------------------------------------------
HRAReferenceToStoredRaster::HRAReferenceToStoredRaster()
    : HRAReferenceToRaster()
    {
    }

//-----------------------------------------------------------------------------
// Constructor from source only.
//-----------------------------------------------------------------------------
HRAReferenceToStoredRaster::HRAReferenceToStoredRaster(const HFCPtr<HRAStoredRaster>& pi_pSource)
    : HRAReferenceToRaster(reinterpret_cast<HFCPtr<HRARaster>const&> (pi_pSource))
    {
    HFCPtr<HRAStoredRaster> pStoredRaster (reinterpret_cast<HRAStoredRaster*> (pi_pSource.GetPtr()));
    HFCPtr<HGF2DTransfoModel> pSourcePhysicalToSourceLog(pStoredRaster->GetPhysicalCoordSys()->GetTransfoModelTo(pStoredRaster->GetCoordSys()));
    HFCPtr<HGF2DTransfoModel> pRefLogToSourceLog(GetCoordSys()->GetTransfoModelTo(pStoredRaster->GetCoordSys()));
    m_pPhysicalToLogical = pSourcePhysicalToSourceLog->ComposeInverseWithDirectOf(*pRefLogToSourceLog);
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
HRAReferenceToStoredRaster::~HRAReferenceToStoredRaster()
    {
    }

//-----------------------------------------------------------------------------
// SetCoordSysImplementation.
// This method is called whenever our internal coordinate system changed.
// Just reinitialize m_physicalToLogical.
//-----------------------------------------------------------------------------
void HRAReferenceToStoredRaster::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rOldCoordSys)
    {
    HFCPtr<HRAStoredRaster> pStoredRaster (reinterpret_cast<HRAStoredRaster*> (m_pSource.GetPtr()));
    HFCPtr<HGF2DTransfoModel> pSourcePhysicalToSourceLog(pStoredRaster->GetPhysicalCoordSys()->GetTransfoModelTo(pStoredRaster->GetCoordSys()));
    HFCPtr<HGF2DTransfoModel> pRefLogToSourceLog(GetCoordSys()->GetTransfoModelTo(pStoredRaster->GetCoordSys()));
    m_pPhysicalToLogical = pSourcePhysicalToSourceLog->ComposeInverseWithDirectOf(*pRefLogToSourceLog);

    T_Super::SetCoordSysImplementation(pi_rOldCoordSys);
    }

//-----------------------------------------------------------------------------
// NotifyGeometryChanged.
// This method is called whenever the source internal coordinate (or transformation model) system changed.
// Modify our internal coordinate system in order to keep the image at the same position according to
// the logical coordinate system of the source raster.
//-----------------------------------------------------------------------------
bool HRAReferenceToStoredRaster::NotifyGeometryChanged       (const HMGMessage& pi_rMessage)
    {
    bool PropagateMessage = false;

    // Retrieve pointer to sender object
    HFCPtr<HGF2DCoordSys> pSenderCoordSys = ((HRARaster*) pi_rMessage.GetSender())->GetCoordSys();

    HFCPtr<HRAStoredRaster> pStoredRaster (reinterpret_cast<HRAStoredRaster*> (m_pSource.GetPtr()));
    HFCPtr<HGF2DTransfoModel> pSourceLogToSourcePhysical(pStoredRaster->GetCoordSys()->GetTransfoModelTo(pStoredRaster->GetPhysicalCoordSys()));

    // Compute diff model
    HFCPtr<HGF2DTransfoModel> pDiffModel (pSourceLogToSourcePhysical->ComposeInverseWithDirectOf(*m_pPhysicalToLogical));

    SetCoordSys(new HGF2DCoordSys(*pDiffModel, pSenderCoordSys));

    return PropagateMessage;
    }

