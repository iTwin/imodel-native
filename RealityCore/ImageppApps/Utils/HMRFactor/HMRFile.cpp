/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRFile.cpp $
|    $RCSfile: HMRFile.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2010/08/02 13:29:35 $
|     $Author: Jean.Lalande $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "HMRFile.h"

// I++
#include <Imagepp/all/h/HRFMacros.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HRFHMRFile.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
HMRFile::HMRFile(TCHAR* pFilename)
    {
    m_Status = 0;

    try
        {
        HFCPtr<HFCURLFile>  SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(pFilename)); 
        HFCPtr<HRFRasterFile> pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, HFC_READ_WRITE_OPEN | HFC_SHARE_READ_ONLY);

        if (pRasterFile == 0 ||
            !pRasterFile->IsCompatibleWith(HRFHMRFile::CLASS_ID))
            {
            m_Status = INVALIDFILE;
            }
        else
            {
            m_pHRFHMRFile = (HFCPtr<HRFHMRFile >&)pRasterFile;
            }
        }

    catch (HFCException& rException) //HFCFileReadOnlyException
        {

        if (dynamic_cast<HFCCannotLockFileException*>(&rException) != 0)
            {
            m_Status = CANTBELOCKED;       // Error : src file cannot be locked
            }
        else
            {        
            try
                {
                HFCPtr<HFCURLFile>  SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + 
                                                                         "://" + Utf8String(pFilename)); 
                HFCPtr<HRFRasterFile> pRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile((HFCPtr<HFCURL>)SrcFileName, 
                                                                                 HFC_READ_ONLY | HFC_SHARE_READ_ONLY);
                
                m_Status = INVALIDACCESSMODE;       // Error : src file is read-only.
                }
            catch (HFCException& )
                {
                m_Status = INVALIDFILE; // Error : Error invalid file or not supported"
                }
            }
        }
    catch(...)
        {
        m_Status = UNKNOWNERROR; // Error : "Unknown error"
        }   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
HMRFile::~HMRFile()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool HMRFile::ApplyFactor (double factor)
    {
    bool bWasModified (false);

    if (m_Status == 0 && 
        m_pHRFHMRFile != 0 &&
        factor != 0.0)
        {
        // Apply Factor to Origin and scaling
        HFCPtr<HGF2DTransfoModel> pTransfoModel (m_pHRFHMRFile->GetPageDescriptor (0)->GetTransfoModel());

        if (pTransfoModel->IsCompatibleWith(HGF2DAffine::CLASS_ID))
            {
            // Set the scaling
            ((HFCPtr<HGF2DAffine>&)pTransfoModel)->SetXScaling (((HFCPtr<HGF2DAffine>&)pTransfoModel)->GetXScaling () * factor);
            ((HFCPtr<HGF2DAffine>&)pTransfoModel)->SetYScaling (((HFCPtr<HGF2DAffine>&)pTransfoModel)->GetYScaling () * factor);

            // Now the translation
            HGF2DDisplacement  displacement (((HFCPtr<HGF2DAffine>&)pTransfoModel)->GetTranslation ());

            displacement *= factor;

            ((HFCPtr<HGF2DAffine>&)pTransfoModel)->SetTranslation (displacement);

            m_pHRFHMRFile->GetPageDescriptor (0)->SetTransfoModel(*pTransfoModel);

            bWasModified = true;
            }
        }

    return bWasModified;
    }