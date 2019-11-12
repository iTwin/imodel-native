//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFLocalCacheFileCreator
//-----------------------------------------------------------------------------
// This class describes the CacheFile implementation
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRFLocalCacheFileCreator.h>
#include <ImagePP/all/h/HRFiTiffFile.h>
#include <ImagePP/all/h/HFCStat.h>
#include <ImagePP/all/h/HRFRasterFileCache.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HRFUtility.h>

//-----------------------------------------------------------------------------
// This is a helper class to instantiate an implementation object
// without knowing the different implementations.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFLocalCacheFileCreator::HRFLocalCacheFileCreator()
    :HRFCacheFileCreator()
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFLocalCacheFileCreator::~HRFLocalCacheFileCreator()
    {
    }


//-----------------------------------------------------------------------------
// Public
// GetTentativeURLFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFLocalCacheFileCreator::GetTentativeURLFor(
    const HFCPtr<HFCURL>& pi_rpURLFileName,
    const Utf8String&        pi_Extension,
    uint64_t             pi_Offset) const
    {
    return ComposeURLFor(pi_rpURLFileName, pi_Extension, pi_Offset);
    }


//-----------------------------------------------------------------------------
// public
// ComposeURLFor
//-----------------------------------------------------------------------------
HFCPtr<HFCURL> HRFLocalCacheFileCreator::ComposeURLFor(
    const HFCPtr<HFCURL>& pi_rpURLFileName,
    const Utf8String&        pi_Extension,
    uint64_t             pi_Offset) const
    {
    // Get the filename
    Utf8String ComposedFileName(ComposeFilenameFor(pi_rpURLFileName));

    // Add the offset to the file name
    if (pi_Offset != 0)
        {
        Utf8Char StrOffset[256];
        BeStringUtilities::Snprintf(StrOffset, "%ld", pi_Offset);

        ComposedFileName += StrOffset;
        }

    // Add the extension to the file name
    ComposedFileName += pi_Extension;

    // Add the path to the Booster url
    BeFileName localPath;
    ImageppLib::GetHost().GetImageppLibAdmin()._GetLocalCacheDirPath(localPath);
    WString ComposedFileNameW(ComposedFileName.c_str(), BentleyCharEncoding::Utf8);
    localPath.AppendToPath(ComposedFileNameW.c_str());

    return HFCURL::CreateFrom(localPath);
    }

//-----------------------------------------------------------------------------
// Protected
// ComposeFilenameFor
// Compose a Booster FileName with the URL from source file
//-----------------------------------------------------------------------------
Utf8String HRFLocalCacheFileCreator::ComposeFilenameFor(const HFCPtr<HFCURL>& pi_rpURLFileName) const
    {
    Utf8String ComposedName(pi_rpURLFileName->GetURL());

    // Compose the file name for the specified URL
    Utf8String  Seps("\\/:*?\"<>|");
    size_t   Pos = 0;
    while ((Pos = ComposedName.find_first_of (Seps, Pos)) != Utf8String::npos)
        {
        ComposedName[Pos] = '_';
        Pos++;
        }

    return ComposedName;
    }




//-----------------------------------------------------------------------------
// Private
// Sets the tags that identify a cache file
//-----------------------------------------------------------------------------
void HRFLocalCacheFileCreator::SetCacheTags(HFCPtr<HRFRasterFile>& pi_rpFile)
    {
    HPRECONDITION(pi_rpFile != 0);

    for (uint32_t Page = 0; Page < pi_rpFile->CountPages(); Page++)
        {
        HFCPtr<HRFPageDescriptor> pPage(pi_rpFile->GetPageDescriptor(Page));

        if (!pPage->IsEmpty())
            {
            ImageppLibAdmin::CompatibleSoftware softwareName;
            ImageppLib::GetHost().GetImageppLibAdmin()._GetCacheCompatibleSoftwareNames(softwareName);
            if (!softwareName.empty())
                pPage->SetTag(new HRFAttributeSoftware(*softwareName.begin()));
                
            Utf8String description;
            ImageppLib::GetHost().GetImageppLibAdmin()._GetCacheDescription(description);
            if (!description.empty())
                pPage->SetTag(new HRFAttributeDocumentName(description));
}
        }
    }



//-----------------------------------------------------------------------------
// Private
// Indicates if the cache is valid
//-----------------------------------------------------------------------------
bool HRFLocalCacheFileCreator::IsValidCache(const HFCPtr<HRFRasterFile>& pi_rpFile)
    {
    HPRECONDITION(pi_rpFile != 0);
    HPRECONDITION(pi_rpFile->CountPages() > 0);
    bool Result = true;
    bool Is32bitsWithAlpha = false;

    // verify if the false has a 32-bit with alpha pixel type,
    for (uint32_t Page = 0; Page < pi_rpFile->CountPages(); Page++)
        {
        HFCPtr<HRFPageDescriptor> pPage(pi_rpFile->GetPageDescriptor(0));

        for (uint32_t Res = 0; (!Is32bitsWithAlpha) && (Res < pPage->CountResolutions()); Res++)
            {
            HFCPtr<HRPPixelType> pPixel(pPage->GetResolutionDescriptor((uint16_t)Res)->GetPixelType());

            // if it is a 32-bit type with an alpha channel, this source cannot handle it, so
            // use a 24-bit pixel type.
            Is32bitsWithAlpha = ((pPixel->CountIndexBits() != 0) &&
                                 (pPixel->CountPixelRawDataBits() == 32) &&
                                 (pPixel->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE));
            }
        }

    // if the pixel type is not 32-bit with alpha, the cache is valid. Otherwise,
    // we must verify if it is an older cache and decide whether or not to invalidate it.
    if (Is32bitsWithAlpha)
        {
        // Get the descriptor of page 0
        HFCPtr<HRFPageDescriptor> pPage(pi_rpFile->GetPageDescriptor(0));

        // verify if the software name was set on the extender (this)
        ImageppLibAdmin::CompatibleSoftware softwareName;
        ImageppLib::GetHost().GetImageppLibAdmin()._GetCacheCompatibleSoftwareNames(softwareName);

        if (Result && !softwareName.empty())
            {
            // The software tag is set for this application, we can now assume
            // false as a result, unless the file as the same data.
            Result = false;

            // Get the tag from the file
            HRFAttributeSoftware const* pFileSoftware(pPage->FindTagCP<HRFAttributeSoftware>());

            // verify that the software tag is in the file
            if (pFileSoftware != 0) 
                {
                // verify if one of the compatible software is teh one
                for (ImageppLibAdmin::CompatibleSoftware::const_iterator Itr = softwareName.begin();
                     (!Result) && (Itr != softwareName.end());
                     Itr++)
                    {
                    Result = (pFileSoftware->GetData() == Itr->c_str());
                    }
                }
            }

        // verify if the document name was set on the extender (this)
        Utf8String description;
        ImageppLib::GetHost().GetImageppLibAdmin()._GetCacheDescription(description);
        if (Result && !description.empty())
            {
            // The document name tag is set for this application, we can now assume
            // false as a result, unless the file as the same data.
            Result = false;

            // verify that the document name tag is in the file
            HRFAttributeDocumentName const* pFileDocument(pPage->FindTagCP<HRFAttributeDocumentName>());
            if (pFileDocument !=0) 
                {
                // compare with the wanted software
                Result = (pFileDocument->GetData() == description);
                }
            }
        }

    return (Result);
    }
