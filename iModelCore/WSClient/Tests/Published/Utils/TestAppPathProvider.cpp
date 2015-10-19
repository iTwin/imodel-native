/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/TestAppPathProvider.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestAppPathProvider.h"

USING_NAMESPACE_WSCLIENT_UNITTESTS

TestAppPathProvider::TestAppPathProvider()
    {
    BeTest::GetHost().GetDocumentsRoot(m_documentsDirectory);
    BeTest::GetHost().GetTempDir(m_temporaryDirectory);
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_platformAssetsDirectory);

    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);
    m_localStateDirectory = outputRoot;
    }
