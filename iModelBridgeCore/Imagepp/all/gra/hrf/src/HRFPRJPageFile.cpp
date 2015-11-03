//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPRJPageFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFPRJPageFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HTIFFTag.h>

#include <Imagepp/all/h/HGFAngle.h>
#include <Imagepp/all/h/HRFPRJPageFile.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCIniFileBrowser.h>

#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFUtility.h>



//-----------------------------------------------------------------------------
// Class HRFPRJCapabilities
//-----------------------------------------------------------------------------
HRFPRJCapabilities::HRFPRJCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeographicType);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    }


// Singleton
HFC_IMPLEMENT_SINGLETON(HRFPRJPageFileCreator)

//-----------------------------------------------------------------------------
// Class HRFPRJPageFileCreator
//
// This is a utility class to create a specific object.
// It is used by the Page File factory.
//-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 @return true if the file sister file PRJ associate with the Raster file
         is found.

 @param pi_rpForRasterFile Raster file source.
 @param pi_ApplyonAllFiles true : Try to find an associate PRJ sister file with
                                  all types of files(ITiff, GeoTiff, etc)
                     false(default) : Try to find an associate sister file PRJ
                     only with the files don't support Georeference.
-----------------------------------------------------------------------------*/
bool HRFPRJPageFileCreator::HasFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile,
                                    bool                         pi_ApplyonAllFiles) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    bool HasPageFile = false;

    // Check only the first page in the File.
    // Check ... The present page file type is meant to replace or add a geocoding contrary to other
    // sister files that only replace or add georeference (transformation model).
    // If the fuile format does not support geocoding or if it has no geocoding or ApplyonAllFiles is set then
    // the presence of the PRJ sister file will result into loading the geocoding.
    if ((pi_rpForRasterFile->GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID)) &&
        (pi_rpForRasterFile->CountPages() <= 1) &&  // don't support the multipage
        (pi_ApplyonAllFiles || pi_rpForRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID) ||
         !(pi_rpForRasterFile->GetCapabilities()->HasCapabilityOfType(HRFGeocodingCapability::CLASS_ID, HFC_READ_ONLY)) ||
           (NULL == pi_rpForRasterFile->GetPageDescriptor(0)->GetGeocodingCP())) )
        {
        HFCPtr<HFCURL>  URLForPageFile = ComposeURLFor(pi_rpForRasterFile->GetURL());
        if (URLForPageFile != 0)
            {
            HFCStat PageFileStat(URLForPageFile);

            // Check if the decoration file exist and the time stamp.
            if (PageFileStat.IsExistent())
                HasPageFile = true;
            }
        }

    return HasPageFile;
    }

//-----------------------------------------------------------------------------
// public
// CreateFor
//-----------------------------------------------------------------------------
HFCPtr<HRFPageFile> HRFPRJPageFileCreator::CreateFor(const HFCPtr<HRFRasterFile>&  pi_rpForRasterFile) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    //Supported only in read-only
    HPRECONDITION(pi_rpForRasterFile->GetAccessMode().m_HasCreateAccess == false);

    // Multi-page not supported
    if (pi_rpForRasterFile->CountPages() > 1)
        throw HRFMultiPageNotSupportedException(pi_rpForRasterFile->GetURL()->GetURL());

    HFCPtr<HRFPageFile> pPageFile;

    pPageFile = new HRFPRJPageFile(ComposeURLFor(pi_rpForRasterFile->GetURL()),
                                   pi_rpForRasterFile->GetAccessMode());

    return pPageFile;
    }

//-----------------------------------------------------------------------------
// public
// ComposeURLFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFPRJPageFileCreator::ComposeURLFor(const HFCPtr<HFCURL>& pi_rpURLFileName) const
    {
    HPRECONDITION(pi_rpURLFileName != 0);

    HFCPtr<HFCURL> URLForPageFile;

    if (!pi_rpURLFileName->IsCompatibleWith(HFCURLFile::CLASS_ID))
        throw HFCInvalidUrlForSisterFileException(pi_rpURLFileName->GetURL());

    // Decompose the file name
    WString DriveDirName;

    // Extract the Path
    WString Path(((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetHost()+WString(L"\\")+((HFCPtr<HFCURLFile>&)pi_rpURLFileName)->GetPath());

    // Find the file extension
    WString::size_type DotPos = Path.rfind(L'.');

    // Extract the extension and the drive dir name
    if (DotPos != WString::npos)
        {
        // Compose the decoration file name
        DriveDirName = Path.substr(0, DotPos);
        URLForPageFile = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + DriveDirName + WString(L".prj"));
        }
    else
        URLForPageFile = new HFCURLFile(WString(HFCURLFile::s_SchemeName() + L"://") + Path + WString(L".prj"));

    return URLForPageFile;
    }

//-----------------------------------------------------------------------------
// public
// GetCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPRJPageFileCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFPRJCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Class HRFPRJPageFile
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFPRJPageFile::HRFPRJPageFile(const HFCPtr<HFCURL>&   pi_rpURL,
                               HFCAccessMode           pi_AccessMode)
    : HRFPageFile(pi_rpURL, pi_AccessMode)
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess);

    m_pFile = HFCBinStream::Instanciate(pi_rpURL, pi_AccessMode, 0, true);

    ReadFile();

    CreateDescriptor();
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFPRJPageFile::~HRFPRJPageFile()
    {
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPRJPageFile::GetCapabilities () const
    {
    return HRFPRJPageFileCreator::GetInstance()->GetCapabilities();
    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFPRJPageFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_HMRWORLD;
    }


void HRFPRJPageFile::WriteToDisk()
    {
    HASSERT(0); //Only supported in read-only mode

    }



//-----------------------------------------------------------------------------
// Private
// ReadFile
//-----------------------------------------------------------------------------
void HRFPRJPageFile::ReadFile()
    {
    HPRECONDITION(m_pFile != 0);

    HFCIniFileBrowser InitFile(m_pFile);

    string    currentString;
    char      currentChar;

    // Read the whole file
    while (!m_pFile->EndOfFile())
        {
        m_pFile->Read(&currentChar, 1);
        currentString += currentChar;
        }

    // Save WKT fragment
    m_WKT.AssignUtf8(currentString.c_str());

    }

//-----------------------------------------------------------------------------
// Private
// CreateDescriptor
//-----------------------------------------------------------------------------
void HRFPRJPageFile::CreateDescriptor()
    {
    HPMAttributeSet               TagList;
    HFCPtr<HPMGenericAttribute>   pTag;

    GeoCoordinates::BaseGCSPtr pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS();
    pBaseGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorESRI, m_WKT.c_str()); 

    HRFScanlineOrientation TransfoModelSLO = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

    // Create the Page information and add it to the list of page descriptor
    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor(GetAccessMode(),
                                  GetCapabilities(),    // Capabilities,
                                  0,                    // RepresentativePalette,
                                  0,                    // Histogram,
                                  0,                    // Thumbnail,
                                  0,                    // ClipShape,
                                  0,                    // TransfoModel,
                                  &TransfoModelSLO,     // TransfoModelOrientation
                                  0,                    // Filters
                                  &TagList);            // Tag


    pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pBaseGCS.get()));

    m_ListOfPageDescriptor.push_back(pPage);
    }

