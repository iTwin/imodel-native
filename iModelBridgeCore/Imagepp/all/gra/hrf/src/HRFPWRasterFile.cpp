//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPWRasterFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFPWRasterFile.h>
#include <Imagepp/all/h/HRFPWEditor.h>

#if defined(IPP_HAVE_PROJECTWISE_SUPPORT) 

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/interface/IHRFPWFileHandler.h>

//-----------------------------------------------------------------------------
// class HRFPWCapabilities
//-----------------------------------------------------------------------------
HRFPWCapabilities::HRFPWCapabilities()
    : HRFcTiffCapabilities()
    {
    }

//-----------------------------------------------------------------------------
// class HRFPWRasterFile
//-----------------------------------------------------------------------------

HRFPWRasterFile::HRFPWRasterFile(const HFCPtr<HFCURL>&  pi_rpURL,
                                 HFCAccessMode          pi_AccessMode,
                                 uint64_t              pi_Offset)
    : HRFcTiffFile(pi_rpURL, pi_AccessMode, pi_Offset + sizeof(HRFPWRasterFile::Header)),
      m_pPWHandler(0)
    {
    uint32_t BlobSize;
    ReadProjectWiseBlob(0, (Byte*)&m_OriginalFileInfo, &BlobSize);
    }

HRFPWRasterFile::~HRFPWRasterFile()
    {
    }

HFCPtr<HRFRasterFile> HRFPWRasterFile::Create(const HFCPtr<HFCURL>&  pi_rpURL,
                                              GUID                   pi_DocumentID,
                                              const HFCPtr<HFCURL>&  pi_rpPWUrl)
    {
    HPRECONDITION(pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID));

    HFCPtr<HRFRasterFile> pTrueRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(pi_rpURL, HFC_SHARE_READ_ONLY);
    if (pTrueRasterFile != 0)
        {
        return Create(pTrueRasterFile, pi_DocumentID, pi_rpPWUrl);
        }
    else
        return 0;
    }

HFCPtr<HRFRasterFile> HRFPWRasterFile::Create(const HFCPtr<HRFRasterFile>&  pi_rpRasterFile,
                                              GUID                          pi_DocumentID,
                                              const HFCPtr<HFCURL>&         pi_rpPWUrl)
    {
    HPRECONDITION(pi_rpRasterFile != 0);

    // multipage is not supported yet
    if (pi_rpRasterFile->CountPages() > 1 ||
        pi_rpRasterFile->GetPageDescriptor(0)->IsUnlimitedResolution() ||
        pi_rpRasterFile->IsCompatibleWith(HRFFileId_WMS/*HRFWMSFile::CLASS_ID*/) ||
        pi_rpRasterFile->IsCompatibleWith(HRFFileId_GeoRaster/*HRFGeoRasterFile::CLASS_ID*/))
        return 0;

    HFCPtr<HRFPWRasterFile> pPWFile;
    HFCPtr<HRFRasterFile> pCacheFile = GenericImprove(pi_rpRasterFile, HRFiTiffCacheFileCreator::GetInstance(), false, false);

    HFCPtr<HRFPageDescriptor> pPage = pCacheFile->GetPageDescriptor(0);
    pCacheFile = 0;

    HFCPtr<HRFRasterFileCapabilities> pPWCapabilities(new HRFPWCapabilities());

    HFCPtr<HRFResolutionDescriptor> pSrcResDesc;
    HFCPtr<HRFResolutionDescriptor> pNewResDesc;
    HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;

    bool Supported = true;
    for (unsigned short Res = 0; Res < pPage->CountResolutions() && Supported; Res++)
        {
        pSrcResDesc = pPage->GetResolutionDescriptor(Res);
        // create a new page with HRFPWCapabilities
        if (pSrcResDesc->CanCreateWith(HFC_WRITE_AND_CREATE, pPWCapabilities))
            {
            pNewResDesc =
                new HRFResolutionDescriptor(HFC_CREATE_ONLY,                        // AccessMode
                                            pPWCapabilities,                        // Capabilities,
                                            pSrcResDesc->GetResolutionXRatio(),     // XResolutionXRatio,
                                            pSrcResDesc->GetResolutionYRatio(),     // YResolutionYRatio,
                                            pSrcResDesc->GetPixelType(),            // PixelType,
                                            pSrcResDesc->GetCodec(),                // CodecsList,
                                            pSrcResDesc->GetReaderBlockAccess(),    // RStorageAccess,
                                            pSrcResDesc->GetWriterBlockAccess(),    // WStorageAccess,
                                            pSrcResDesc->GetScanlineOrientation(),  // ScanLineOrientation,
                                            pSrcResDesc->GetInterleaveType(),       // InterleaveType
                                            pSrcResDesc->IsInterlace(),             // IsInterlace,
                                            pSrcResDesc->GetWidth(),                // Width,
                                            pSrcResDesc->GetHeight(),               // Height,
                                            pSrcResDesc->GetBlockWidth(),           // BlockWidth,
                                            pSrcResDesc->GetBlockHeight(),          // BlockHeight,
                                            (pSrcResDesc->HasBlocksDataFlag()? pSrcResDesc->GetBlocksDataFlag() : 0),       // BlocksDataFlag
                                            pSrcResDesc->GetBlockType(),            // Storage Type
                                            pSrcResDesc->GetNumberOfPass(),         // NumberOfPass
                                            pSrcResDesc->GetPaddingBits(),          // PaddingBits
                                            pSrcResDesc->GetDownSamplingMethod());  // DownSamplingMethod

            ListOfResolutionDescriptor.push_back(pNewResDesc);
            }
        else
            Supported = false;
        }

    if (Supported)
        {
        // create a page without tags
        HFCPtr<HRFPageDescriptor> pNewPage =
            new HRFPageDescriptor(pPage->GetAccessMode(),
                                  pPage->GetCapabilities(),
                                  ListOfResolutionDescriptor,
                                  (pPage->HasRepresentativePalette()? &pPage->GetRepresentativePalette() : 0),
                                  (pPage->HasHistogram()? pPage->GetHistogram() : 0),
                                  (pPage->HasThumbnail()? pPage->GetThumbnail() : 0),
                                  (pPage->HasClipShape()? pPage->GetClipShape() : 0),
                                  (pPage->HasTransfoModel()? pPage->GetTransfoModel() : 0),
                                  (pPage->HasFilter()? &pPage->GetFilter() : 0),
                                  0,
                                  pPage->GetDuration(),
                                  pPage->IsUnlimitedResolution());

        if (pNewPage->CanCreateWith(HFC_CREATE_ONLY, pPWCapabilities, ListOfResolutionDescriptor))
            {
            pNewPage = new HRFPageDescriptor(HFC_CREATE_ONLY,
                                             pPWCapabilities,
                                             ListOfResolutionDescriptor,
                                             (pPage->HasRepresentativePalette()? &pPage->GetRepresentativePalette() : 0),
                                             (pPage->HasHistogram()? pPage->GetHistogram() : 0),
                                             (pPage->HasThumbnail()? pPage->GetThumbnail() : 0),
                                             (pPage->HasClipShape()? pPage->GetClipShape() : 0),
                                             (pPage->HasTransfoModel()? pPage->GetTransfoModel() : 0),
                                             (pPage->HasFilter()? &pPage->GetFilter() : 0),
                                             0,
                                             pPage->GetDuration(),
                                             pPage->IsUnlimitedResolution());
            pPWFile = new HRFPWRasterFile(pi_rpPWUrl, HFC_CREATE_ONLY);
            pPWFile->AddPage(pNewPage);

            pPWFile = 0;

            // add the PW header to the ctiff

            uint32_t Buffer1Size = (uint32_t)sizeof(HRFPWRasterFile::Header);
            HArrayAutoPtr<Byte> pBuffer1(new Byte[Buffer1Size]);

            time_t FileTimestamps = HFCStat(pi_rpRasterFile->GetURL()).GetModificationTime();
            HASSERT_X64(FileTimestamps < ULONG_MAX);  // see the cast below
            __time32_t FileTimestamps32 = (__time32_t)FileTimestamps;

            // create the header
            strncpy((*(Header*)pBuffer1.get()).FileIdentification, "Bentley ProjectWise Raster File", 31);
            (*(Header*)pBuffer1.get()).DocumentTimestamps = FileTimestamps32;

            uint32_t Buffer2Size = 1024;
            HArrayAutoPtr<Byte> pBuffer2(new Byte[Buffer2Size]);

            HAutoPtr<HFCBinStream> pFile(HFCBinStream::Instanciate(pi_rpPWUrl, HFC_READ_WRITE, 0, true));

            uint64_t FileSize = pFile->GetSize();
            FileSize += Buffer1Size;

            size_t Count = pFile->Read(pBuffer2, Buffer2Size);
            pFile->SeekToPos(0);
            pFile->Write(pBuffer1, Buffer1Size); // write header
            uint64_t FilePos;
            while (Count > 0)
                {
                if (Count < Buffer2Size)
                    {
                    pFile->Write(pBuffer2, Count);
                    Count = 0;
                    }
                else
                    {
                    pFile->Write(pBuffer2, (Buffer2Size - Buffer1Size));
                    memcpy(pBuffer1, &pBuffer2[Buffer2Size - Buffer1Size], Buffer1Size);
                    FilePos = pFile->GetCurrentPos();
                    Count = pFile->Read(pBuffer2, Buffer2Size);
                    pFile->SeekToPos(FilePos);
                    pFile->Write(pBuffer1, Buffer1Size);
                    }
                }
            pFile = 0;  // close the file
            pBuffer1 = 0;
            pBuffer2 = 0;

            pPWFile = new HRFPWRasterFile(pi_rpPWUrl, HFC_READ_WRITE);

            OriginalFileInfo FileInfo;
            FileInfo.ClassID    = pi_rpRasterFile->GetClassID();
            FileInfo.WorldID    = pi_rpRasterFile->GetWorldIdentificator();
            FileInfo.DocumentID = pi_DocumentID;
            FileInfo.Timestamp  = FileTimestamps32;

            pPWFile->WritePrivateDirectory(0);
            pPWFile->WriteProjectWiseBlob(0, (Byte*)&FileInfo, sizeof(FileInfo));

            pCacheFile = 0;
            }
        }

    return (HFCPtr<HRFRasterFile>&)pPWFile;
    }

// File manipulation
bool HRFPWRasterFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    return HRFcTiffFile::AddPage(pi_pPage);
    }

HRFResolutionEditor* HRFPWRasterFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                             unsigned short pi_Resolution,
                                                             HFCAccessMode  pi_AccessMode)
    {
    m_pPWHandler = (m_pPWHandler == 0 ? HRFRasterFileFactory::GetInstance()->GetPWHandler() : m_pPWHandler);

    if (m_pPWHandler != 0)
        return new HRFPWEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        throw HRFPWNoHandlerException(GetURL()->GetURL());
    }



bool HRFPWRasterFile::ReadProjectWiseBlob(uint32_t pi_Page, Byte* po_pData, uint32_t* po_pSize) const
    {
    return GetFilePtr()->ReadProjectWiseBlob(pi_Page, po_pData, po_pSize);
    }

bool HRFPWRasterFile::WriteProjectWiseBlob(uint32_t pi_Page, const Byte* pi_pData, uint32_t pi_Size)
    {
    return GetFilePtr()->WriteProjectWiseBlob(pi_Page, pi_pData, pi_Size);
    }



//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------

void HRFPWRasterFile::InitPrivateTagDefault(HRFiTiffFile::HMRHeader* po_pHMRHeader)
    {
    HPRECONDITION(po_pHMRHeader != 0);

    // call the ancestor
    HRFiTiffFile::InitPrivateTagDefault(po_pHMRHeader);

    po_pHMRHeader->m_Version       = CTIFF_PW_VERSION;
    po_pHMRHeader->m_MinorVersion  = 0;

    }



//-----------------------------------------------------------------------------
// class HRFPWCreator
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_SINGLETON(HRFPWCreator)

HRFPWCreator::HRFPWCreator()
    : HRFcTiffCreator()
    {
    m_ClassID = HRFPWRasterFile::CLASS_ID;
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
//
// Same code as HRFcTiffFile
//-----------------------------------------------------------------------------
bool HRFPWCreator::IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                 uint64_t                pi_Offset) const
    {
    HTIFFFile*  pTiff;
    bool       bResult;

    HPRECONDITION(pi_rpURL != 0);


    // try to open the TIFF file, if it cannot be opened,
    // it is not a TIFF, so set the result to false
    HTIFFError* pErr;

    (const_cast<HRFPWCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pTiff = new HTIFFFile (pi_rpURL, pi_Offset + sizeof(HRFPWRasterFile::Header), HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if ((pTiff->IsValid(&pErr)) || ((pErr != 0) && !pErr->IsFatal()))
        {
        bResult = true;

        // validate each pages
        uint32_t PageCount = pTiff->NumberOfPages();
        if (PageCount == 0)
            {
            bResult = ValidatePageDirectory(pTiff, 0, CTIFF_PW_VERSION);
            }
        else
            {
            for (uint32_t Page = 0; Page < PageCount && bResult; Page++)
                bResult = ValidatePageDirectory(pTiff, Page, CTIFF_PW_VERSION);
            }
        }
    else
        bResult = false;

    // close the file
    delete pTiff;

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFPWCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFPWCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }


// Identification information
WString HRFPWCreator::GetLabel() const
    {
    return WString(L"ProjectWise File Format");
    }

WString HRFPWCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

WString HRFPWCreator::GetExtensions() const
    {
    return WString(L"*.pw");
    }


// allow to Open an image file READ/WRITE and CREATE
HFCPtr<HRFRasterFile> HRFPWCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset) const
    {

    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFPWRasterFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// public
// GetOriginalClassID
//
// Return the ClassID of the server raster file
//-----------------------------------------------------------------------------
HCLASS_ID HRFPWRasterFile::GetOriginalClassID() const
    {
    return m_OriginalFileInfo.ClassID;
    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
//
// Return the World ID of the server raster file
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFPWRasterFile::GetWorldIdentificator () const
    {
    return (m_OriginalFileInfo.WorldID);
    }

#endif  // IPP_HAVE_PROJECTWISE_SUPPORT