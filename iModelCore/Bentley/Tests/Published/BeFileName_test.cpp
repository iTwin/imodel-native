/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeFileName_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>

#if defined (BENTLEY_WIN32)
    #include "Shlwapi.h"
#else
    #if !defined (__DFTYPES_H)
    struct _FILETIME
    {
    uint32_t dwLowDateTime;
    uint32_t dwHighDateTime;
    };
    #endif
#endif

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #define WINDOWS_EXTENDED_PATH_PREFIX L"\\\\?\\"
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    10/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameSetName)
{
    WCharCP fileNameUnixW       = L"/dir1/dir2/temp.txt";
    CharCP  fileNameUnixA       =  "/dir1/dir2/temp.txt";
    WCharCP fileNameWindowsW    = L"\\dir1\\dir2\\temp.txt";
    CharCP  fileNameWindowsA    =  "\\dir1\\dir2\\temp.txt";
    WCharCP fileNameMixedUpW    = L"\\dir1/dir2\\temp.txt";
    CharCP  fileNameMixedUpA    =  "\\dir1/dir2\\temp.txt";

#if defined (_WIN32)
    WCharCP fileNameExpectedW   = fileNameWindowsW;
    CharCP  fileNameExpectedA   = fileNameWindowsA;
#else
    WCharCP fileNameExpectedW   = fileNameUnixW;
    CharCP  fileNameExpectedA   = fileNameUnixA;
#endif

    BeFileName fromFileNameUnixW    (fileNameUnixW);
    BeFileName fromFileNameWindowsW (fileNameWindowsW);
    BeFileName fromFileNameMixedUpW (fileNameMixedUpW);

    BeFileName fromFileNameUnixA;
    BeFileName fromFileNameWindowsA;
    BeFileName fromFileNameMixedUpA;

    fromFileNameUnixA.SetNameA    (fileNameUnixA);
    fromFileNameWindowsA.SetNameA (fileNameWindowsA);
    fromFileNameMixedUpA.SetNameA (fileNameMixedUpA);

    ASSERT_TRUE (0 == wcscmp (fileNameExpectedW, fromFileNameUnixW.GetName ()));
    ASSERT_TRUE (0 == wcscmp (fileNameExpectedW, fromFileNameUnixA.GetName ()));
    ASSERT_TRUE (0 == wcscmp (fileNameExpectedW, fromFileNameWindowsW.GetName ()));
    ASSERT_TRUE (0 == wcscmp (fileNameExpectedW, fromFileNameWindowsA.GetName ()));
    ASSERT_TRUE (0 == wcscmp (fileNameExpectedW, fromFileNameMixedUpW.GetName ()));
    ASSERT_TRUE (0 == wcscmp (fileNameExpectedW, fromFileNameMixedUpA.GetName ()));

    char fileNameA[MAX_PATH];

    fromFileNameUnixW.GetNameA (fileNameA);
    ASSERT_TRUE (0 == strcmp (fileNameExpectedA, fileNameA));

    fromFileNameUnixA.GetNameA (fileNameA);
    ASSERT_TRUE (0 == strcmp (fileNameExpectedA, fileNameA));

    fromFileNameWindowsW.GetNameA (fileNameA);
    ASSERT_TRUE (0 == strcmp (fileNameExpectedA, fileNameA));

    fromFileNameWindowsA.GetNameA (fileNameA);
    ASSERT_TRUE (0 == strcmp (fileNameExpectedA, fileNameA));

    fromFileNameMixedUpW.GetNameA (fileNameA);
    ASSERT_TRUE (0 == strcmp (fileNameExpectedA, fileNameA));

    fromFileNameMixedUpA.GetNameA (fileNameA);
    ASSERT_TRUE (0 == strcmp (fileNameExpectedA, fileNameA));

    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    10/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameSetNameUtf8)
{
#if defined (_WIN32)
    Utf8String fileNameUtf8  = "\\junk1\\junk2\\junk3.txt";
    WCharCP expectedFileName = L"\\junk1\\junk2\\junk3.txt";
#else
    Utf8String fileNameUtf8  = "/junk1/junk2/junk3.txt";
    WCharCP expectedFileName = L"/junk1/junk2/junk3.txt";
#endif

    BeFileName fromFileNameUtf8 (fileNameUtf8);
    
    ASSERT_TRUE (0 == wcscmp (expectedFileName, fromFileNameUtf8.GetName ()));
    ASSERT_TRUE (0 == strcmp (fileNameUtf8.c_str (), fromFileNameUtf8.GetNameUtf8().c_str ()));
    
    fromFileNameUtf8.Clear ();
    fromFileNameUtf8.SetNameUtf8 (fileNameUtf8);
    
    ASSERT_TRUE (0 == wcscmp (expectedFileName, fromFileNameUtf8.GetName ()));
    ASSERT_TRUE (0 == strcmp (fileNameUtf8.c_str (), fromFileNameUtf8.GetNameUtf8().c_str ()));
    
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameParseName1)
{
    BeFileName  fileName ("\\dir1\\dir2\\temp.txt", true);
    BeFileName  fileNameW (L"/dir1/dir2/temp.txt");
    WString     dirName;

#if defined (_WIN32)
    WCharCP     expected = L"\\dir1\\dir2\\";
#else
    WCharCP     expected = L"/dir1/dir2/";
#endif

    ASSERT_TRUE (0 == wcscmp (fileName.GetName (), fileNameW.GetName ()));

    fileName.ParseName (NULL, &dirName, NULL, NULL);
    ASSERT_TRUE (0 == wcscmp (expected, dirName.c_str ()));

    dirName = L"?";
    fileNameW.ParseName (NULL, &dirName, NULL, NULL);
    ASSERT_TRUE (0 == wcscmp (expected, dirName.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameParseName2)
{
    BeFileName  fileName (L"/data/temp.txt");
    WString     driveName, dirName, baseName, extName;

#if defined (_WIN32)
    WCharCP     expectedDirName = L"\\data\\";
#else
    WCharCP     expectedDirName = L"/data/";
#endif

    fileName.ParseName (&driveName, &dirName, &baseName, &extName);

    ASSERT_TRUE (0 == wcscmp (L"", driveName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (expectedDirName, dirName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"temp", baseName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"txt", extName.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameParseName3)
{
    BeFileName  fileName (L"/junk1/junk2/temp.txt");
    WString     driveName, dirName, baseName, extName;
    Utf8String s=fileName.GetUri(); 
#if defined (_WIN32)
    WCharCP     expectedDirName = L"\\junk1\\junk2\\";
#else
    WCharCP     expectedDirName = L"/junk1/junk2/";
#endif

    fileName.ParseName (&driveName, &dirName, &baseName, &extName);

    ASSERT_TRUE (0 == wcscmp (L"", driveName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (expectedDirName, dirName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"temp", baseName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"txt", extName.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameParseName4)
{
    BeFileName  fileName (L"/data");
    WString     driveName, dirName, baseName, extName;

    fileName.ParseName (&driveName, &dirName, &baseName, &extName);

    ASSERT_TRUE (0 == wcscmp (L"", driveName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (WCSDIR_SEPARATOR, dirName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"data", baseName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"", extName.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameParseName5)
{
    BeFileName  fileName ("/junk1/junk2/", true);
    WString     driveName, dirName, baseName, extName;

#if defined (_WIN32)
    WCharCP     expectedDirName = L"\\junk1\\junk2\\";
#else
    WCharCP     expectedDirName = L"/junk1/junk2/";
#endif

    fileName.ParseName (&driveName, &dirName, &baseName, &extName);

    ASSERT_TRUE (0 == wcscmp (L"", driveName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (expectedDirName, dirName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"", baseName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"", extName.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    10/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameParseName6)
{
    WString driveName, dirName, baseName, extName;
    WCharCP fileNameUnix    = L"/junk1/junk2/temp.txt";
    WCharCP fileNameWindows = L"\\junk1\\junk2\\temp.txt";

#if defined (_WIN32)
    WCharCP expectedDirName = L"\\junk1\\junk2\\";
#else
    WCharCP expectedDirName = L"/junk1/junk2/";
#endif
    
    BeFileName::ParseName (&driveName, &dirName, &baseName, &extName, fileNameUnix);
    
    ASSERT_TRUE (0 == wcscmp (L"", driveName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (expectedDirName, dirName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"temp", baseName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"txt", extName.c_str ()));
    
    driveName.clear ();
    dirName.clear ();
    baseName.clear ();
    extName.clear ();
    
    BeFileName::ParseName (&driveName, &dirName, &baseName, &extName, fileNameWindows);
    
    ASSERT_TRUE (0 == wcscmp (L"", driveName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (expectedDirName, dirName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"temp", baseName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"txt", extName.c_str ()));
    
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName1)
{
    BeFileName fileName (NULL, L"/junk1/junk2", L"junk3", L"txt");

#if defined (_WIN32)
    WCharCP expectedFileName = L"\\junk1\\junk2\\junk3.txt";
#else
    WCharCP expectedFileName = L"/junk1/junk2/junk3.txt";
#endif

    char buffer[MAX_PATH];
    fileName.GetNameA (buffer);

    ASSERT_TRUE (0 == wcscmp (expectedFileName, fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName2)
{
    BeFileName fileName (NULL, L"/junk1/junk2/", L"junk3", L"txt");

#if defined (_WIN32)
    WCharCP expectedFileName = L"\\junk1\\junk2\\junk3.txt";
#else
    WCharCP expectedFileName = L"/junk1/junk2/junk3.txt";
#endif

    ASSERT_TRUE (0 == wcscmp (expectedFileName, fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName3)
{
    BeFileName fileName (NULL, L"/junk1/junk2/", L"junk3", L".txt");

#if defined (_WIN32)
    WCharCP expectedFileName = L"\\junk1\\junk2\\junk3.txt";
#else
    WCharCP expectedFileName = L"/junk1/junk2/junk3.txt";
#endif

    ASSERT_TRUE (0 == wcscmp (expectedFileName, fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName4)
{
    BeFileName fileName (NULL, L"/junk1/junk2/", L"junk3.txt", NULL);

#if defined (_WIN32)
    WCharCP expectedFileName = L"\\junk1\\junk2\\junk3.txt";
#else
    WCharCP expectedFileName = L"/junk1/junk2/junk3.txt";
#endif

    ASSERT_TRUE (0 == wcscmp (expectedFileName, fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName5)
{
    BeFileName fileName (NULL, L"/junk1/junk2/", NULL, NULL);

#if defined (_WIN32)
    WCharCP expectedDirName = L"\\junk1\\junk2\\";
#else
    WCharCP expectedDirName = L"/junk1/junk2/";
#endif

    ASSERT_TRUE (0 == wcscmp (expectedDirName, fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName6)
{
    BeFileName fileName (NULL, L"/junk1/junk2", NULL, NULL);

#if defined (_WIN32)
    WCharCP expectedDirName = L"\\junk1\\junk2\\";
#else
    WCharCP expectedDirName = L"/junk1/junk2/";
#endif

    ASSERT_TRUE (0 == wcscmp (expectedDirName, fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName7)
{
    BeFileName fileName (NULL, NULL, L"junk3.txt", NULL);

    ASSERT_TRUE (0 == wcscmp (L"junk3.txt", fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBuildName8)
{
    BeFileName fileName (NULL, NULL, NULL, NULL);

    ASSERT_TRUE (0 == wcscmp (L"", fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    12/13
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, TrimWhiteSpace)
    {
    // should skip over leading white space characters
    BeFileName fileName (NULL, NULL, L" junk3.txt", NULL);
    ASSERT_TRUE (0 == wcscmp (L"junk3.txt", fileName.GetName()));

    fileName.SetName (L"   \t\njunk3.txt");
    ASSERT_TRUE (0 == wcscmp (L"junk3.txt", fileName.GetName()));

    // should trim off all trailing whitespace characters
    fileName.SetName (L"junk3.txt ");
    ASSERT_TRUE (0 == wcscmp (L"junk3.txt", fileName.GetName()));

    fileName.SetName (L"junk3.txt   \t\n");
    ASSERT_TRUE (0 == wcscmp (L"junk3.txt", fileName.GetName()));

    // skip and trim case
    fileName.SetName (L"  \t  \njunk3.txt   \t  \n");
    ASSERT_TRUE (0 == wcscmp (L"junk3.txt", fileName.GetName()));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Brandon.Bohrer                  01/12
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetDirectoryNameWithDrive)
{
#ifdef _WIN32
    BeFileName fileName (L"C:\\foo\\bar\\bat.txt");
    ASSERT_TRUE (L"C:\\foo\\bar\\" == BeFileName::GetDirectoryName (fileName.GetName()));
    ASSERT_TRUE (L"C:\\foo\\bar\\" == fileName.GetDirectoryName());
#else
    BeFileName fileName (L"/foo/bar/bat.txt");
    ASSERT_TRUE (L"/foo/bar/" == BeFileName::GetDirectoryName (fileName.GetName()));
    ASSERT_TRUE (L"/foo/bar/" == fileName.GetDirectoryName());
#endif
}

//---------------------------------------------------------------------------------------
// @betest                                      Brandon.Bohrer                  01/12
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetDirectoryNameNoDriveAbsolute)
{
#ifdef _WIN32
    WString expected (L"\\foo\\bar\\");
#else
    WString expected (L"/foo/bar/");
#endif
    
    // static version
    WString actual (BeFileName::GetDirectoryName (L"/foo/bar/bat.txt"));
    ASSERT_TRUE (expected == actual);

    // instance version
    BeFileName fileName (L"/foo/bar/bat.txt");
    ASSERT_TRUE (expected == fileName.GetDirectoryName());
}

//---------------------------------------------------------------------------------------
// @betest                                      Brandon.Bohrer                  01/12
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetDirectoryNameNoDriveRelative)
{
#ifdef _WIN32
    WString expected (L"foo\\bar\\");
#else
    WString expected (L"foo/bar/");
#endif
    // static version
    WString actual (BeFileName::GetDirectoryName (L"foo/bar/bat.txt"));
    ASSERT_TRUE (expected == actual);

    // instance version
    BeFileName fileName (L"foo/bar/bat.txt");
    ASSERT_TRUE (expected == fileName.GetDirectoryName());
}
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                  02/16
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetDirectoryNameWithoutDrive)
{
#ifdef _WIN32
    BeFileName fileName (L"C:\\foo\\bar\\bat.txt");
    EXPECT_STREQ (L"\\foo\\bar\\" , BeFileName::GetDirectoryWithoutDevice (fileName.GetName()).c_str());
    EXPECT_STREQ(L"\\foo\\bar\\" , fileName.GetDirectoryWithoutDevice().c_str());
#else
    BeFileName fileName (L"/foo/bar/bat.txt");
    EXPECT_STREQ (L"/foo/bar/" , BeFileName::GetDirectoryWithoutDevice (fileName.GetName()).c_str());
    EXPECT_STREQ (L"/foo/bar/" , fileName.GetDirectoryWithoutDevice().c_str());
#endif
}

//---------------------------------------------------------------------------------------
// @betest                                      Brandon.Bohrer                  01/12
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetExtension)
{
    // static version
    ASSERT_TRUE (L"" == BeFileName::GetExtension (NULL));
    ASSERT_TRUE (L"" == BeFileName::GetExtension (L""));
    ASSERT_TRUE (L"txt" == BeFileName::GetExtension (L"/foo/bar/bat.txt"));
    ASSERT_TRUE (L"" == BeFileName::GetExtension (L"/foo/bar/bat"));
    ASSERT_TRUE (L"" == BeFileName::GetExtension (L"/foo/bar/bat."));
    ASSERT_TRUE (L"xml" == BeFileName::GetExtension (L"/foo/bar/foo.partfile.xml"));
    ASSERT_TRUE (L"emacs" == BeFileName::GetExtension (L"/foo/bar.emacs"));

    // instance version
    BeFileName fileName;
    ASSERT_TRUE (L"" == fileName.GetExtension());

    fileName.SetName (L"/foo/bar/bat.txt");
    ASSERT_TRUE (L"txt" == fileName.GetExtension());

    fileName.SetName (L"/foo/bar/bat");
    ASSERT_TRUE (L"" == fileName.GetExtension());

    fileName.SetName (L"/foo/bar/bat.");
    ASSERT_TRUE (L"" == fileName.GetExtension());

    fileName.SetName (L"/foo/bar/foo.partfile.xml");
    ASSERT_TRUE (L"xml" == fileName.GetExtension());

    fileName.SetName (L"/foo/bar.emacs");
    ASSERT_TRUE (L"emacs" == fileName.GetExtension());
}

//---------------------------------------------------------------------------------------
// @betest                                      Brandon.Bohrer                  01/12
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetFileNameAndExtension)
{
    // static version
    ASSERT_TRUE (L"" == BeFileName::GetFileNameAndExtension (NULL));
    ASSERT_TRUE (L"" == BeFileName::GetFileNameAndExtension (L""));
    ASSERT_TRUE (L"bat.txt" == BeFileName::GetFileNameAndExtension (L"/foo/bar/bat.txt"));
    ASSERT_TRUE (L"bat" == BeFileName::GetFileNameAndExtension (L"/foo/bar/bat"));
    ASSERT_TRUE (L"bat" == BeFileName::GetFileNameAndExtension (L"/foo/bar/bat."));
    ASSERT_TRUE (L".emacs" == BeFileName::GetFileNameAndExtension (L"/foo/bar/.emacs"));
    ASSERT_TRUE (L"foo.partfile.xml" == BeFileName::GetFileNameAndExtension (L"/foo.bar/foo.partfile.xml"));

    // instance version
    BeFileName fileName;
    ASSERT_TRUE (L"" == fileName.GetFileNameAndExtension());

    fileName.SetName (L"/foo/bar/bat.txt");
    ASSERT_TRUE (L"bat.txt" == fileName.GetFileNameAndExtension());

    fileName.SetName (L"/foo/bar/bat");
    ASSERT_TRUE (L"bat" == fileName.GetFileNameAndExtension());

    fileName.SetName (L"/foo/bar/bat.");
    ASSERT_TRUE (L"bat" == fileName.GetFileNameAndExtension());

    fileName.SetName (L"/foo/bar/.emacs");
    ASSERT_TRUE (L".emacs" == fileName.GetFileNameAndExtension());

    fileName.SetName (L"/foo.bar/foo.partfile.xml");
    ASSERT_TRUE (L"foo.partfile.xml" == fileName.GetFileNameAndExtension());
}

//---------------------------------------------------------------------------------------
// @betest                                      Brandon.Bohrer                  01/12
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetFileNameWithoutExtension)
{
    // static version
    ASSERT_TRUE (L"" == BeFileName::GetFileNameWithoutExtension (NULL));
    ASSERT_TRUE (L"" == BeFileName::GetFileNameWithoutExtension (L""));
    ASSERT_TRUE (L"bat" == BeFileName::GetFileNameWithoutExtension (L"/foo/bar/bat.txt"));
    ASSERT_TRUE (L"bat" == BeFileName::GetFileNameWithoutExtension (L"/foo/bar/bat"));
    ASSERT_TRUE (L"bat" == BeFileName::GetFileNameWithoutExtension (L"/foo/bar/bat."));
    ASSERT_TRUE (L"" == BeFileName::GetFileNameWithoutExtension (L"/foo/bar/.emacs"));
    ASSERT_TRUE (L"foo.partfile" == BeFileName::GetFileNameWithoutExtension (L"/foo/bar/foo.partfile.xml"));

    // instance version
    BeFileName fileName;
    ASSERT_TRUE (L"" == fileName.GetFileNameWithoutExtension());

    fileName.SetName (L"/foo/bar/bat.txt");
    ASSERT_TRUE (L"bat" == fileName.GetFileNameWithoutExtension());

    fileName.SetName (L"/foo/bar/bat");
    ASSERT_TRUE (L"bat" == fileName.GetFileNameWithoutExtension());

    fileName.SetName (L"/foo/bar/bat.");
    ASSERT_TRUE (L"bat" == fileName.GetFileNameWithoutExtension());

    fileName.SetName (L"/foo/bar/.emacs");
    ASSERT_TRUE (L"" == fileName.GetFileNameWithoutExtension());

    fileName.SetName (L"/foo/bar/foo.partfile.xml");
    ASSERT_TRUE (L"foo.partfile" == fileName.GetFileNameWithoutExtension());
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameDoesPathExist1)
{
    BeFileName fileName (NULL, NULL, L"junk3.txt", NULL);

    ASSERT_FALSE (BeFileName::DoesPathExist (fileName.GetName ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameDoesPathExist2)
{
#if defined (BENTLEY_WIN32)

    BeFileName fileName (NULL, NULL, L"C:\\", NULL);

#elif defined (BENTLEY_WINRT)
    
    BeFileName fileName;
    BeTest::GetHost().GetOutputRoot (fileName);

#elif defined (ANDROID)    

    BeFileName fileName (NULL, NULL, L"/data", NULL);
    
#elif defined (__APPLE__)    
    
    BeFileName fileName (NULL, NULL, L"/var", NULL);
    
#elif defined (__unix__)    
    
    BeFileName fileName (NULL, NULL, L"/usr", NULL);

#endif
    
    ASSERT_TRUE (BeFileName::DoesPathExist (fileName.GetName ()));
    ASSERT_TRUE (BeFileName::DoesPathExist (fileName)); // test BeFileName operator WCharCP()
    SUCCEED ();
}


//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameIsDirectory1)
{
    BeFileName fileName;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (fileName);
    
    ASSERT_TRUE (BeFileName::IsDirectory (fileName));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    09/11
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameIsDirectory2)
{
    BeFileName fileName (L"/junk1/junk2/");

    ASSERT_FALSE (BeFileName::IsDirectory (fileName));
    SUCCEED ();
}

//-------------------------------------------------------------------------------------------------------
// @bsimethod                                                    Sam.Wilson      3/13
//-------------------------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetFileSizeAndLastModificationTime)
    {
    uint64_t startTimeMillis = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();

    BeFileName tmp;
    BeTest::GetHost().GetOutputRoot (tmp);
    tmp.AppendToPath (L"BeFileNameTests_GetFileSize.txt");

    Utf8String utf8Name;
    BeStringUtilities::WCharToUtf8 (utf8Name, tmp);
    FILE* fp = fopen (utf8Name.c_str(), "w");
    fwrite ("0123456789", 10, 1, fp);
    fclose (fp);

    ASSERT_TRUE (BeFileName::DoesPathExist (tmp.GetName ()));

    uint64_t sz;
    ASSERT_TRUE( BeFileName::GetFileSize (sz, tmp) == BeFileNameStatus::Success );
    ASSERT_EQ( sz, 10 );
    
    time_t           fileModificationTime = 0;
    ASSERT_TRUE (BeFileNameStatus::Success == BeFileName::GetFileTime (NULL, NULL, &fileModificationTime, tmp.GetName()) );

    uint64_t fileModificationMillis = static_cast <uint64_t> (fileModificationTime * 1000LL);

    uint64_t currentTimeMillis      = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();

    uint64_t startTimeMillisTruncated = (startTimeMillis/1000)*1000;
    // the actual resolution of file times on Android and iOS is seconds (even though we report them as milliseconds).
    ASSERT_LE (startTimeMillisTruncated, fileModificationMillis);
    ASSERT_GE (currentTimeMillis, fileModificationMillis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/13
+---------------+---------------+---------------+---------------+---------------+------*/
static WString  toNativePath (WStringCR str)
    {
    WString s(str);
    #ifndef _WIN32
        if (s.find (':') != WString::npos)
            return L"***Drive letters not supported in unix paths***";
    #endif
    BeFileName n (s.c_str());       // converts alt dir sep to dir sep
    return n.GetName();
    }

#ifdef _WIN32
    #define DRV L"D:"
#else
    #define DRV L""
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    testFindRelativePath (WCharCP fullPathIn, WCharCP basePathIn, WCharCP expectedRelativePathIn)
    {
    WString fullPath = toNativePath (fullPathIn);
    WString basePath = toNativePath (basePathIn);
    WString expectedRelativePath = toNativePath (expectedRelativePathIn);

    WString     relativePath;
    BeFileName::FindRelativePath (relativePath, fullPath.c_str(), basePath.c_str());

    ASSERT_TRUE (relativePath.Equals (expectedRelativePath));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef _WIN32
TEST (BeFileNameTests, FindRelativePath1)
    {
    WString     basePath     = DRV L"\\comp1\\comp2\\";
    WString     fullPath     = L"C:\\comp1\\comp2\\comp3\\comp4\\file.txt";
    WString     expectedPath = L"C:\\comp1\\comp2\\comp3\\comp4\\file.txt";
    testFindRelativePath (fullPath.c_str(), basePath.c_str(), expectedPath.c_str());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, FindRelativePath2)
    {
    WString     basePath     = DRV L"\\comp1\\comp2\\";
    WString     fullPath     = DRV L"\\comp1\\comp2\\comp3\\comp4\\file.txt";
    WString     expectedPath = L"comp3\\comp4\\file.txt";
    testFindRelativePath (fullPath.c_str(), basePath.c_str(), expectedPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, FindRelativePath3)
    {
    WString     basePath     = DRV L"\\comp1\\comp2\\";
    WString     fullPath     = DRV L"\\comp1\\file.txt";
    WString     expectedPath = L"..\\file.txt";
    testFindRelativePath (fullPath.c_str(), basePath.c_str(), expectedPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef _WIN32
TEST (BeFileNameTests, FindRelativePath4)
    {
    WString     basePath     = DRV L"\\comp1\\comp2\\";
    WString     fullPath     = L"\\comp1\\file.txt";
    WString     expectedPath = L"\\comp1\\file.txt";
    testFindRelativePath (fullPath.c_str(), basePath.c_str(), expectedPath.c_str());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, FindRelativePath5)
    {
    WString     basePath     = DRV L"\\";
    WString     fullPath     = DRV L"\\comp1\\comp2\\comp3\\comp4\\file.txt";
    WString     expectedPath = L"comp1\\comp2\\comp3\\comp4\\file.txt";
    testFindRelativePath (fullPath.c_str(), basePath.c_str(), expectedPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    testResolveRelativePath (WCharCP relativePathIn, WCharCP basePathIn, WCharCP expectedFullPathIn)
    {
    WString relativePath = toNativePath (relativePathIn);
    WString basePath = toNativePath (basePathIn);
    WString expectedFullPath = toNativePath (expectedFullPathIn);
    WString     fullPath;
    BeFileName::ResolveRelativePath (fullPath, relativePath.c_str(), basePath.c_str());

    ASSERT_TRUE (fullPath.Equals (expectedFullPath));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, ResolveRelativePath1)
    {
    WString     basePath;
    WString     relativePath;
    WString     expectedPath;

    basePath     = DRV L"\\comp1\\comp2\\";
    relativePath = L"comp3\\comp4\\file.txt";
    expectedPath = DRV L"\\comp1\\comp2\\comp3\\comp4\\file.txt";
    testResolveRelativePath (relativePath.c_str(), basePath.c_str(), expectedPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef _WIN32
TEST (BeFileNameTests, ResolveRelativePath2)
    {
    WString     basePath;
    WString     relativePath;
    WString     expectedPath;

    basePath     = DRV L"\\comp1\\comp2\\";
    relativePath = DRV L"\\comp3\\file.txt";
    expectedPath = DRV L"\\comp3\\file.txt";
    testResolveRelativePath (relativePath.c_str(), basePath.c_str(), expectedPath.c_str());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, ResolveRelativePath3)
    {
    WString     basePath;
    WString     relativePath;
    WString     expectedPath;

    basePath     = DRV L"\\comp1\\comp2\\";
    relativePath = L"..\\comp3\\comp4\\file.txt";
    expectedPath = DRV L"\\comp1\\comp3\\comp4\\file.txt";
    testResolveRelativePath (relativePath.c_str(), basePath.c_str(), expectedPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, ResolveRelativePath4)
    {
    WString     basePath;
    WString     relativePath;
    WString     expectedPath;

    basePath     = DRV L"\\comp1\\comp2\\abc.txt";
    relativePath = L"..\\comp3\\comp4\\file.txt";
    expectedPath = DRV L"\\comp1\\comp3\\comp4\\file.txt";
    testResolveRelativePath (relativePath.c_str(), basePath.c_str(), expectedPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, ResolveRelativePath5)
    {
    WString     basePath;
    WString     relativePath;
    WString     expectedPath;

    basePath     = DRV L"\\comp1\\comp2\\abc.txt";
    relativePath = L"..\\..\\..\\comp3\\comp4\\file.txt";
    expectedPath = DRV L"\\comp3\\comp4\\file.txt";
    testResolveRelativePath (relativePath.c_str(), basePath.c_str(), expectedPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Grigas.Petraitis    03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, CreateNewDirectory)
    {
    BeFileNameStatus status;
    BeFileName outRoot;
    BeTest::GetHost ().GetOutputRoot (outRoot);

    BeFileName path = outRoot;
    path.AppendToPath (L"test");

    // remove the directory if it exists
    if (BeFileName::DoesPathExist (path))
        {
        status = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    // make sure we can create a new directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (BeFileName::DoesPathExist (path));

    // make sure we get "alredy exists" flag when creating existing directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::AlreadyExists == status);

    // make sure we can create directory tree
    path.AppendToPath (L"a");
    path.AppendToPath (L"b");
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (BeFileName::DoesPathExist (path));

    // make sure we get correct result when we can't create a directory;
    // Windows systems (up to WinRT) don't have protected directories
#ifndef BENTLEY_WIN32
#ifdef BENTLEY_WINRT
    path.SetName (L"C:\\somenonexistent\\protected\\directory");
#else
    path.SetName (L"\\bin\\somenonexistent\\protected\\directory");
#endif
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::CantCreate == status);
#endif
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Julija.Suboc   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, AppendSeparator)
    {
    BeFileName outRoot;
    BeTest::GetHost ().GetOutputRoot (outRoot);
    //prepare test data
    WString pathExpected(outRoot.GetName());
    if (*pathExpected.rbegin() != WCSDIR_SEPARATOR_CHAR)    // don't assume that outRoot has a trailing separator
        pathExpected.append (WCSDIR_SEPARATOR);
    pathExpected.AppendA("newFolder");
    BeFileName path;
    //Append separator to empty filename
    path.AppendSeparator();
    Utf8String emptyUtf8(path.GetNameUtf8());
    ASSERT_TRUE(emptyUtf8.CompareTo(DIR_SEPARATOR)==0)<<"Failed to add separator to empty file name";
    //Append separator to path without separator
    path.SetName(outRoot);
    path.AppendToPath(L"newFolder");
    ASSERT_TRUE(pathExpected.CompareTo(path.GetName())==0)<<"Constructed path does not match path expected. \nExpected: "<<
        Utf8String(pathExpected).c_str()<<" \nConstructed: "<<path.GetNameUtf8().c_str();
    pathExpected.AppendA(DIR_SEPARATOR);
    path.AppendSeparator();
    ASSERT_TRUE(pathExpected.CompareTo(path.GetName())==0)<<
        "Path does not match path expected after appending separator. \nExpected: "<<Utf8String(pathExpected).c_str()<<
        " \nConstructed: "<<path.GetNameUtf8().c_str();
    //Append separator to path with separator
    path.AppendSeparator();
    ASSERT_TRUE(pathExpected.CompareTo(path.GetName())==0)<<
        "Path does not match path expected after appending separator. \nExpected: "<<Utf8String(pathExpected).c_str()<<
        " \nConstructed: "<<path.GetNameUtf8().c_str();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Julija.Suboc   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (BeFileNameTests, AppendSeparatorToWString)
    {
    BeFileName outRoot;
    BeTest::GetHost ().GetOutputRoot (outRoot);
    //prepare test data
    WString testPath(outRoot.GetName());
    if (*testPath.rbegin() != WCSDIR_SEPARATOR_CHAR)    // don't assume that outRoot has a trailing separator
        testPath.append (WCSDIR_SEPARATOR);
    testPath.AppendA("newFolder");
    WString testPathExpected(testPath); 
    testPathExpected.AppendA(DIR_SEPARATOR);
    //Append separator to string without separator
    BeFileName::AppendSeparator(testPath);
    ASSERT_TRUE(testPathExpected.CompareTo(testPath)==0)<<
        "Path does not match path expected after appending separator. \nExpected: "<<Utf8String(testPathExpected).c_str()<<
        " \nConstructed: "<<Utf8String(testPath).c_str();
    //Append separator to path with separator
    BeFileName::AppendSeparator(testPath);
    ASSERT_TRUE(testPathExpected.CompareTo(testPath)==0)<<
        "Path does not match path expected after appending separator. \nExpected: "<<Utf8String(testPathExpected).c_str()<<
        " \nConstructed: "<<Utf8String(testPath).c_str();
    //Append separator to empty path
    WString empty;
    BeFileName::AppendSeparator(empty);
    ASSERT_TRUE(empty.CompareTo(WCSDIR_SEPARATOR)==0)<<"Failed to append separator to empty string";
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  09/13
// Desc: Testing SetNameUTF8 for file name with Kanji Characters
// Expected result: Name with Kanji Characters should be recognised
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameSetNameUtf8Rev3)
{
#if defined (_WIN32)
    Utf8String fileNameUtf8 ( L"\\junk1\\junk2\\本.txt") ;
    WCharCP expectedFileName = L"\\junk1\\junk2\\本.txt";
#else
    Utf8String fileNameUtf8  = "/junk1/junk2/本.txt";
    WCharCP expectedFileName = L"/junk1/junk2/本.txt";
#endif

    BeFileName fromFileNameUtf8 (fileNameUtf8);
    
    ASSERT_TRUE (0 == wcscmp (expectedFileName, fromFileNameUtf8.GetName ()));
    ASSERT_TRUE (0 == strcmp (fileNameUtf8.c_str (), fromFileNameUtf8.GetNameUtf8().c_str ()));
    
    fromFileNameUtf8.Clear ();
    fromFileNameUtf8.SetNameUtf8 (fileNameUtf8);
    
    ASSERT_TRUE (0 == wcscmp (expectedFileName, fromFileNameUtf8.GetName ()));
    ASSERT_TRUE (0 == strcmp (fileNameUtf8.c_str (), fromFileNameUtf8.GetNameUtf8().c_str ()));
    
    SUCCEED ();
}

//#if defined(_WIN32) || defined(NEEDS_WORK_NON_WIN32)

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  09/13
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeMoveFile)
{
    BeFileName inRoot;
    BeFileName outRoot;
    BeTest::GetHost ().GetOutputRoot (outRoot);
    BeTest::GetHost ().GetOutputRoot (inRoot);
    BeFileName dest(inRoot);
    BeFileName path(outRoot);
    
    dest.AppendToPath(L"destDir");
    if (BeFileName::DoesPathExist (dest))
        {
        BeFileNameStatus destStatus;
        destStatus = BeFileName::EmptyAndRemoveDirectory (dest);
        ASSERT_TRUE (BeFileNameStatus::Success == destStatus);
        }

    BeFileNameStatus status = BeFileName::CreateNewDirectory (dest);
    ASSERT_TRUE (BeFileNameStatus::Success == status)<<(uint32_t)status;
    ASSERT_TRUE (BeFileName::DoesPathExist (dest));

    path.AppendToPath(L"test");
    if (BeFileName::DoesPathExist (path))
        {
        BeFileNameStatus destStatus;
        destStatus = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == destStatus);
        }

    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status)<<(uint32_t)status;
    ASSERT_TRUE (BeFileName::DoesPathExist (path));

    WCharCP expectedFileName = L"TestFile.txt";
    path.AppendToPath (expectedFileName);
    dest.AppendToPath (expectedFileName);
    BeFile testFile;
    BeFileStatus status1 = testFile.Create(path, true);
    ASSERT_TRUE(status1 == BeFileStatus::Success)<<"Failed to create file, file: "<<path;
    EXPECT_TRUE(testFile.IsOpen())<<"Open?";
    testFile.Close();
    
    BeFileName::BeMoveFile (path ,dest);

    ASSERT_TRUE (BeFileName::DoesPathExist (dest));
       
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  09/13
// Expected Result: File name is changed
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeMoveFile2)
{
#if defined (_WIN32)
    WCharCP fileName1  = L"\\junk1\\junk2\\temp.txt";
    WCharCP expectedFileName = L"\\dir1\\dir2\\temp2.txt";
    int numRetries = 0;
#else
    WCharCP fileName1  = L"/junk1/junk2/temp.txt";
    WCharCP expectedFileName = L"/dir1/dir2/temp2.txt";
    int numRetries = 0;
#endif

    BeFileName fromFileName1 (fileName1);
    BeFileName fromFileWcharCP (expectedFileName);
    
    BeFileNameStatus status = fromFileName1.BeMoveFile (fromFileName1 ,fromFileWcharCP, numRetries);
   
    ASSERT_TRUE (BeFileNameStatus::UnknownError == status); //returns an unknown error becasue there is no physical file present to be replaced
       
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    03/14
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeMoveFileAgainstEmptyDir)
    {
    BeFileNameStatus status;

    BeFileName dir1;
    BeTest::GetHost().GetOutputRoot (dir1);
    dir1.AppendToPath (L"BeMoveFileAgainstEmptyDir1");
    dir1.AppendSeparator();

    if (dir1.DoesPathExist())
        {
        status = BeFileName::EmptyAndRemoveDirectory (dir1.GetName());
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    BeFileName dir2;
    BeTest::GetHost().GetOutputRoot (dir2);
    dir2.AppendToPath (L"BeMoveFileAgainstEmptyDir2");
    dir2.AppendSeparator();

    if (dir2.DoesPathExist())
        {
        status = BeFileName::EmptyAndRemoveDirectory (dir2.GetName());
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    status = BeFileName::CreateNewDirectory (dir1.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (dir1.IsDirectory());
    ASSERT_TRUE (dir1.DoesPathExist());

    status = BeFileName::BeMoveFile (dir1, dir2);
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (dir2.IsDirectory());
    ASSERT_TRUE (dir2.DoesPathExist());
    ASSERT_FALSE (dir1.DoesPathExist());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    03/14
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeMoveFileAgainstDir)
    {
    BeFileNameStatus status;

    BeFileName dir1;
    BeTest::GetHost().GetOutputRoot (dir1);
    dir1.AppendToPath (L"BeMoveFileAgainstDir1");
    dir1.AppendSeparator();

    if (dir1.DoesPathExist())
        {
        status = BeFileName::EmptyAndRemoveDirectory (dir1.GetName());
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    BeFileName dir2;
    BeTest::GetHost().GetOutputRoot (dir2);
    dir2.AppendToPath (L"BeMoveFileAgainstDir2");
    dir2.AppendSeparator();

    if (dir2.DoesPathExist())
        {
        status = BeFileName::EmptyAndRemoveDirectory (dir2.GetName());
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    status = BeFileName::CreateNewDirectory (dir1.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (dir1.IsDirectory());
    ASSERT_TRUE (dir1.DoesPathExist());

    BeFileName fileName1 (NULL, dir1.GetName(), L"temp.txt", NULL);
    BeFileName fileName2 (NULL, dir2.GetName(), L"temp.txt", NULL);

    BeFile file;
    BeFileStatus fileStatus = file.Create (fileName1.GetName());
    ASSERT_TRUE (BeFileStatus::Success == fileStatus);
    fileStatus = file.Write (NULL, "0123456789", 10);
    ASSERT_TRUE (BeFileStatus::Success == fileStatus);
    fileStatus = file.Close();
    ASSERT_TRUE (BeFileStatus::Success == fileStatus);

    status = BeFileName::BeMoveFile (dir1, dir2);
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (dir2.IsDirectory());
    ASSERT_TRUE (dir2.DoesPathExist());
    ASSERT_TRUE (fileName2.DoesPathExist());
    ASSERT_FALSE (fileName1.DoesPathExist());
    ASSERT_FALSE (dir1.DoesPathExist());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    11/13
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, LongPaths)
    {
    WCharCP longSubDir1   = L"_________A_________B_________C_________D_________E_________F_________G_________H_________I";
    WCharCP longSubDir2   = L"_________J_________K_________L_________M_________N_________O_________P_________Q_________R";
    WCharCP longSubDir3   = L"_________S_________T_________U_________V_________W_________X_________Y_________Z";
    WCharCP longBaseName  = L"_________1_________2_________3_________4_________5_________6_________7_________8_________9_________0";
    WCharCP longExtension = L"thisIsAnUnusuallyLongFileExtension";
    BeFileNameStatus fileNameStatus;
    BeFileStatus fileStatus;

    // build a name that we know will exceed Windows MAX_PATH (260)
    BeFileName fileName;
    BeTest::GetHost().GetOutputRoot (fileName);
    fileName.AppendToPath (longSubDir1);
    fileName.AppendSeparator();

    // clean out anything left over from last run
    BeFileName::EmptyAndRemoveDirectory (fileName.GetName());
    ASSERT_TRUE (!fileName.DoesPathExist());

    fileNameStatus = BeFileName::CreateNewDirectory (fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (fileName.IsDirectory());

    fileName.AppendToPath (longSubDir2);
    fileName.AppendSeparator();
    fileNameStatus = BeFileName::CreateNewDirectory (fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (fileName.IsDirectory());

    fileName.AppendToPath (longSubDir3);
    fileName.AppendSeparator();
    fileNameStatus = BeFileName::CreateNewDirectory (fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (fileName.IsDirectory());

    uint64_t freeBytes = 0;
    fileNameStatus = BeFileName::BeGetDiskFreeSpace (freeBytes, fileName);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (freeBytes > 0);

    freeBytes = 0;
    fileNameStatus = BeFileName::BeGetDiskFreeSpace (freeBytes, fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (freeBytes > 0);

    fileName.AppendToPath (longBaseName);
    fileName.AppendExtension (longExtension);

    // verify that we can open/create a file with a long name
    BeFile file;
    fileStatus = file.Create (fileName.GetName());
    ASSERT_TRUE(BeFileStatus::Success ==fileStatus);

    fileStatus = file.Write (NULL, "0123456789", 10);
    ASSERT_TRUE(BeFileStatus::Success ==fileStatus);

    fileStatus = file.Close();
    ASSERT_TRUE(BeFileStatus::Success ==fileStatus);

    ASSERT_TRUE (fileName.DoesPathExist());
    ASSERT_TRUE (!fileName.IsDirectory());
    ASSERT_TRUE (!fileName.IsSymbolicLink());
    ASSERT_TRUE (BeFileName::DoesPathExist (fileName.GetName()));
    ASSERT_TRUE (!BeFileName::IsDirectory (fileName.GetName()));
    ASSERT_TRUE (!BeFileName::IsSymbolicLink (fileName.GetName()));
    
    // verify CheckAccess supports long names (both instance and static versions)
    fileNameStatus = fileName.CheckAccess (BeFileNameAccess::ReadWrite);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);

    fileNameStatus = BeFileName::CheckAccess (fileName.GetName(), BeFileNameAccess::ReadWrite);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);

    // verify GetFileSize supports long names (both instance and static versions)
    uint64_t fileSize = 0;
    fileNameStatus = fileName.GetFileSize (fileSize);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_EQ (10, fileSize);

    fileSize = 0;
    fileNameStatus = BeFileName::GetFileSize (fileSize, fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_EQ (10, fileSize);

    // verify GetFileTime/SetFileTime support long names (both instance and static versions)
    time_t fileModifiedTime = 0;
    fileNameStatus = fileName.GetFileTime (NULL, NULL, &fileModifiedTime);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (0 != fileModifiedTime);

    time_t newModifiedTime = fileModifiedTime + 1;
    fileNameStatus = fileName.SetFileTime (NULL, &newModifiedTime);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);

    fileNameStatus = fileName.GetFileTime (NULL, NULL, &fileModifiedTime);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_EQ (newModifiedTime, fileModifiedTime);

    newModifiedTime = fileModifiedTime + 1;
    fileNameStatus = BeFileName::SetFileTime (fileName.GetName(), NULL, &newModifiedTime);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);

    fileNameStatus = BeFileName::GetFileTime (NULL, NULL, &fileModifiedTime, fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_EQ (newModifiedTime, fileModifiedTime);

    // verify SetFileReadOnly supports long names (both instance and static versions)
    fileNameStatus = fileName.SetFileReadOnly (true);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);

    fileNameStatus = BeFileName::SetFileReadOnly (fileName.GetName(), false);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);

    // verify parsing functions support long names
    BeFileName devAndDir (BeFileName::DevAndDir, fileName.GetName());
    BeFileName nameAndExt (BeFileName::NameAndExt, fileName.GetName());
    BeFileName fileName2 (devAndDir.GetName());
    fileName2.AppendToPath (nameAndExt.GetName());
    ASSERT_TRUE (0 == wcscmp (fileName.GetName(), fileName2.GetName()));

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    WString dev, dir, name, ext;
    fileName.ParseName (&dev, &dir, &name, &ext);
    ASSERT_TRUE (0 != wcsncmp (dev.c_str(), WINDOWS_EXTENDED_PATH_PREFIX, wcslen (WINDOWS_EXTENDED_PATH_PREFIX)));
    ASSERT_TRUE (0 == wcsncmp (dir.c_str(), WINDOWS_EXTENDED_PATH_PREFIX, wcslen (WINDOWS_EXTENDED_PATH_PREFIX)));

    WString dev2, dir2, name2, ext2;
    fileName2.ParseName (&dev2, &dir2, &name2, &ext2);
    ASSERT_TRUE (0 != wcsncmp (dev2.c_str(), WINDOWS_EXTENDED_PATH_PREFIX, wcslen (WINDOWS_EXTENDED_PATH_PREFIX)));
    ASSERT_TRUE (0 == wcsncmp (dir2.c_str(), WINDOWS_EXTENDED_PATH_PREFIX, wcslen (WINDOWS_EXTENDED_PATH_PREFIX)));
#endif

    // verify BeCopyFile supports long names
    WString fileName2W (fileName2.GetName());
    fileName2W.append (L"2");
    fileName2.SetName (fileName2W.c_str());
    fileNameStatus = BeFileName::BeCopyFile (fileName, fileName2);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (fileName2.DoesPathExist());

    // verify BeMoveFile supports long names
    WString fileName3W (fileName.GetName());
    fileName3W.append (L"3");
    BeFileName fileName3 (fileName3W.c_str());
    fileNameStatus = BeFileName::BeMoveFile (fileName2, fileName3);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (fileName3.DoesPathExist());

    // verify BeDeleteFile supports long names (both instance and static versions)
    fileNameStatus = fileName.BeDeleteFile();
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (!fileName.DoesPathExist());

    fileNameStatus = BeFileName::BeDeleteFile (fileName3.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (!fileName3.DoesPathExist());

    // clean out everything from this run
    BeTest::GetHost().GetOutputRoot (fileName);
    fileName.AppendToPath (longSubDir1);
    fileName.AppendSeparator();
    BeFileName::EmptyAndRemoveDirectory (fileName.GetName());
    ASSERT_TRUE (!fileName.DoesPathExist());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    11/13
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, CreateNewDirectoryLongPaths)
    {
    WCharCP longSubDir1   = L"_________A_________B_________C_________D_________E_________F_________G_________H_________I_________J_________K_________L_________M_________N";
    BeFileNameStatus fileNameStatus;

    BeFileName fileName;
    BeTest::GetHost().GetOutputRoot (fileName);
    fileName.AppendToPath (longSubDir1);
    fileName.AppendSeparator();

    // clean out anything left over from last run
    BeFileName::EmptyAndRemoveDirectory (fileName.GetName());
    ASSERT_TRUE (!fileName.DoesPathExist());

    fileNameStatus = BeFileName::CreateNewDirectory (fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (fileName.IsDirectory());

    WString subDirName (L"x");

    // test all lengths up to and exceeding MAX_PATH
    for (int i=0; i<150; i++)
        {
        subDirName.append (L"x");

        BeFileName subDir (fileName.GetName());
        subDir.AppendToPath (subDirName.c_str());

        fileNameStatus = BeFileName::CreateNewDirectory (subDir.GetName());
        ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
        ASSERT_TRUE (subDir.DoesPathExist());
        ASSERT_TRUE (subDir.IsDirectory());
        }

    // clean out everything from this run 
    BeFileName::EmptyAndRemoveDirectory (fileName.GetName());
    ASSERT_TRUE (!fileName.DoesPathExist());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    11/13
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, FileTimeAlternatives)
    {
    BeFileName fileName;
    BeFileNameStatus fileNameStatus;
    BeFile file;
    BeFileStatus fileStatus;

    BeTest::GetHost().GetTempDir (fileName);
    fileName.AppendToPath (L"FileTimeAlternatives.temp");

    fileStatus = file.Create (fileName.GetName());
    ASSERT_TRUE(BeFileStatus::Success ==fileStatus);

    fileStatus = file.Write (NULL, "0123456789", 10);
    ASSERT_TRUE(BeFileStatus::Success ==fileStatus);

    fileStatus = file.Close();
    ASSERT_TRUE(BeFileStatus::Success ==fileStatus);

    time_t ctime = 0, atime = 0, mtime = 0;

    fileNameStatus = BeFileName::GetFileTime (&ctime, &atime, &mtime, fileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (ctime > 0);
    ASSERT_TRUE (atime > 0);
    ASSERT_TRUE (mtime > 0);

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)

    struct _stati64 status;
    int e = _wstati64 (fileName.GetName(), &status);
    ASSERT_TRUE (-1 != e);

    ASSERT_EQ (ctime, status.st_ctime);
    ASSERT_EQ (atime, status.st_atime);
    ASSERT_EQ (mtime, status.st_mtime);
    
#endif
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    11/13
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, TestCanonicalize)
    {
#if defined (BENTLEY_WIN32)

    BOOL successful;
    wchar_t outputPath[MAX_PATH];

    successful = ::PathCanonicalizeW (outputPath, L"..\\..\\file.ext");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"..\\inbetween\\..\\file.ext");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"..\\inbetween\\..\\inbetween\\..\\file.ext");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"\\a\\..\\b\\file.ext");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"\\a\\b\\");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"/a/b/");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"rel1\\rel2\\file.ext");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"rel1\\rel2\\..\\file.ext");
    ASSERT_TRUE (successful != FALSE);

    successful = ::PathCanonicalizeW (outputPath, L"rel1\\rel2\\.\\file.ext");
    ASSERT_TRUE (successful != FALSE);

#endif
    }

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    11/13
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, TrailingSeparatorBehavior)
    {
    WString fileName;
    BeFileNameStatus fileNameStatus;

    fileNameStatus = BeFileName::FixPathName (fileName, L"/dir1/dir2/", true);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (WCSDIR_SEPARATOR_CHAR == fileName.end()[-1]);

    fileNameStatus = BeFileName::FixPathName (fileName, L"/dir1/dir2/", false);
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (WCSDIR_SEPARATOR_CHAR != fileName.end()[-1]);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                    02/16
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetFileSizeForSymbolicLink)
    {
#if defined (BENTLEY_WIN32)

    BeFileName rootDir;
    BeTest::GetHost().GetOutputRoot(rootDir);
    BeFileName targetFileName, symLinkFileName;
    targetFileName = symLinkFileName = rootDir;
    targetFileName.AppendToPath(L"targetFile.txt");
    symLinkFileName.AppendToPath(L"SymlinkedFile.txt");
    
    BeFile file;
    ASSERT_TRUE(BeFileStatus::Success == file.Create(targetFileName.c_str(), true));
    Utf8String someData = "hakuna matata";
    ASSERT_TRUE(BeFileStatus::Success == file.Write(nullptr, someData.c_str(), (uint32_t)someData.length()));
    ASSERT_TRUE(BeFileStatus::Success == file.Close());

    Utf8String target_utf8(targetFileName.c_str());
    Utf8String symlinkFile_utf8(symLinkFileName.c_str());

    CreateSymbolicLinkA(symlinkFile_utf8.c_str(), target_utf8.c_str(), 0);

    ASSERT_TRUE (BeFileName::IsSymbolicLink (symLinkFileName.c_str()));

    // following a sym link is not yet supported on WinRT
    uint64_t fileSize;
    BeFileNameStatus fileNameStatus = BeFileName::GetFileSize (fileSize, symLinkFileName.GetName());
    ASSERT_TRUE (BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE (fileSize > 0);

    // compare against legacy way of calculating file size (that does not support names >= MAX_PATH)
    struct _stati64 status;
    int e = _wstati64 (symLinkFileName.GetName(), &status);

    ASSERT_TRUE (-1 != e);
    ASSERT_EQ (fileSize, status.st_size);

#endif
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  09/13
// Desc: Testing of RemoveQuotes() method.
// Expected Result: File name is changed and has unicode that of a string without quotes
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameRemoveQuotes)
{
    WCharCP fileName1  = L"\"temp.txt\"";
    WCharCP expectedName = L"temp.txt";

    BeFileName fromFileName1 (fileName1);
    fromFileName1.RemoveQuotes ();

    EXPECT_NE(fromFileName1.GetName(),fileName1)<<  "After removing quotes\n"<<fromFileName1.GetName()<<"\n"<<"before removing quotes\n"<<fileName1 ;
    EXPECT_TRUE (0==wcscmp(expectedName, fromFileName1.GetName()));

    // Just confirm that it works fine if there are no Quotes
    WCharCP fileName2 = L"temp.txt";

    BeFileName fromFileName2(fileName2);
    fromFileName2.RemoveQuotes();

    EXPECT_NE(fromFileName2.GetName(), fileName1) << "After removing quotes\n" << fromFileName2.GetName() << "\n" << "before removing quotes\n" << fileName2;
    EXPECT_TRUE(0 == wcscmp(expectedName, fromFileName2.GetName()));

}

//------------------------------------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of ParseNameNoClear    (    WStringP     dev, WStringP     dir, WStringP     name, WStringP     ext ) method.
// Expected Result: Parsing is done in accordance with the method with no clear.
//-------------------------------------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameParseNameNoClear2)
{
    BeFileName  fileName ("/junk1/junk2/");
    WString     driveName, dirName, baseName, extName;

#if defined (_WIN32)
    WCharCP     expectedDirName = L"\\junk1\\junk2\\";
#else
    WCharCP     expectedDirName = L"/junk1/junk2/";
#endif

    fileName.ParseNameNoClear (&driveName, &dirName, &baseName, &extName);

    ASSERT_TRUE (0 == wcscmp (L"", driveName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (expectedDirName, dirName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"", baseName.c_str ()));
    ASSERT_TRUE (0 == wcscmp (L"", extName.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                  02/16
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, GetDevice)
{
#ifdef _WIN32
    BeFileName fileName (L"C:\\foo\\bar\\bat.txt");
    EXPECT_STREQ (L"C" , BeFileName::GetDevice (fileName.GetName()).c_str());
    EXPECT_STREQ (L"C" , fileName.GetDevice().c_str());
#else
    BeFileName fileName (L"/foo/bar/bat.txt");
    EXPECT_STREQ (L"" , BeFileName::GetDevice (fileName.GetName()).c_str());
    EXPECT_STREQ (L"" , fileName.GetDevice().c_str());
#endif
}

//------------------------------------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of GetDirectoryWithoutDevice    (    WCharCP Path ) method.
// Expected Result: Directory name should be parsed and returned
//-------------------------------------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameGetDirectoryWithoutDevice)
{
#ifdef _WIN32
    WCharCP Path = L"C:\\bar\\bat.txt";
    
    ASSERT_TRUE (L"\\bar\\"==BeFileName::GetDirectoryWithoutDevice (Path));
#else
    WCharCP Path = L"/bar/bat.txt";
    
    ASSERT_TRUE (L"/bar/" == BeFileName::GetDirectoryWithoutDevice (Path));
#endif
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of OverrideNameParts(WCharCP overrideName) method.
// Expected Result: File name is replaced by an override name
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameOverrideNameParts)
{
#if defined (_WIN32)
    WCharCP fileName1  = L"temp.txt";
    WCharCP overrideName = L"dump.txt";
  
#else
     WCharCP fileName1  = L"temp.txt";
     WCharCP overrideName = L"dump.txt";
    
#endif

    BeFileName fromFileName1 (fileName1);
        
    fromFileName1.OverrideNameParts (overrideName);
    EXPECT_TRUE(0==wcscmp(fromFileName1.GetName(),overrideName)) ;
    
    SUCCEED ();
}

//---------------------------------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of OverrideNameParts(WCharCP overrideName) method.
// Expected Result: File name is replaced by an override name, in this case the current name should be preserved
//---------------------------------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameOverrideNameParts1)
{
#if defined (_WIN32)
    WCharCP fileName1  = L"temp.txt";
    WCharCP overrideName = L"";
  
#else
     WCharCP fileName1  = L"temp.txt";
     WCharCP overrideName = L"";
    
#endif

    BeFileName fromFileName1 (fileName1);
        
    fromFileName1.OverrideNameParts (overrideName);
    EXPECT_TRUE(0==wcscmp(fromFileName1.GetName(),fileName1)) ;
    
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of CheckAccess() method for Read Access.
// Expected Result: File name has no quotes so unicode should also not change
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests,BeFileNameCheckAccessRead)
{
    WCharCP fileName1 = L"temp.txt";
    //WCharCP expectedName = L"temp.txt";
    BeFileNameAccess accs = BeFileNameAccess::Read  ;

    BeFileName fromFileName1;
        
    BeFileNameStatus status= BeFileName::CheckAccess(fileName1, accs);
    EXPECT_TRUE(BeFileNameStatus::FileNotFound==status) ;
    
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of CheckAccess() method for Write Access.
// Expected Result: File name has no quotes so unicode should also not change
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameCheckAccessWrite)
    {
    BeFileNameStatus status;
    BeFileName outRoot;
    BeTest::GetHost ().GetOutputRoot (outRoot);
    BeFileName path = outRoot;
    path.AppendToPath (L"test");
    BeFileNameAccess accs = BeFileNameAccess::Write  ;

     // remove the directory if it exists
    if (BeFileName::DoesPathExist (path))
        {
        status = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    // make sure we can create a new directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (BeFileName::DoesPathExist (path));
    
    
    // make sure we get "alredy exists" flag when creating existing directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::AlreadyExists == status);
     if (BeFileName::DoesPathExist (path))
        {
        status = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }
    // make sure we can create directory tree
    path.AppendToPath (L"a");
    path.AppendToPath (L"b");
    path.AppendToPath (L"TestFile.txt");
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status)<<(uint32_t)status;
    ASSERT_TRUE (BeFileName::DoesPathExist (path));

    WCharCP fullPath = path;
    BeFileName completePath (fullPath);

    BeFileNameStatus accessStatus = BeFileName::CheckAccess(fullPath,accs );

    EXPECT_TRUE(BeFileNameStatus::Success ==accessStatus)<<(uint32_t)accessStatus;

    // make sure we get correct result when we can't create a directory;
    // Windows systems (up to WinRT) don't have protected directories
#ifndef BENTLEY_WIN32
#ifdef BENTLEY_WINRT
    path.SetName (L"C:\\somenonexistent\\protected\\directory");
#else
    path.SetName (L"\\bin\\somenonexistent\\protected\\directory");
#endif
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::CantCreate == status);
#endif
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of CheckAccess() method for Read/Write Access.
// Expected Result: File name has no quotes so unicode should also not change
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameCheckAccessReadWrite)
    {
    BeFileNameStatus status;
    BeFileName outRoot;
    BeTest::GetHost ().GetOutputRoot (outRoot);
    BeFileName path = outRoot;
    path.AppendToPath (L"test");
    BeFileNameAccess accs = BeFileNameAccess::ReadWrite  ;

     // remove the directory if it exists
    if (BeFileName::DoesPathExist (path))
        {
        status = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    // make sure we can create a new directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (BeFileName::DoesPathExist (path));
    
    // make sure we get "alredy exists" flag when creating existing directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::AlreadyExists == status);
     if (BeFileName::DoesPathExist (path))
        {
        status = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }
    // make sure we can create directory tree
    path.AppendToPath (L"a");
    path.AppendToPath (L"b");
    path.AppendToPath (L"TestFile.txt");
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status)<<(uint32_t)status;
    ASSERT_TRUE (BeFileName::DoesPathExist (path));

    WCharCP fullPath = path;
    BeFileName completePath (fullPath);

    BeFileNameStatus    AccessStatus = BeFileName::CheckAccess(fullPath,accs );

    EXPECT_TRUE(BeFileNameStatus::Success ==AccessStatus)<<(uint32_t)AccessStatus;

    // make sure we get correct result when we can't create a directory;
    // Windows systems (up to WinRT) don't have protected directories
#ifndef BENTLEY_WIN32
#ifdef BENTLEY_WINRT
    path.SetName (L"C:\\somenonexistent\\protected\\directory");
#else
    path.SetName (L"\\bin\\somenonexistent\\protected\\directory");
#endif
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::CantCreate == status);
#endif
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of BeGetDiskFreeSpace method.
// Expected Result: File name has no quotes so unicode should also not change
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, BeFileNameBeGetDiskFreeSpace)
    {
    BeFileNameStatus status;
    BeFileName outRoot;
    BeTest::GetHost ().GetOutputRoot (outRoot);
    BeFileName path = outRoot;
    path.AppendToPath (L"test");
    uint64_t  freeBytes;

     // remove the directory if it exists
    if (BeFileName::DoesPathExist (path))
        {
        status = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }

    // make sure we can create a new directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status);
    ASSERT_TRUE (BeFileName::DoesPathExist (path));
    
    // make sure we get "alredy exists" flag when creating existing directory
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::AlreadyExists == status);
     if (BeFileName::DoesPathExist (path))
        {
        status = BeFileName::EmptyAndRemoveDirectory (path);
        ASSERT_TRUE (BeFileNameStatus::Success == status);
        }
    // make sure we can create directory tree
    path.AppendToPath (L"a");
    path.AppendToPath (L"b");
    path.AppendToPath (L"TestFile.txt");
    status = BeFileName::CreateNewDirectory (path);
    ASSERT_TRUE (BeFileNameStatus::Success == status)<<(uint32_t)status;
    ASSERT_TRUE (BeFileName::DoesPathExist (path));

    WCharCP fullPath = path;
    BeFileName completePath (fullPath);

    BeFileNameStatus    diskSpaceStatus = BeFileName:: BeGetDiskFreeSpace(freeBytes, fullPath);

    EXPECT_TRUE(BeFileNameStatus::Success ==diskSpaceStatus)<<(uint32_t)diskSpaceStatus;

    // TO DO: Need to verify if Free bytes have correct value

    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Abbreviate method.
// Expected Result: File name has no quotes so unicode should also not change
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, Abbreviate)
    {
    WCharCP fileName1 = L"Temporary.txt";
    WCharCP expectedName = L"...rary.txt";
    size_t maxLength = 12;

    BeFileName fromFileName1(fileName1);
        
    WString abbreviatedName = fromFileName1.Abbreviate (maxLength);
    //EXPECT_STREQ(fromFileName1.GetName(),fileName1)<< "After removing quotes\n"<<fromFileName1.GetName()<<"\n"<<"before removing quotes\n"<<fileName1 ;
    EXPECT_TRUE (0==wcscmp(expectedName, abbreviatedName.c_str()))<<"\n"<<abbreviatedName.c_str();

    WCharCP fileName2 = L"/dir1/dir2/dir3/dir4/dir5/dir6/test.txt";
    BeFileName fromFileName2 (fileName2);

    // check case where abbreviation length is greater than actual name length
    abbreviatedName = fromFileName2.Abbreviate (128);
    ASSERT_TRUE (0 == wcscmp (abbreviatedName.c_str(), fromFileName2.GetName()));

    // check case where abbreviation length equals actual name length
    abbreviatedName = fromFileName2.Abbreviate (fromFileName2.size());
    ASSERT_TRUE (0 == wcscmp (abbreviatedName.c_str(), fromFileName2.GetName()));

    // check cases where abbreviation length is less than actual name length
    abbreviatedName = fromFileName2.Abbreviate (fromFileName2.size()-1);
    ASSERT_LE (abbreviatedName.size(), fromFileName2.size()-1);

    abbreviatedName = fromFileName2.Abbreviate (12);
    ASSERT_LE (abbreviatedName.size(), (size_t) 12);

    // check impratical cases - arbitrary behavior is that no abbreviation happens
    abbreviatedName = fromFileName2.Abbreviate (3);
    ASSERT_TRUE (0 == wcscmp (abbreviatedName.c_str(), fromFileName2.GetName()));

    abbreviatedName = fromFileName2.Abbreviate (2);
    ASSERT_TRUE (0 == wcscmp (abbreviatedName.c_str(), fromFileName2.GetName()));
    }

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  2/14
// Desc: Testing of UNC path Support.
//---------------------------------------------------------------------------------------
TEST(BeFileNameTests, BeFileNameUNCPathTest)
{
#if defined (_WIN32)
    BeFileNameStatus status;
    //Vault2 is a shared location for Bentley Pakistan Coleagues on local server, used the path for local testing purposes not intended to be used globally
    BeFileName path(L"\\\\vault2\\Users\\HassanA\\GraphiteTesting");
    BeFileNameAccess accs = BeFileNameAccess::Write;

    // remove the directory if it exists
    if (BeFileName::DoesPathExist(path))
    {
        status = BeFileName::EmptyAndRemoveDirectory(path);
        ASSERT_TRUE(BeFileNameStatus::Success == status);
    }

    // make sure we can create a new directory
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::Success == status);
    ASSERT_TRUE(BeFileName::DoesPathExist(path));


    // make sure we get "alredy exists" flag when creating existing directory
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::AlreadyExists == status);
    if (BeFileName::DoesPathExist(path))
    {
        status = BeFileName::EmptyAndRemoveDirectory(path);
        ASSERT_TRUE(BeFileNameStatus::Success == status);
    }
    // make sure we can create directory tree
    path.AppendToPath(L"a");
    path.AppendToPath(L"b");
    path.AppendToPath(L"TestFile.txt");
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::Success == status) << (uint32_t)status;
    ASSERT_TRUE(BeFileName::DoesPathExist(path));

    WCharCP fullPath = path;
    BeFileName completePath(fullPath);

    BeFileNameStatus accessStatus = BeFileName::CheckAccess(fullPath, accs);

    EXPECT_TRUE(BeFileNameStatus::Success == accessStatus) << (uint32_t)accessStatus;
#endif

}


//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  2/14
// Desc: Testing of prefixed UNC path Support.
// 
//---------------------------------------------------------------------------------------
TEST(BeFileNameTests, UNCPrefixedTest)
{
#if defined (_WIN32)
    BeFileNameStatus status;

    // The complete path will be something like this: \\localhost\\Shareddir\\MyFolder\\SecondFolder\\AnotherFolder\\OneMoreFolder\\NeededFolder\\HisFodler\\HerFolder\\TooManyFolders\\PiledFolders\\thisfile.txt
    BeFileName path(L"\\\\?\\\\localhost\\thisfile.txt");
    BeFileNameAccess accs = BeFileNameAccess::Write;

    // remove the directory if it exists
    if (BeFileName::DoesPathExist(path))
    {
        status = BeFileName::EmptyAndRemoveDirectory(path);
        ASSERT_TRUE(BeFileNameStatus::Success == status);
    }

    // make sure we can create a new directory
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::Success == status);
    ASSERT_TRUE(BeFileName::DoesPathExist(path));


    // make sure we get "alredy exists" flag when creating existing directory
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::AlreadyExists == status);
    if (BeFileName::DoesPathExist(path))
    {
        status = BeFileName::EmptyAndRemoveDirectory(path);
        ASSERT_TRUE(BeFileNameStatus::Success == status);
    }
    // make sure we can create directory tree
    path.AppendToPath(L"a");
    path.AppendToPath(L"b");
    path.AppendToPath(L"TestFile.txt");
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::Success == status) << (uint32_t)status;
    ASSERT_TRUE(BeFileName::DoesPathExist(path));

    WCharCP fullPath = path;
    BeFileName completePath(fullPath);

    BeFileNameStatus accessStatus = BeFileName::CheckAccess(fullPath, accs);

    EXPECT_TRUE(BeFileNameStatus::Success == accessStatus) << (uint32_t)accessStatus;
#endif

}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  2/14
// Desc: Testing of prefixed UNC path Support.
// 
//---------------------------------------------------------------------------------------
TEST(BeFileNameTests, UNCReadOnlyTest)
{
#if defined (_WIN32)
    BeFileNameStatus status;

    // The complete path will be something like this: \\localhost\\Shareddir\\MyFolder\\SecondFolder\\AnotherFolder\\OneMoreFolder\\NeededFolder\\HisFodler\\HerFolder\\TooManyFolders\\PiledFolders\\thisfile.txt
    BeFileName path(L"\\\\?\\\\localhost\\Shareddir\\MyFolder\\SecondFolder\\AnotherFolder\\OneMoreFolder\\NeededFolder\\HisFodler\\HerFolder\\TooManyFolders\\PiledFolders\\thisfile.txt");
    BeFileNameAccess accs = BeFileNameAccess::Read;

    // remove the directory if it exists
    if (BeFileName::DoesPathExist(path))
    {
        status = BeFileName::EmptyAndRemoveDirectory(path);
        ASSERT_TRUE(BeFileNameStatus::Success == status);
    }

    // make sure we can create a new directory
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::Success == status);
    ASSERT_TRUE(BeFileName::DoesPathExist(path));


    // make sure we get "alredy exists" flag when creating existing directory
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::AlreadyExists == status);
    if (BeFileName::DoesPathExist(path))
    {
        status = BeFileName::EmptyAndRemoveDirectory(path);
        ASSERT_TRUE(BeFileNameStatus::Success == status);
    }
    // make sure we can create directory tree
    path.AppendToPath(L"a");
    path.AppendToPath(L"b");
    path.AppendToPath(L"TestFile.txt");
    status = BeFileName::CreateNewDirectory(path);
    ASSERT_TRUE(BeFileNameStatus::Success == status) << (uint32_t)status;
    ASSERT_TRUE(BeFileName::DoesPathExist(path));

    WCharCP fullPath = path;
    BeFileName completePath(fullPath);

    BeFileNameStatus accessStatus = BeFileName::CheckAccess(fullPath, accs);

    EXPECT_TRUE(BeFileNameStatus::Success == accessStatus) << (uint32_t)accessStatus;

    BeFileNameStatus fileNameStatus = path.BeDeleteFile();
    ASSERT_TRUE(BeFileNameStatus::Success == fileNameStatus);
    ASSERT_TRUE(!path.DoesPathExist());
#endif

}

//---------------------------------------------------------------------------------------
// @betest                                     Sam.Wilson       03/14
//---------------------------------------------------------------------------------------
TEST(BeFileNameTests, ReadOnly)
{
    // Create a file in the temp directory
    BeFileName tempFileName;
    BeTest::GetHost().GetTempDir (tempFileName);
    tempFileName.AppendToPath (L"BeFileNameTests_IsReadOnly.txt");
    tempFileName.BeDeleteFile();
    BeFile tempFile;
    ASSERT_TRUE (!tempFileName.DoesPathExist());
    ASSERT_TRUE (!tempFileName.IsFileReadOnly());
    ASSERT_TRUE (tempFile.Create (tempFileName.c_str()) == BeFileStatus::Success);
    ASSERT_TRUE (tempFile.Close() == BeFileStatus::Success);
    ASSERT_TRUE (tempFileName.DoesPathExist());

    //  Test Is/SetReadOnly
    ASSERT_TRUE (!tempFileName.IsFileReadOnly());
    ASSERT_TRUE (tempFileName.SetFileReadOnly(true) == BeFileNameStatus::Success);
    ASSERT_TRUE (tempFileName.IsFileReadOnly());

    //  Test BeDeleteFile's handling of RO file

#ifdef _WIN32 // Note: *nix will delete the file even if it's open.
    ASSERT_TRUE (tempFile.Open (tempFileName.c_str(), BeFileAccess::Read) == BeFileStatus::Success);
    ASSERT_TRUE (tempFileName.BeDeleteFile() != BeFileNameStatus::Success); // should fail because file is open.
    ASSERT_TRUE (tempFileName.IsFileReadOnly());
    ASSERT_TRUE (tempFile.Close() == BeFileStatus::Success);
#endif
    
    ASSERT_TRUE (tempFileName.IsFileReadOnly());
    ASSERT_TRUE (tempFileName.BeDeleteFile() == BeFileNameStatus::Success); // should blow right by the read-only status
    ASSERT_TRUE (!tempFileName.DoesPathExist());
}
//---------------------------------------------------------------------------------------
// @bsimethod                                         Umar.Hayat                    02/16
//---------------------------------------------------------------------------------------
#if defined (BENTLEY_WIN32)
static void SetupDirectory(BeFileNameCR root)
    {
    if (!root.DoesPathExist())
        BeFileName::CreateNewDirectory(root.c_str());

    BeFileName child1, child2;
    child1 = child2 = root;
    child1.AppendToPath(L"Child1");
    child2.AppendToPath(L"Child2");

    if (!child1.DoesPathExist())
        BeFileName::CreateNewDirectory(child1.c_str());
    if (!child2.DoesPathExist())
        BeFileName::CreateNewDirectory(child2.c_str());
    BeFile file0, file1, file2, file3;
    BeFileName fileName0, fileName1, fileName2;

    fileName0 = root;
    fileName0.AppendToPath(L"File0.txt");
    EXPECT_EQ(BeFileStatus::Success, file0.Create(fileName0, true));
    fileName1 = child1;
    fileName1.AppendToPath(L"File1.log");
    EXPECT_EQ(BeFileStatus::Success, file1.Create(fileName1, true));
    fileName2 = child2;
    fileName2.AppendToPath(L"File2.txt");
    file2.Create(fileName2, true);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                         Umar.Hayat                    02/16
//---------------------------------------------------------------------------------------
static void VerifyDirectory(BeFileNameCR root, bool SubFolderExpected = true)
    {
    ASSERT_TRUE(root.DoesPathExist());

    BeFileName child1, child2;
    child1 = child2 = root;
    BeFileName::AppendToPath(child1, L"Child1");
    BeFileName::AppendToPath(child2, L"Child2");
    ASSERT_TRUE(SubFolderExpected == child1.DoesPathExist());
    ASSERT_TRUE(SubFolderExpected == child2.DoesPathExist());

    BeFile file0, file1, file2, file3;
    BeFileName fileName0, fileName1, fileName2;

    fileName0 = root;
    BeFileName::AppendToPath(fileName0, L"File0.txt");
    fileName1 = child1;
    BeFileName::AppendToPath(fileName1, L"File1.log");
    fileName2 = child2;
    BeFileName::AppendToPath(fileName2, L"File2.txt");
    ASSERT_TRUE(fileName0.DoesPathExist());
    ASSERT_TRUE(SubFolderExpected == fileName1.DoesPathExist());
    ASSERT_TRUE(SubFolderExpected == fileName2.DoesPathExist());

    }
#endif

//---------------------------------------------------------------------------------------
// @betest                                          Umar.Hayat                    02/16
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, CloneDirectory)
    {
#if defined (BENTLEY_WIN32)

    BeFileName rootDir, srcDir, destDir1, destDir2;
    BeTest::GetHost().GetOutputRoot(rootDir);
    srcDir = destDir1 = destDir2 = rootDir;
    SetupDirectory(srcDir.AppendToPath(L"srcDir"));

    //--------------------------------------------------------------------------------------------------------------------
    // Clone with sub directry 
    EXPECT_TRUE(BeFileNameStatus::Success == BeFileName::CloneDirectory(srcDir, destDir1.AppendToPath(L"destDir1WithSub"), true));
    VerifyDirectory(destDir1);

    //--------------------------------------------------------------------------------------------------------------------
    // Clone without sub directories
    EXPECT_TRUE(BeFileNameStatus::Success == BeFileName::CloneDirectory(srcDir, destDir2.AppendToPath(L"destDir2"),false));
    VerifyDirectory(destDir2, false);

    //--------------------------------------------------------------------------------------------------------------------
    // Source Does not exist 
    EXPECT_TRUE(BeFileNameStatus::Success != BeFileName::CloneDirectory(BeFileName(rootDir).AppendToPath(L"DirNoExist"), BeFileName(rootDir).AppendToPath(L"DestDirNoExist")));

#endif
    }

//---------------------------------------------------------------------------------------
// @betest                                          Umar.Hayat                    02/16
//---------------------------------------------------------------------------------------
TEST(BeFileNameTests, IsAbsolutePath)
    {
#if defined (BENTLEY_WIN32)
    Utf8CP fileSpec = "C:\\dir\\dir\\somefile.txt";
#else
    Utf8CP fileSpec = "/var/dir/somefile.txt";
#endif
    BeFileName fileName(fileSpec, BentleyCharEncoding::Utf8);
    EXPECT_TRUE(fileName.IsAbsolutePath());

    BeFileName fileName2(fileName.GetFileNameAndExtension());
    EXPECT_FALSE(fileName2.IsAbsolutePath());
    }

//---------------------------------------------------------------------------------------
// @betest                                          Umar.Hayat                    02/16
//---------------------------------------------------------------------------------------
TEST (BeFileNameTests, PopDir)
    {
#if defined (BENTLEY_WIN32)
    CharCP fileSpec = "C:\\dir\\somefile.txt";
#else
    CharCP fileSpec = "/var/dir/somefile.txt";
#endif
    BeFileName fileName(fileSpec, BentleyCharEncoding::Locale);

#if defined (BENTLEY_WIN32)
    EXPECT_STREQ(L"C:\\dir\\somefile.txt", fileName.c_str());
    fileName.PopDir();
    EXPECT_STREQ(L"C:\\dir", fileName.c_str());
    fileName.PopDir();
    EXPECT_STREQ(L"C:", fileName.c_str());
    fileName.PopDir();
    EXPECT_STREQ(L"", fileName.c_str());
#else
    EXPECT_STREQ(L"/var/dir/somefile.txt", fileName.c_str());
    fileName.PopDir();
    EXPECT_STREQ(L"/var/dir", fileName.c_str());
    fileName.PopDir();
    EXPECT_STREQ(L"/var", fileName.c_str());
    fileName.PopDir();
    EXPECT_STREQ(L"", fileName.c_str());
#endif
    }