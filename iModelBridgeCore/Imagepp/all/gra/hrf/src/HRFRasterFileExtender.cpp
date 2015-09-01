//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFileExtender.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class: HRFRasterFileExtender
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFRasterFileExtender.h>
#include <Imagepp/all/h/HMDContext.h>

//-----------------------------------------------------------------------------
// Public
// HRFRasterFileExtender
// Constructor and default value
//-----------------------------------------------------------------------------
HRFRasterFileExtender::HRFRasterFileExtender(const HFCPtr<HRFRasterFile>&  pi_rpOriginalFile)
    : HRFRasterFile(pi_rpOriginalFile->GetURL(), pi_rpOriginalFile->GetAccessMode(), 0)
    {
    m_pOriginalFile = pi_rpOriginalFile;
    }

//-----------------------------------------------------------------------------
// Public
// ~HRFRasterFileExtender
// Destructor
//-----------------------------------------------------------------------------
HRFRasterFileExtender::~HRFRasterFileExtender()
    {
    }


//-----------------------------------------------------------------------------
// Public
// GetKey
// Returns the exclusive key assigned to this object
//-----------------------------------------------------------------------------
inline HFCExclusiveKey& HRFRasterFileExtender::GetKey() const
    {
    HPRECONDITION(GetOriginalFile() != 0);

    return (GetOriginalFile()->GetKey());
    }


//-----------------------------------------------------------------------------
// public
// GetOriginalFile
// Allow to obtain the associated raster file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile>& HRFRasterFileExtender::GetOriginalFile() const
    {
    if (m_pOriginalFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        return ((HFCPtr<HRFRasterFileExtender>&)m_pOriginalFile)->GetOriginalFile();
    else
        return (HFCPtr<HRFRasterFile>&)m_pOriginalFile;
    }

//-----------------------------------------------------------------------------
// public
// GetExtendedFile
// Allow to obtain the associated raster file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileExtender::GetExtendedFile() const
    {
    return (m_pOriginalFile);
    }


//-----------------------------------------------------------------------------
// public
// IsCacheExtender
// Return false.
//
// Note : This method must be overwritten by all extender that cache a raster
//        file.
//-----------------------------------------------------------------------------
bool HRFRasterFileExtender::IsCacheExtender() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFRasterFileExtender::Save()
    {
    if (m_pOriginalFile->GetAccessMode().m_HasCreateAccess || m_pOriginalFile->GetAccessMode().m_HasWriteAccess)
        m_pOriginalFile->Save();
    }

//-----------------------------------------------------------------------------
// Public
// SetContext
// Set the context
//-----------------------------------------------------------------------------
void HRFRasterFileExtender::SetContext(uint32_t                 pi_PageIndex,
                                       const HFCPtr<HMDContext>& pi_rpContext)
    {
    return m_pOriginalFile->SetContext(pi_PageIndex, pi_rpContext);
    }

//-----------------------------------------------------------------------------
// Public
// GetContext
// Get the context
//-----------------------------------------------------------------------------
HFCPtr<HMDContext> HRFRasterFileExtender::GetContext(uint32_t pi_PageIndex) const
    {
    return m_pOriginalFile->GetContext(pi_PageIndex);
    }

//-----------------------------------------------------------------------------
// Public
// IsOriginalRasterDataStorage
//-----------------------------------------------------------------------------
inline bool HRFRasterFileExtender::IsOriginalRasterDataStorage() const
    {
    return m_pOriginalFile->IsOriginalRasterDataStorage();
    }

//-----------------------------------------------------------------------------
// Public
// CancelCreate
//-----------------------------------------------------------------------------
inline void HRFRasterFileExtender::CancelCreate()
    {
    m_pOriginalFile->CancelCreate();

    //Synchronize the cancellation with the extender.
    HRFRasterFile::CancelCreate();
    }