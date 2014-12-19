//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPageFile.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFPageFile
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCStat.h>

#include <Imagepp/all/h/HRFPageFile.h>
#include <Imagepp/all/h/HRFResolutionEditor.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFPageFile::HRFPageFile(const HFCPtr<HFCURL>& pi_rpURL,
                                HFCAccessMode         pi_AccessMode)
    {
    HPRECONDITION(pi_rpURL != 0);
    m_pURL           = pi_rpURL;
    m_FileAccessMode = pi_AccessMode;
    m_DefaultRatioToMeter = 1.0;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFPageFile::~HRFPageFile()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Set the default ratio to meter in case the file has no units information.
//-----------------------------------------------------------------------------
void HRFPageFile::SetDefaultRatioToMeter(double pi_RatioToMeter)
    {
    HPRECONDITION(pi_RatioToMeter != 0.0);

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    if ((pPageDescriptor != 0) &&
        (pPageDescriptor->HasTransfoModel() == true) &&
        (m_DefaultRatioToMeter != pi_RatioToMeter))
        {
        double RatioToMeter = pi_RatioToMeter / m_DefaultRatioToMeter;
        m_DefaultRatioToMeter = pi_RatioToMeter;

        IRasterBaseGcsPtr pBaseGCS = pPageDescriptor->GetGeocoding();

        HFCPtr<HGF2DTransfoModel> pTransfoModel = pPageDescriptor->GetTransfoModel();
        
        if ((pBaseGCS != 0) && (pBaseGCS->IsValid()))
            {
            pTransfoModel = HCPGeoTiffKeys::TranslateToMeter(pPageDescriptor->GetTransfoModel(),
                                                             RatioToMeter,
                                                             true,
                                                             false,
                                                             NULL,
                                                             pBaseGCS);
            }

        pPageDescriptor->SetTransfoModel(*pTransfoModel, true /*ignore capabilities*/);
        pPageDescriptor->SetTransfoModelUnchanged();
        }
    }

//-----------------------------------------------------------------------------
// Public
// Get the default ratio to meter
//-----------------------------------------------------------------------------
double HRFPageFile::GetDefaultRatioToMeter() const
    {
    return m_DefaultRatioToMeter;
    }

//-----------------------------------------------------------------------------
// Public
// FoundFileFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFPageFileCreator::FoundFileFor(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Get the URL for the sister file.
    HFCPtr<HFCURL> pPageFileURL = ComposeURLFor(pi_rpURL);

    HAutoPtr<HFCStat> pPageFileStat;
    pPageFileStat = new HFCStat(pPageFileURL);

    // Check if the decoration file exist
    if (!pPageFileStat->IsExistent())
        pPageFileURL = 0;

    return pPageFileURL;
    }
