/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#define BENTLEY_NON_PORTABLE
#if defined(BENTLEY_WIN32)
#include <windows.h>
#endif

#undef GetCurrentTime

#include "PerformanceTestFixture.h"
#include <Bentley/DateTime.h>

BEGIN_GEOMLIBS_TESTS_NAMESPACE

#if defined(BENTLEY_WIN32)
    void GetVersionInfoAsString(Utf8String& version)
    {
    TCHAR szVersionFile[MAX_PATH]; 
    GetModuleFileName(NULL, szVersionFile, MAX_PATH );

    DWORD  verHandle = NULL;
    UINT   size      = 0;
    LPBYTE lpBuffer  = NULL;
    DWORD  verSize   = GetFileVersionInfoSize( szVersionFile, &verHandle);

    if (verSize != NULL)
        {
        LPSTR verData = new char[verSize];

        if (GetFileVersionInfo( szVersionFile, verHandle, verSize, verData))
            {
            if (VerQueryValue(verData,"\\",(VOID FAR* FAR*)&lpBuffer,&size))
                {
                if (size)
                    {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)
                        {

                        // Doesn't matter if you are on 32 bit or 64 bit,
                        // DWORD is always 32 bits, so first two revision numbers
                        // come from dwFileVersionMS, last two come from dwFileVersionLS
                        version.Sprintf( "%d.%d.%d.%d",
                            ( verInfo->dwFileVersionMS >> 16 ) & 0xffff,
                            ( verInfo->dwFileVersionMS >>  0 ) & 0xffff,
                            ( verInfo->dwFileVersionLS >> 16 ) & 0xffff,
                            ( verInfo->dwFileVersionLS >>  0 ) & 0xffff
                            );
                        }
                    }
                }
            }
        delete[] verData;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PerformanceTestFixture::LogResultsToFile(bmap<Utf8String, double> results)
    {
    FILE* logFile=NULL;

    BeFileName dir;
    BeTest::GetHost().GetOutputRoot (dir);
    WCharCP processorArchitecture = (8 == sizeof(void*)) ? L"Winx64" : L"Winx86";
    dir.AppendToPath(processorArchitecture);
    dir.AppendToPath(L"TestResults");
    if (!dir.DoesPathExist())
        BeFileName::CreateNewDirectory (dir.c_str());

    dir.AppendToPath (L"GeomLibsPerformanceResults.csv");

    bool existingFile = dir.DoesPathExist();

    logFile = fopen(dir.GetNameUtf8().c_str(), "a+"); 
    PERFORMANCELOG.infov (L"CSV Results filename: %ls\n", dir.GetName());

    if (!existingFile)
        fprintf (logFile, "Date, Version Number, Test Description, Time (secs)\n");

    Utf8String versionNumber("9.99.99.99");
#if defined(BENTLEY_WIN32)
    GetVersionInfoAsString(versionNumber);
#endif

    Utf8String dateTime = DateTime::GetCurrentTime().ToString();
    FOR_EACH(T_TimerResultPair const& pair, results)
        {
        fprintf (logFile, "%s, %s, %s, %lf\n", dateTime.c_str(), versionNumber.c_str(), pair.first.c_str(), pair.second);
        }

    fclose (logFile);

    }


END_GEOMLIBS_TESTS_NAMESPACE
