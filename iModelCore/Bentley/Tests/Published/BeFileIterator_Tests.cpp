/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeFileIterator_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/BeDirectoryIterator.h>

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson  05/2013
//---------------------------------------------------------------------------------------
#ifdef WIP_NEEDS_WORK // *** Bentley unit tests must not depend on BeTestDocuments
#endif
TEST (BeFileListIterator, Test1)
    {
    BeFileName docs;
    BeTest::GetHost().GetDocumentsRoot (docs);
    BeFileName wildcard (NULL, docs, L"*", NULL);
    BeFileListIterator it (wildcard, /*recursive*/true);
    BeFileName name;
    size_t count=0;
    bool found2dMetric = false;
    bool found3dMetric = false;
    while (it.GetNextFileName (name) == SUCCESS)
        {
        BeFileName dir (BeFileName::DevAndDir, name);
        ASSERT_TRUE( 0 == BeStringUtilities::Wcsnicmp (docs, dir, wcslen(docs)) );
        BeFileName base (BeFileName::Basename, name);
        if (0==BeStringUtilities::Wcsicmp (base, L"2dMetricGeneral"))
            found2dMetric = true;
        else if (0==BeStringUtilities::Wcsicmp (base, L"3dMetricGeneral"))
            found3dMetric = true;
        ++count;
        }
    ASSERT_TRUE( count > 2 );
    ASSERT_TRUE( found2dMetric );
    ASSERT_TRUE( found3dMetric );
    }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson  05/2013
//---------------------------------------------------------------------------------------
static size_t countV8InDir (BeFileNameCR dir)
    {
    size_t count = 0;
    BeFileName entryName;
    bool isDir;
    for (BeDirectoryIterator dirs (dir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
        {
        if (isDir)
            count += countV8InDir (entryName);
        else
            {
            if (BeFileName::GetExtension(entryName).EqualsI (L"v8"))
                ++count;
            }
        }
    return count;
    }

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson  05/2013
//---------------------------------------------------------------------------------------
#ifdef WIP_NEEDS_WORK // *** Bentley unit tests must not depend on BeTestDocuments
#endif
TEST (BeDirectoryIterator, Test1)
    {
    BeFileName docs;
    BeTest::GetHost().GetDocumentsRoot (docs);

    //  Count the number of files ending in ".v8" by iterating recursively through the doc directory tree.
    size_t countv8 = countV8InDir (docs);
    ASSERT_TRUE( countv8 > 2 );

    //  Verify that the WalkDirsAndMatch utility gets the same count.
    bvector<BeFileName> found;
    BeDirectoryIterator::WalkDirsAndMatch (found, docs, L"*.v8", /*recursive*/true);
    ASSERT_EQ( found.size(), countv8 );
    }
