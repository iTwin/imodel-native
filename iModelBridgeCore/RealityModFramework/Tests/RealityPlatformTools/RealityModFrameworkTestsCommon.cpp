/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

//__BENTLEY_INTERNAL_ONLY__
#include <Bentley/BeTextFile.h>

//#include "RealityModFrameworkTestsCommon.h"
#include "../Common/RealityModFrameworkTestsCommon.h"

WString RealityModFrameworkTestsUtils::s_exePath = L"";

ErrorClass* ErrorClass::s_error = nullptr;

MockWSGRequest* MockWSGRequestFixture::s_mockWSGInstance = nullptr;

ErrorClass* MockRealityDataServiceFixture::s_errorClass = nullptr;
RealityDataService* MockRealityDataServiceFixture::s_realityDataService = nullptr;

ErrorClass* MockGeoCoordinationServiceFixture::s_errorClass = nullptr;
GeoCoordinationService* MockGeoCoordinationServiceFixture::s_geoCoordinateService = nullptr;

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
Utf8String RealityModFrameworkTestsUtils::GetTestDataContent(WString filename)
    {
    auto path = GetTestDataPath(filename);

    BeFileStatus status;
    BeTextFilePtr jsonFile = BeTextFile::Open(status, path.GetWCharCP(), TextFileOpenType::Read, TextFileOptions::None, TextFileEncoding::Utf8);
    
    if( status == BeFileStatus::Success )
        {
        WString entireFile;
        WString buffer;
        while(jsonFile->GetLine(buffer) == TextFileReadStatus::Success )
            {
            entireFile.append(buffer);
            };

        Utf8String jsonString;
        BeStringUtilities::WCharToUtf8(jsonString, entireFile.GetWCharCP());

        return jsonString;
        }

    return Utf8String("");
    }

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
WString RealityModFrameworkTestsUtils::GetDirectory()
    {
    WChar exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);

    WString exeDir = exePath;
    size_t pos = exeDir.find_last_of(L"/\\");
    exeDir = exeDir.substr(0, pos + 1);
    return exeDir;
    }

//=====================================================================================
//! @bsimethod                                   Remi.Charbonneau              06/2017
//=====================================================================================
WString RealityModFrameworkTestsUtils::GetTestDataPath(WString filename)
    {
    if (s_exePath.empty())
        {
        s_exePath = GetDirectory();
        }

    auto path = s_exePath;
    path.append(filename);
    return path;
    }