/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/FSTest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "TestsHelper.h"

BEGIN_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE

//--------------------------------------------------------------------------------------+
// @bsiclass                                                     Vincas.Razma    08/2014
// Class for testing against File System
//---------------+---------------+---------------+---------------+---------------+------+
class FSTest
    {
    public:
        static BeFileName GetAssetsDir ();
        static BeFileName GetTempDir ();
        static BeFileName StubFilePath (Utf8StringCR customFileName = "");
        static BeFileName StubFile (Utf8StringCR content = "TestContent", Utf8StringCR customFileName = "");
        static Utf8String ReadFile (BeFileNameCR filePath);
        static void WriteToFile (Utf8StringCR content, BeFileNameCR filePath);
    };

END_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE
