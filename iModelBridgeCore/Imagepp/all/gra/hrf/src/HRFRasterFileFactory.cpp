//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFileFactory.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRFRasterFileFactory
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCAccessMode.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>


// **** Kept here for history purpose only **********
#ifdef _HRF_COM_SUPPORT
    // Private COM interface for external file formats recognized by Image++ (version 1)
    // 513DEE64-6041-47F0-9211-2F712CE5E010
    //const IID HRF_COM1_IID_IClassID = {0x513DEE64,0x6041,0x47F0,{0x92,0x11,0x2F,0x71,0x2C,0xE5,0xE0,0x10}};
    // Private COM interface for external file formats recognized by Image++ (version 2)
    // CDC3B8C7-9AAD-4CE7-B893-6EF9CBC4AC14
    //const IID HRF_COM2_IID_IClassID = {0xCDC3B8C7,0x9AAD,0x4CE7,{0xB8,0x93,0x6E,0xF9,0xCB,0xC4,0xAC,0x14}};
    #define COM1_LIST_REG_KEY L"SOFTWARE\\Bentley\\AdditionalImageFileFormats"
    #define COM2_LIST_REG_KEY L"SOFTWARE\\Bentley\\AdditionalImageFileFormats\\HRFCOM2"
#endif

// Singleton
HFC_IMPLEMENT_SINGLETON(HRFRasterFileFactory)

//-----------------------------------------------------------------------------
// Public
// OpenFile
// Raster File object instanciation
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileFactory::OpenFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                     bool                 pi_AsReadOnly,
                                                     uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCAccessMode AccessMode = HFC_READ_ONLY | HFC_SHARE_READ_WRITE;

    if (!pi_AsReadOnly)
        AccessMode = DetectAccessMode(pi_rpURL);

    return OpenFile(pi_rpURL, AccessMode, pi_Offset);
    }

//-----------------------------------------------------------------------------
// Public
// OpenFileAs
// Raster File object instanciation
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileFactory::OpenFileAs(const HFCPtr<HFCURL>&       pi_rpURL,
                                                       const HRFRasterFileCreator* pi_pCreator,
                                                       bool                       pi_AsReadOnly,
                                                       uint64_t                   pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_pCreator != 0);

    HFCStat FileStat(pi_rpURL);

    // Open the file as specific creator
    if (!FileStat.IsExistent())
        throw(HFCFileNotFoundException(pi_rpURL->GetURL()));

    // Check if it is the good type of file format
    if (!pi_pCreator->IsKindOfFile(pi_rpURL, pi_Offset))
        throw(HFCFileNotSupportedException(pi_rpURL->GetURL()));

    HFCAccessMode AccessMode = HFC_READ_ONLY | HFC_SHARE_READ_WRITE;

    if (!pi_AsReadOnly)
        AccessMode = DetectAccessMode(pi_rpURL);

    return OpenFileAs(pi_rpURL, pi_pCreator, AccessMode, pi_Offset);
    }

//-----------------------------------------------------------------------------
// Public
// NewFile
// Raster File object instanciation
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileFactory::NewFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile>   p_RasterFile;
    HRFRasterFileCreator const*   pCreator = nullptr;

    // Instantiate the raster file
    pCreator = FindCreator(pi_rpURL, HFC_READ_WRITE_CREATE, pi_Offset);
    p_RasterFile = pCreator->Create(pi_rpURL, HFC_READ_WRITE_CREATE, 0);

    if (p_RasterFile == 0)
        throw(HFCFileNotCreatedException(pi_rpURL->GetURL()));

    return p_RasterFile;
    }

//-----------------------------------------------------------------------------
// Public
// NewFile
// Raster File object instanciations
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileFactory::NewFileAs(const HFCPtr<HFCURL>&       pi_rpURL,
                                                      const HRFRasterFileCreator* pi_pCreator,
                                                      uint64_t                   pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_pCreator != 0);
    return pi_pCreator->Create(pi_rpURL, HFC_READ_WRITE_CREATE, pi_Offset);
    }


//-----------------------------------------------------------------------------
// Public
// OpenFile
// Raster File object instanciation
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileFactory::OpenFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                     HFCAccessMode         pi_AccessMode,
                                                     uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile> p_RasterFile = 0;

    HFCStat FileStat(pi_rpURL);

    // Create the new file
    if (pi_AccessMode.m_HasCreateAccess)
        p_RasterFile = NewFile(pi_rpURL);
    else
        {
        switch(FileStat.DetectAccess())
            {
            case HFCStat::AccessGranted:
                break;  // Will try to open file.
            case HFCStat::TargetNotFound:
                throw(HFCFileNotFoundException(pi_rpURL->GetURL()));
            case HFCStat::AccessDenied:
                throw(HFCFilePermissionDeniedException(pi_rpURL->GetURL()));
            case HFCStat::AccessError:
            default:
                throw(HFCCannotOpenFileException(pi_rpURL->GetURL()));
            }

        HRFRasterFileCreator const*   pCreator = 0;

        // Obtain the registry
        pCreator = FindCreator(pi_rpURL, pi_AccessMode, pi_Offset);
        HASSERT(pCreator != 0);

        // Instantiate the raster file
        p_RasterFile = pCreator->Create(pi_rpURL, pi_AccessMode, pi_Offset);

        //Usually, a more precise exception should be thrown by the HRFRasterFile derived class
        //capable of handling the file's format.
        if (p_RasterFile == 0)
            throw(HFCCannotOpenFileException(pi_rpURL->GetURL()));
        }

    return p_RasterFile;
    }


//-----------------------------------------------------------------------------
// Public
// OpenFileAs
// Raster File object instanciation
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRasterFileFactory::OpenFileAs(const HFCPtr<HFCURL>&       pi_rpURL,
                                                       const HRFRasterFileCreator* pi_pCreator,
                                                       HFCAccessMode               pi_AccessMode,
                                                       uint64_t                   pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_pCreator != 0);
    HFCPtr<HRFRasterFile>   p_RasterFile;

    // Instantiate the raster file
    p_RasterFile = pi_pCreator->Create(pi_rpURL, pi_AccessMode, pi_Offset);

    //Usually, a more precise exception should be thrown by the HRFRasterFile derived class
    //capable of handling the file's format.
    if (p_RasterFile == 0)
        throw(HFCCannotOpenFileException(pi_rpURL->GetURL()));

    return p_RasterFile;
    }
//-----------------------------------------------------------------------------
// public
// FindCreator
// Search with the URL, AccessMode and offset the appropriate creator
//-----------------------------------------------------------------------------
const HRFRasterFileCreator* HRFRasterFileFactory::FindCreator(const HFCPtr<HFCURL>& pi_rpURL,
                                                              HFCAccessMode            pi_AccessMode,
                                                              uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HRFRasterFileCreator* pCreator = 0;
    Creators  TheCreators;
    Creators  InvalidCreators;

    // Get the creators for the specified access mode
    TheCreators = GetCreators(pi_AccessMode);

    //Some creators must have been found, otherwise, the Imagepp user
    //shouldn't have asked the access mode in the first place.
    HASSERT(TheCreators.size() != 0);

    // Iterator is used to loop through the vector.
    Creators::const_iterator CreatorIterator = TheCreators.begin();

    // Search a creator by the extension
    while (CreatorIterator != TheCreators.end())
        {
        // Test if this is the good creator for this file
        if ((*CreatorIterator)->SupportsURL(pi_rpURL))
            {
            if (!pi_AccessMode.m_HasCreateAccess)
                {
                if (!(*CreatorIterator)->IsKindOfFile(pi_rpURL, pi_Offset))
                    {
                    InvalidCreators.push_back(*CreatorIterator);
                    CreatorIterator++;
                    }
                else
                    {
                    pCreator = *CreatorIterator;
                    CreatorIterator = TheCreators.end();
                    }
                }
            else
                {
                pCreator = *CreatorIterator;
                CreatorIterator = TheCreators.end();
                }
            }
        else
            CreatorIterator++;
        }

    if (pCreator == 0 && !pi_AccessMode.m_HasCreateAccess && m_FactoryScanOnOpen)
        {
        // If the research by registry failed
        // We try to find with the is kind of file
        CreatorIterator = TheCreators.begin();
        Creators::const_iterator InvalidCreatorsItr(InvalidCreators.begin());

        // Search a creator by the is kind of file test
        while (CreatorIterator != TheCreators.end())
            {
            if (InvalidCreatorsItr != InvalidCreators.end() && *CreatorIterator == *InvalidCreatorsItr)
                InvalidCreatorsItr++;
            else if ((*CreatorIterator)->IsKindOfFile(pi_rpURL, pi_Offset))
                {
                // Found the creator
                pCreator = *CreatorIterator;
                // Stop the research
                CreatorIterator = TheCreators.end();
                }
            else
                CreatorIterator++;
            }
        }

    if (pCreator == 0)
        throw(HFCFileNotSupportedException(pi_rpURL->GetURL()));

    return pCreator;
    }

//-----------------------------------------------------------------------------
// Public
// DetectAccessMode
//-----------------------------------------------------------------------------
HFCAccessMode HRFRasterFileFactory::DetectAccessMode(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HFCAccessMode   AccessMode;

    // Automatic detect file access
    if (pi_rpURL->GetSchemeType().compare(HFCURLFile::s_SchemeName()) == 0)
        {
        WString FileName(((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost());
        FileName += L"\\";
        FileName += ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

        // Try to find the access mode of the specified file.
        if (static_cast<int>(BeFileName::CheckAccess(FileName.c_str(), BeFileNameAccess::ReadWrite)) == 0) 
            AccessMode = HFC_READ_WRITE_OPEN | HFC_SHARE_READ_ONLY;
        else
            AccessMode = HFC_READ_ONLY | HFC_SHARE_READ_WRITE;
        }
    else
        AccessMode = HFC_READ_ONLY | HFC_SHARE_READ_WRITE;

    return AccessMode;
    }


//-----------------------------------------------------------------------------
// Public
// GetCreators
// Raster File Creators registry
//-----------------------------------------------------------------------------
void HRFRasterFileFactory::GetCreators(Creators& creators, HFCAccessMode pi_AccessMode) const
    {
    HPRECONDITION(creators.empty());

    for (uint32_t Index(0); Index < m_Creators.size(); ++Index)
        {
        HFCAccessMode accessMode(m_Creators[Index]->GetSupportedAccessMode());

        if(accessMode.IsIncluded(pi_AccessMode))
            {
            creators.push_back(m_Creators[Index]);
            }
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCreators
// Raster File Creators registry
//-----------------------------------------------------------------------------
const HRFRasterFileFactory::Creators& HRFRasterFileFactory::GetCreators(
    HFCAccessMode pi_AccessMode) const
    {
    // remove the sharing from the access mode
    HFCAccessMode AccessMode = pi_AccessMode;
    AccessMode.m_HasReadShare       = false;
    AccessMode.m_HasWriteShare      = false;
    if (AccessMode == HFC_READ_ONLY)
        {
        if (m_ReadCreators.empty())
            {
            for (uint32_t Index=0; Index < m_Creators.size(); Index++)
                {
                if (m_Creators[Index]->GetSupportedAccessMode().m_HasReadAccess)
                    ((HRFRasterFileFactory*)this)->m_ReadCreators.push_back(m_Creators[Index]);
                }
            }
        return m_ReadCreators;
        }
    else if (AccessMode == HFC_WRITE_ONLY)
        {
        if (m_WriteCreators.empty())
            {
            for (uint32_t Index=0; Index < m_Creators.size(); Index++)
                {
                if (m_Creators[Index]->GetSupportedAccessMode().m_HasWriteAccess)
                    ((HRFRasterFileFactory*)this)->m_WriteCreators.push_back(m_Creators[Index]);
                }
            }
        return m_WriteCreators;
        }
    else if (AccessMode == HFC_CREATE_ONLY)
        {
        if (m_CreateCreators.empty())
            {
            for (uint32_t Index=0; Index < m_Creators.size(); Index++)
                {
                if (m_Creators[Index]->GetSupportedAccessMode().m_HasCreateAccess)
                    ((HRFRasterFileFactory*)this)->m_CreateCreators.push_back(m_Creators[Index]);
                }
            }
        return m_CreateCreators;
        }
    else
        return m_Creators;
    }


//-----------------------------------------------------------------------------
// Public
// GetCreatorsMap
// Raster File Creators Map registry
//-----------------------------------------------------------------------------
const HRFRasterFileFactory::CreatorsMap&
HRFRasterFileFactory::GetCreatorsMap(HFCAccessMode pi_AccessMode) const
    {
    const CreatorsMap* pResult = 0;

    // remove the sharing from access mode
    HFCAccessMode AccessMode = pi_AccessMode;
    AccessMode.m_HasReadShare       = false;
    AccessMode.m_HasWriteShare      = false;
    if (AccessMode == HFC_READ_ONLY)
        {
        if (m_ReadCreatorsMap.empty())
            {
            for (CreatorsMap::const_iterator Itr = m_CreatorsMap.begin(); Itr != m_CreatorsMap.end(); ++Itr)
                {
                if ((*Itr).second->GetSupportedAccessMode().m_HasReadAccess)
                    const_cast<HRFRasterFileFactory*>(this)->m_ReadCreatorsMap.insert(CreatorsMap::value_type((*Itr).first, (*Itr).second));
                }
            }
        pResult = &m_ReadCreatorsMap;
        }

    else if (AccessMode == HFC_WRITE_ONLY)
        {
        if (m_WriteCreatorsMap.empty())
            {
            for (CreatorsMap::const_iterator Itr = m_CreatorsMap.begin(); Itr != m_CreatorsMap.end(); ++Itr)
                {
                if ((*Itr).second->GetSupportedAccessMode().m_HasWriteAccess)
                    const_cast<HRFRasterFileFactory*>(this)->m_WriteCreatorsMap.insert(CreatorsMap::value_type((*Itr).first, (*Itr).second));
                }
            }

        pResult = &m_WriteCreatorsMap;
        }

    else if (AccessMode == HFC_CREATE_ONLY)
        {
        if (m_CreateCreatorsMap.empty())
            {
            for (CreatorsMap::const_iterator Itr = m_CreatorsMap.begin(); Itr != m_CreatorsMap.end(); ++Itr)
                {
                if ((*Itr).second->GetSupportedAccessMode().m_HasCreateAccess)
                    const_cast<HRFRasterFileFactory*>(this)->m_CreateCreatorsMap.insert(CreatorsMap::value_type((*Itr).first, (*Itr).second));
                }
            }

        pResult = &m_CreateCreatorsMap;
        }

    else
        pResult = &m_CreatorsMap;

    HPOSTCONDITION(pResult != 0);
    return (*pResult);
    }


//-----------------------------------------------------------------------------
// Protected
//-----------------------------------------------------------------------------
HRFRasterFileFactory::HRFRasterFileFactory ()
    : m_FactoryScanOnOpen(true),
      m_pPWHandler(0)
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
// singleton destruction
//-----------------------------------------------------------------------------
HRFRasterFileFactory::~HRFRasterFileFactory()
    {
    // Empty the registry
    m_Creators.clear();
    m_WriteCreators.clear();
    m_ReadCreators.clear();
    m_CreateCreators.clear();
    m_CreatorsMap.clear();
    m_WriteCreatorsMap.clear();
    m_ReadCreatorsMap.clear();
    m_CreateCreatorsMap.clear();
    }

//-----------------------------------------------------------------------------
// Public
// RegisterCreator
// Register the Raster File Creator
//-----------------------------------------------------------------------------
void HRFRasterFileFactory::RegisterCreator(const HRFRasterFileCreator* pi_pCreator)
    {
    if (pi_pCreator->CanRegister())
        {
        // Register the Raster File Creators ensuring that we do not add duplicates
        if (m_CreatorsMap.insert(CreatorsMap::value_type(pi_pCreator->GetRasterFileClassID(), (HRFRasterFileCreator*)pi_pCreator)).second)
            {
            m_Creators.push_back((HRFRasterFileCreator*)pi_pCreator);
            }
        }
    }


void HRFRasterFileFactory::SetRasterDllDirectory(HCLASS_ID pi_ClassID, const WString& pi_rDir)
    {
    DllDirMap::iterator Itr(m_DllDir.find(pi_ClassID));
    if (Itr == m_DllDir.end())
        m_DllDir.insert(DllDirMap::value_type(pi_ClassID, pi_rDir));
    else
        {
        m_DllDir.erase(Itr);
        m_DllDir.insert(DllDirMap::value_type(pi_ClassID, pi_rDir));
        }
    }

//-----------------------------------------------------------------------------
// Public
// RegisterPWHandler
//-----------------------------------------------------------------------------
void HRFRasterFileFactory::RegisterPWHandler(IHRFPWFileHandler* pi_pHandler)
    {
    m_pPWHandler = pi_pHandler;
    }

//-----------------------------------------------------------------------------
// Public
// GetPWHandler
//-----------------------------------------------------------------------------
IHRFPWFileHandler* HRFRasterFileFactory::GetPWHandler() const
    {
    return m_pPWHandler;
    }

void HRFRasterFileFactory::SetFactoryScanOnOpen(bool pi_FactoryScanOnOpen)
    {
    m_FactoryScanOnOpen = pi_FactoryScanOnOpen;
    }

bool HRFRasterFileFactory::GetFactoryScanOnOpen() const
    {
    return m_FactoryScanOnOpen;
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
HRFRasterFileCreator* HRFRasterFileFactory::GetCreator(HCLASS_ID pi_ClassID) const
    {
    CreatorsMap::const_iterator Itr(m_CreatorsMap.find(pi_ClassID));

    HRFRasterFileCreator* pResult = 0;
    if (Itr != m_CreatorsMap.end())
        pResult = Itr->second;

    return pResult;
    }


/*---------------------------------------------------------------------------------**//**
* Used this to avoid direct dependency on external libraries.
* ex: 
* HRFPDFCreator::GetInstance()->IsKindOfFile(*urlIter)                    << BAD
* HRFRasterFileFactory::GetInstance()->IsKindOfFile(HRFFileId_PDF, pURL)  << GOOD no direct dependency on PDF
* @bsimethod                                                   Mathieu.Marchand  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFRasterFileFactory::IsKindOfFile(HCLASS_ID rasterFileClassID, HFCPtr<HFCURL> const& pUrl) const
    {
    HRFRasterFileCreator* pCreator = GetCreator(rasterFileClassID);
    if(NULL == pCreator)
        return false;   // File format not registred. 

    return pCreator->IsKindOfFile(pUrl);    
    }
