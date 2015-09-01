//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFiTiffCacheFileCreator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFiTiffCacheFileCreator
//-----------------------------------------------------------------------------
// This class describes the CacheFile implementation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFLocalCacheFileCreator.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFCacheController.h>
#include <Imagepp/all/h/HRFBilFile.h>

// MakeDir and Access

static const WString s_cTiffExtensionCache = L".cache.cTIFF"; // ctiff extension
static const WString s_iTiffExtensionCache = L".cache.iTIFF"; // itiff extension

HFC_IMPLEMENT_SINGLETON(HRFiTiffCacheFileCreator)
//-----------------------------------------------------------------------------
// This is a helper class to instantiate an implementation object
// without knowing the different implementations.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFiTiffCacheFileCreator::HRFiTiffCacheFileCreator()
    : HRFLocalCacheFileCreator()
    {
    PostConstructor();
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFiTiffCacheFileCreator::~HRFiTiffCacheFileCreator()
    {
    }

//-----------------------------------------------------------------------------
// This methods allow to know if we have a cache file for the specified raster file.
//-----------------------------------------------------------------------------
bool HRFiTiffCacheFileCreator::HasCacheFor(const HFCPtr<HRFRasterFile>&    pi_rpForRasterFile,
                                            uint32_t                        pi_Page) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    HPRECONDITION(pi_Page == -1 || (pi_Page >= 0 && pi_Page < pi_rpForRasterFile->CountPages()));

    bool                       IsCacheFound = false;
    HFCPtr<HRFRasterFile>       pCacheFile;
    const HFCPtr<HRFRasterFile> pRasterFile = pi_rpForRasterFile;

    // get the file's modification time
    time_t FileModificationTime;
    pi_rpForRasterFile->GetFileStatistics(NULL, &FileModificationTime, NULL);

    // Get the Orignal file
    HFCPtr<HRFRasterFile> pOriginalFile = pRasterFile;
    if (pRasterFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        pOriginalFile = ((const HFCPtr<HRFRasterFileExtender>&)pRasterFile)->GetOriginalFile();

    IsCacheFound = HasCacheFor(pOriginalFile->GetURL(),
                               FileModificationTime,
                               pi_Page,
                               pOriginalFile->GetOffset());

    return IsCacheFound;
    }

//-----------------------------------------------------------------------------
// This methods allow to know if we have a cache file for the specified raster file URL.
//-----------------------------------------------------------------------------
bool HRFiTiffCacheFileCreator::HasCacheFor(const HFCPtr<HFCURL>& pi_rForRasterFileURL,
                                            uint32_t              pi_Page) const
    {
    time_t FileModificationTime;

    FileModificationTime = HFCStat(pi_rForRasterFileURL).GetModificationTime();

    return HasCacheFor(pi_rForRasterFileURL, FileModificationTime, pi_Page, 0);
    }

//-----------------------------------------------------------------------------
// This methods allow to know the cache file URL for the specified raster file.
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFiTiffCacheFileCreator::GetCacheURLFor(const HFCPtr<HRFRasterFile>&    pi_rpForRasterFile,
                                                        uint32_t                        pi_Page) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    HPRECONDITION(pi_Page == -1 || (pi_Page >= 0 && pi_Page < pi_rpForRasterFile->CountPages()));
    HPRECONDITION(HasCacheFor(pi_rpForRasterFile));

    HFCPtr<HRFRasterFile> pOriginalFile = pi_rpForRasterFile;

    // Get the Orignal file
    if (pi_rpForRasterFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        pOriginalFile = ((const HFCPtr<HRFRasterFileExtender>&)pi_rpForRasterFile)->GetOriginalFile();

    return GetCacheURLFor(pOriginalFile->GetURL(), pi_Page, pOriginalFile->GetOffset());
    }

//-----------------------------------------------------------------------------
// This methods allow to know the cache file URL for the specified raster file URL.
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFiTiffCacheFileCreator::GetCacheURLFor(const HFCPtr<HFCURL>& pi_rpForRasterFileURL,
                                                        uint32_t              pi_Page,
                                                        uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpForRasterFileURL != 0);
    //HPRECONDITION(HasCacheFor(pi_rpForRasterFileURL));

    return ComposeURLFor(pi_rpForRasterFileURL,
                         s_cTiffExtensionCache,
                         pi_Offset,
                         pi_Page);
    }

//-----------------------------------------------------------------------------
// This factory methods allow to instantiate the Cache file for the specified raster file.
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFiTiffCacheFileCreator::GetCacheFileFor(HFCPtr<HRFRasterFile>&  pi_rpForRasterFile,
                                                                uint32_t                pi_Page) const
    {
    HPRECONDITION(pi_rpForRasterFile != 0);
    HPRECONDITION(pi_Page == -1 || (pi_Page >= 0 && pi_Page < pi_rpForRasterFile->CountPages()));

    HFCPtr<HRFRasterFile> pCacheFile;
    HFCPtr<HRFRasterFile> pRasterFile = pi_rpForRasterFile;

    // Get the file's modification time
    time_t FileModificationTime;    // *** That information is only keep for back compatible between V8i, on Windows platform.
    time_t FileCreationTime;
    pi_rpForRasterFile->GetFileStatistics(&FileCreationTime, &FileModificationTime, NULL);

    // Get the Orignal file
    HFCPtr<HRFRasterFile> pOriginalFile = pRasterFile;
    if (pRasterFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        pOriginalFile = ((HFCPtr<HRFRasterFileExtender>&)pRasterFile)->GetOriginalFile();

    // If there are additional raster file URLs, cache is also dependant of these related files,
    ListOfRelatedURLs::const_iterator Itr;
    time_t TmpFileModificationTime;

    for (Itr = (pOriginalFile->GetRelatedURLs()).begin(); (Itr != (pOriginalFile->GetRelatedURLs()).end()); ++Itr)
        {
        HFCStat FileStatInfo(*Itr);
        // Take most recent file modification time
        TmpFileModificationTime = FileStatInfo.GetModificationTime();
        if (TmpFileModificationTime > FileModificationTime)
            FileModificationTime = TmpFileModificationTime;

        // The creationTime of the source file
        // *** That information is only keep for back compatible between V8i, on Windows platform.
        FileCreationTime = FileStatInfo.GetCreationTime();

        }

    // Compose the decoration file name
    HFCPtr<HFCURL> CacheURL;
    CacheURL = ComposeURLFor(pOriginalFile->GetURL(),
                             s_cTiffExtensionCache,
                             pOriginalFile->GetOffset(),
                             pi_Page);
    HFCStat CacheFileStat(CacheURL);
    bool   OpenNewFile = false;

    try
        {
        // The cache file with the correct name exists and has a valid file name
        if ((CacheFileStat.IsExistent()) &&
            IsModificationTimeValid(FileModificationTime, CacheFileStat.GetModificationTime()))
            {
            // ReOpen the cache
            pCacheFile = HRFcTiffCreator::GetInstance()->Create(CacheURL, HFC_READ_WRITE  | HFC_SHARE_READ_ONLY | HFC_SHARE_WRITE_ONLY);

            // verify if the cache is from MSI 1.0 or a later version.
            if (!HRFLocalCacheFileCreator::IsValidCache(pCacheFile))
                {
                pCacheFile = 0;
                OpenNewFile = true;
                }
            }
        else
            {
            // we didn't find a cache with cTiff extension,
            // check if we have one with iTiff extension (old cache)

            // Compose the decoration file name
            HFCPtr<HFCURL> OldCacheURL = ComposeURLFor(pOriginalFile->GetURL(),
                                                       s_iTiffExtensionCache,
                                                       pOriginalFile->GetOffset(),
                                                       pi_Page);

            HFCStat  OldCacheFileStat(OldCacheURL);

            // The cache file with the correct name exists and has a valid file name
            if ((OldCacheFileStat.IsExistent()) &&
                IsModificationTimeValid(FileModificationTime, OldCacheFileStat.GetModificationTime()))
                {
                // ReOpen the old extension cache
                pCacheFile = HRFcTiffCreator::GetInstance()->Create(OldCacheURL, HFC_READ_WRITE  | HFC_SHARE_READ_ONLY | HFC_SHARE_WRITE_ONLY);

                // verify if the cache is from MSI 1.0 or a later version.
                if (!HRFLocalCacheFileCreator::IsValidCache(pCacheFile))
                    {
                    pCacheFile = 0;
                    OpenNewFile = true;
                    }
                }
            // The file either does not exists, does not have a valid time stamp or as the old
            // name, so create (and possibly overwrite)
            else
                {
                OpenNewFile = true;
                }
            }
        }

    catch(HFCFileException& rException)
        {
        if ((dynamic_cast<HFCFileNotFoundException*>(&rException) == 0) &&
            (dynamic_cast<HFCFileNotSupportedException*>(&rException) == 0))
            throw;

        OpenNewFile = true;
        }

    // A new file must be created.  (Overwrites any existing file)
    if (OpenNewFile)
        {
        pCacheFile = HRFcTiffCreator::GetInstance()->Create(CacheURL, HFC_READ_WRITE_CREATE | HFC_SHARE_READ_ONLY | HFC_SHARE_WRITE_ONLY);
        ((HFCPtr<HRFcTiffFile>&)pCacheFile)->SetSourceFile_CreationDateTime(ctime(&FileCreationTime));
        }

    // control the size of the cache directory
    if (ImageppLib::GetHost().GetImageppLibAdmin()._GetDirectoryCacheSize() > 0)
        {
        HPRECONDITION(CacheURL->IsCompatibleWith(HFCURLFile::CLASS_ID));

        BeFileName localPath;
        ImageppLib::GetHost().GetImageppLibAdmin()._GetLocalCacheDirPath(localPath);

        HFCPtr<HFCURL> pUrl = HFCURL::CreateFrom(localPath);

        HRFCacheController CacheSize(pUrl);
        CacheSize.SetCacheSize(ImageppLib::GetHost().GetImageppLibAdmin()._GetDirectoryCacheSize());
        CacheSize.Control(HRFCacheController::CACHE_CONTROL_SIZE);
        }

    ((HFCPtr<HRFcTiffFile>&)pCacheFile)->SetOriginalFileAccessMode(pRasterFile->GetAccessMode());

    return pCacheFile;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFiTiffCacheFileCreator::GetCapabilities() const
    {
    return HRFcTiffCreator::GetInstance()->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Public
// ComposeURLFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFiTiffCacheFileCreator::ComposeURLFor(const HFCPtr<HFCURL>& pi_rpURLFileName,
                                                       const WString&        pi_Extension,
                                                       uint64_t             pi_Offset,
                                                       uint32_t              pi_Page) const
    {
    if (pi_Page != -1 && pi_Page != 0)
        {
        wostringstream Ext;
        Ext << L".page" << pi_Page << pi_Extension;

        return HRFLocalCacheFileCreator::ComposeURLFor(pi_rpURLFileName, Ext.str().c_str(), pi_Offset);
        }
    else
        return HRFLocalCacheFileCreator::ComposeURLFor(pi_rpURLFileName, pi_Extension, pi_Offset);
    }

//-----------------------------------------------------------------------------
// Public
// IsModificationTimeValid
//-----------------------------------------------------------------------------
bool HRFiTiffCacheFileCreator::IsModificationTimeValid(time_t pi_FileModificationTime,
                                                       time_t pi_CacheFileModificationTime) const
    {
    return ((pi_FileModificationTime - 3.0) <= pi_CacheFileModificationTime &&
            pi_CacheFileModificationTime <= (pi_FileModificationTime + 3.0));
    }

//-----------------------------------------------------------------------------
// Protected
// This methods allow to know if we have a cache file for the specified raster file URL.
//-----------------------------------------------------------------------------
bool HRFiTiffCacheFileCreator::HasCacheFor(const HFCPtr<HFCURL>& pi_rForRasterFileURL,
                                            time_t               pi_FileModificationTime,
                                            uint32_t             pi_Page,
                                            uint64_t             pi_Offset) const
    {
    bool HasCacheFor = false;
    // Compose the decoration file name
    HFCPtr<HFCURL> pCacheURL = ComposeURLFor(pi_rForRasterFileURL,
                                             s_cTiffExtensionCache,
                                             pi_Offset,
                                             pi_Page);

    HFCStat CacheFileStat(pCacheURL);
    // Check if the Cache file exist.
    if (CacheFileStat.IsExistent() &&
        IsModificationTimeValid(pi_FileModificationTime, CacheFileStat.GetModificationTime()))
        {
        // ReOpen the cache
        HasCacheFor = true;
        }
    else
        {
        // we didn't find a cache with cTiff extension,
        // check if we have one with iTiff extension (old cache)

        // Compose the decoration file name with iTiff
        pCacheURL = ComposeURLFor(pi_rForRasterFileURL,
                                  s_iTiffExtensionCache,
                                  pi_Offset,
                                  pi_Page);

        HFCStat OldCacheFileStat(pCacheURL);
        // Check if the Cache file exist.
        if (OldCacheFileStat.IsExistent() &&
            IsModificationTimeValid(pi_FileModificationTime, OldCacheFileStat.GetModificationTime()))
            {
            // ReOpen the cache
            HasCacheFor = true;
            }
        }

    return HasCacheFor;
    }