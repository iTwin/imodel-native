//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/FileFormat/ExporterTestFixture.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExportTester
//-----------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <Bentley/MD5.h>
#include <ImagePP/h/ImageppAPI.h>
#include <ImagePP/all/h/HRFFileFormats.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HUTImportFromFileExportToFile.h>
#include <ImagePP/all/h/HUTImportFromRasterExportToFile.h>
#include "../Common/ImagePPTestConfig.h"
#include <json/json.h>
#include "ExporterTestFixture.h"

//#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include <Imagepp/all/h/HUTClassIDDescriptor.h>
#include <Bentley/BeFileName.h>
#include <Imagepp/all/h/HPMPool.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HCDCodecZlib.h>

USING_NAMESPACE_IMAGEPP

//----------------------------------------------------------------------------------------
//                              RasterTestInfo
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
RasterTestInfo::RasterTestInfo(BeFileNameCR rasterPath) 
    :m_rasterPath(rasterPath)
    {
    m_infoPath = rasterPath;
    m_infoPath.AppendExtension(L"testInfo");

#ifdef NDEBUG
    m_buildType = "Release";
#else
    m_buildType = "Debug";
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
bool RasterTestInfo::Load()
    {
    BeFile file;
    if (BeFileStatus::Success != file.Open(m_infoPath, BeFileAccess::Read))
        return false;

    ByteStream infoStream;
    if (BeFileStatus::Success != file.ReadEntireFile(infoStream))
        return false;

    Utf8String infoString((Utf8CP) infoStream.GetData(), infoStream.GetSize());

    Json::Value value;
    if (!Json::Reader::Parse(infoString, value))
        return false;

    m_buildType = value["BuildType"].asString();
    m_exportDuration = value["DurationMs"].asUInt64();
    m_md5 = value["MD5"].asString();

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
bool RasterTestInfo::Store()
    {
    Json::Value value;
    value["BuildType"] = m_buildType;
    value["DurationMs"] = m_exportDuration;
    value["MD5"] = m_md5.c_str();

    Utf8StringAlias infoString = Json::FastWriter::ToString(value);

    BeFile file;
    if (BeFileStatus::Success != file.Create(m_infoPath, true))
        return false;

    if (BeFileStatus::Success != file.Write(nullptr, infoString.c_str(), (uint32_t) infoString.SizeInBytes()))
        return false;

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
bool RasterTestInfo::ComputeMD5()
    {
    BeFile rasterFile;
    if (BeFileStatus::Success != rasterFile.Open(m_rasterPath, BeFileAccess::Read))
        return false;

    static const size_t s_readChunk = 1024 * 1024;
    std::unique_ptr<Byte[]> pBuffer(new Byte[s_readChunk]);

    uint64_t fileSize = 0;
    if (BeFileStatus::Success != rasterFile.GetSize(fileSize))
        return false;

    MD5 checksum;
    for (uint64_t totalBytesRead = 0; totalBytesRead < fileSize; )
        {
        uint32_t bytesRead = 0;
        if (BeFileStatus::Success != rasterFile.Read(pBuffer.get(), &bytesRead, s_readChunk))
            return false;

        totalBytesRead += bytesRead;

        checksum.Add(pBuffer.get(), bytesRead);
        }

    m_md5 = checksum.GetHashString();

    return true;
    }

//----------------------------------------------------------------------------------------
//                              Utils
//----------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
@description  Depending on the image format of the input file, it makes required
*               modifications for known issues with certain formats.
*
* see ..\HugeDataTest\src\Apps\ImagePP\Wrapper\TestFile.cpp
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//&&MM to remove and replace by an admin methods to write a dummy dates for test
//      There is more then tiff. lrd, tga, ctiff...
bool UpdateTiffHistogramTimestamp(HFCPtr<HFCURL> const& pUrl)
    {
    if (!pUrl->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    HAutoPtr<HTIFFFile> pTiffFile(new HTIFFFile(static_cast<HFCURLFile*>(pUrl.GetPtr())->GetAbsoluteFileName(), HFC_READ_WRITE));

    HTIFFError* pErr = NULL;
    if (!pTiffFile->IsValid(&pErr) || (pErr != NULL && pErr->IsFatal()))
        return false;

    AString value("9999:99:99 99:99:99");

    //Standard Directories
    unsigned int standardDirectoryNb = pTiffFile->NumberOfDirectory(HTIFFFile::STANDARD);

    bool NeedSave = false;

    for (unsigned int i = 0; standardDirectoryNb > i; ++i)
        {
        pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, i));

        if (pTiffFile->TagIsPresent(ImagePP::DATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::DATETIME, value.c_str());
            NeedSave = true;
            }

        if (pTiffFile->TagIsPresent(ImagePP::HMR_HISTOGRAMDATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::HMR_HISTOGRAMDATETIME, value.c_str());
            NeedSave = true;
            }
        }

    //HMR directories
    unsigned int hmrDirectoryNb = pTiffFile->NumberOfDirectory(HTIFFFile::HMR);

    for (unsigned int i = 0; hmrDirectoryNb > i; ++i)
        {
        pTiffFile->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, i));

        if (pTiffFile->TagIsPresent(ImagePP::DATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::DATETIME, value.c_str());
            NeedSave = true;
            }

        if (pTiffFile->TagIsPresent(ImagePP::HMR_HISTOGRAMDATETIME))
            {
            pTiffFile->SetFieldA(ImagePP::HMR_HISTOGRAMDATETIME, value.c_str());
            NeedSave = true;
            }
        }

    if (NeedSave)
        pTiffFile->Save();

    pTiffFile = 0;

    return true;
    }

