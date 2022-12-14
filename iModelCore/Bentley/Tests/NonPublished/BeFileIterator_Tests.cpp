/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/BeDirectoryIterator.h>

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void createFiles(BeFileNameCR root)
    {
    if (!root.DoesPathExist())
        BeFileName::CreateNewDirectory(root.c_str());

    BeFileName child1, child2, child3, child31;
    child1 = child2 = child3 = root;
    child1.AppendToPath(L"Child1");
    child2.AppendToPath(L"Child2");
    child3.AppendToPath(L"Child3");
    child31 = child3;
    child31.AppendToPath(L"Child31");

    if (!child1.DoesPathExist())
        BeFileName::CreateNewDirectory(child1.c_str());
    if (!child2.DoesPathExist())
        BeFileName::CreateNewDirectory(child2.c_str());
    if (!child3.DoesPathExist())
        BeFileName::CreateNewDirectory(child3.c_str());
    if (!child31.DoesPathExist())
        BeFileName::CreateNewDirectory(child31.c_str());


    BeFile file0, file1, file2, file3, file31;
    BeFileName fileName0, fileName1, fileName2, fileName3, fileName31;

    fileName0 = root;
    fileName0.AppendToPath(L"File0.txt");
    EXPECT_EQ(BeFileStatus::Success, file0.Create(fileName0, true));
    fileName1 = child1;
    fileName1.AppendToPath(L"File1.log");
    EXPECT_EQ(BeFileStatus::Success, file1.Create(fileName1, true));
    fileName2 = child2;
    fileName2.AppendToPath(L"File2.txt");
    file2.Create(fileName2, true);
    fileName3 = child3;
    fileName3.AppendToPath(L"File3.log");
    file3.Create(fileName3, true);
    fileName31 = child31;
    fileName31.AppendToPath(L"File31.txt");
    file31.Create(fileName31, true);
    }
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeFileListIterator, Test_Recursive)
    {
    //sleep(10);

    BeFileName root;
    BeTest::GetHost().GetOutputRoot(root);
    root.AppendToPath(L"IteratorTest");
    createFiles(root);

    BeFileName wildcard (NULL, root, L"*", NULL);
    BeFileListIterator it (wildcard, /*recursive*/true);
    BeFileName name;
    size_t count=0;
    bool foundFileName1 = false;
    bool foundFileName31 = false;
    while (it.GetNextFileName (name) == SUCCESS)
        {
        printf("name='%s'\n", Utf8String(name.c_str()).c_str());

        BeFileName dir (BeFileName::DevAndDir, name);
        ASSERT_TRUE( 0 == BeStringUtilities::Wcsnicmp (root, dir, wcslen(root)) );
        BeFileName base (BeFileName::Basename, name);
        if (0==BeStringUtilities::Wcsicmp (base, L"File1"))
            foundFileName1 = true;
        else if (0==BeStringUtilities::Wcsicmp (base, L"File31"))
            foundFileName31 = true;
        ++count;
        }
    ASSERT_EQ( 5 , count);
    ASSERT_TRUE(foundFileName1);
    ASSERT_TRUE(foundFileName31);
    }
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeFileListIterator, Test_NonRecursive)
    {
    BeFileName root;
    BeTest::GetHost().GetOutputRoot(root);
    root.AppendToPath(L"IteratorTestNonRecursive");
    createFiles(root);

    BeFileName wildcard (NULL, root, L"*", NULL);
    BeFileListIterator it (wildcard, /*recursive*/false);
    BeFileName name;
    size_t count=0;
    bool foundFileName0 = false;
    bool foundFileName31 = false;
    while (it.GetNextFileName (name) == SUCCESS)
        {
        BeFileName dir (BeFileName::DevAndDir, name);
        ASSERT_TRUE( 0 == BeStringUtilities::Wcsnicmp (root, dir, wcslen(root)) );
        BeFileName base (BeFileName::Basename, name);
        if (0==BeStringUtilities::Wcsicmp (base, L"File0"))
            foundFileName0 = true;
        else if (0==BeStringUtilities::Wcsicmp (base, L"File31"))
            foundFileName31 = true;
        ++count;
        }
    ASSERT_EQ( 4 , count);
    ASSERT_TRUE(foundFileName0);
    ASSERT_FALSE(foundFileName31);
    }
//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeFileListIterator, Test_EmptyDir)
    {
    BeFileName root;
    BeTest::GetHost().GetOutputRoot(root);
    root.AppendToPath(L"Empty");
    if (!root.DoesPathExist())
        ASSERT_TRUE (BeFileNameStatus::Success ==  BeFileName::CreateNewDirectory(root.c_str()));

    BeFileName wildcard (NULL, root, L"*", NULL);
    BeFileListIterator it (wildcard, /*recursive*/true);
    BeFileName name;
    size_t count=0;
    while (it.GetNextFileName (name) == SUCCESS)
        {
        ++count;
        }
    ASSERT_EQ( 0 , count);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
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
            if (BeFileName::GetExtension(entryName).EqualsI (L"txt"))
                ++count;
            }
        }
    return count;
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST (BeDirectoryIterator, Test1)
    {
    BeFileName root;
    BeTest::GetHost().GetOutputRoot(root);
    root.AppendToPath(L"IteratorTest");
    createFiles(root);
    //  Count the number of files ending in ".txt" by iterating recursively through the doc directory tree.
    size_t countv8 = countV8InDir(root);
    ASSERT_TRUE( countv8 == 3 );

    //  Verify that the WalkDirsAndMatch utility gets the same count.
    bvector<BeFileName> found;
    BeDirectoryIterator::WalkDirsAndMatch(found, root, L"*.txt", /*recursive*/true);
    ASSERT_EQ( found.size(), countv8 );
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BeDirectoryIterator, FileNamePattern_Parse)
    {
    BeFileName dir;
    WString glob;
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    BeFileName wildcard (NULL, L"C:\\abc\\bca", L"a*.txt", NULL);
    FileNamePattern::Parse(dir, glob, wildcard);
    EXPECT_STREQ(L"C:\\abc\\bca", dir.c_str()) << "Directory name is incorrectly parsed";
#else 
    BeFileName wildcard (NULL, L"/var/abc/bca", L"a*.txt", NULL);
    FileNamePattern::Parse(dir, glob, wildcard);
    EXPECT_STREQ(L"/var/abc/bca", dir.c_str()) << "Directory name is incorrectly parsed";
#endif
    EXPECT_STREQ(L"a*.txt", glob.c_str());
    }
