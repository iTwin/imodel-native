//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : ExportTester
//-----------------------------------------------------------------------------

#include <Bentley/BeTest.h>
#include <Bentley/md5.h>
#include <ImagePP/h/ImageppAPI.h>
#include <ImagePP/all/h/HRFFileFormats.h>
#include <ImagePP/all/h/HGFHMRStdWorldCluster.h>
#include <ImagePP/all/h/HUTImportFromFileExportToFile.h>
#include <ImagePP/all/h/HUTImportFromRasterExportToFile.h>
#include "../Common/ImagePPTestConfig.h"
#include <json/json.h>
#include "ExporterTestFixture.h"

//#ifdef USE_GTEST        // TEST_P only available when using gtest.

#include <ImagePP/all/h/HUTClassIDDescriptor.h>
#include <Bentley/BeFileName.h>
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HCDCodecIJG.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HCDCodecZlib.h>

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

// Probably not valid on non windows platforms.
#ifndef BENTLEY_WINRT
    m_computerName = getenv("COMPUTERNAME");
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
Utf8String RasterTestInfo::GetImagePPSourcePath() const
    {
    Utf8String sourcePath = __FILE__;
    sourcePath.ToLower();
    auto result = sourcePath.find("\\imagepp\\");
    BeAssert(std::string::npos != result);
    if (std::string::npos != result)
        sourcePath.resize(result + strlen("\\imagepp\\"));

    return sourcePath;
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

    // Run info
    auto& vRun = value["Run"];
    // runDate = vRun["Date"].asString();
    m_computerName = vRun["ComputerName"].asString();
    m_exportDuration = vRun["DurationMs"].asUInt64();
    m_md5 = vRun["MD5"].asString();

    // Build info
    auto& vBuild = value["Build"];
    // buildDate = vBuild["Date"].asString();
    m_buildType = vBuild["Type"].asString();
    // sourcePath = vBuild["Source"].asString();

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  6/2016
//----------------------------------------------------------------------------------------
bool RasterTestInfo::Store()
    {
    time_t time = (time_t) (BeTimeUtilities::GetCurrentTimeAsUnixMillis() / 1000.0); time;  // Convert in second   
    Utf8String timeString = ctime(&time);
    timeString.Trim();

    Json::Value value;

    // Run info
    auto& vRun = value["Run"];
    vRun["Date"] = timeString;
    vRun["ComputerName"] = m_computerName;
    vRun["DurationMs"] = m_exportDuration;
    vRun["MD5"] = m_md5.c_str();

    // Build information
    auto& vBuild = value["Build"];
    vBuild["Date"] = __DATE__ " " __TIME__;
    vBuild["Type"] = m_buildType;
    vBuild["Source"] = GetImagePPSourcePath();


    Utf8String infoString(Json::FastWriter::ToString(value));

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

