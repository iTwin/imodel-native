/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeStringUtilities_tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/ScopedArray.h>
#include <map>

#define VERIFY(X) ASSERT_TRUE(X)

#if defined (NOT_USED)
struct AW
    {
    char const* m_asc;
    AW (char const* a) : m_asc(a) {;}
    operator char const*() const {return m_asc;}
    };
#endif

//int s_dummy;

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Utf16WChar method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilUtf16WChar)
{

   uint16_t string1[]=  {48,0};
   WCharCP string2= L"0\0";
   Utf16CP string3= string1;

   int status= BeStringUtilities::CompareUtf16WChar(string3,string2);
   EXPECT_EQ(0,status);
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Utf16WCharHello method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilUtf16WCharHello)
{

   uint16_t string1[]=  {72,69,76,76,79,0};
   WCharCP string2= L"HELLO\0";
   Utf16CP string3= string1;

   int status= BeStringUtilities::CompareUtf16WChar(string3,string2);
   EXPECT_EQ(0,status);
  
   
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of WCharToUtf16 method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilWCharToUtf16)
{

  
   WCharCP string2= L"HELLO\0";
   Utf16Buffer utf16String ;
   Utf16BufferR stringUtf16= utf16String;
   size_t count= BeStringUtilities::AsManyAsPossible;

  BentleyStatus status= BeStringUtilities::WCharToUtf16(stringUtf16,string2, count);
  EXPECT_EQ(SUCCESS,status)<<status;
  
  
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of WCharToUtf16NotNull method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilWCharToUtf16NotNull)
{

  
   WCharCP string2= L"HELLO";
   Utf16Buffer utf16String ;
   Utf16BufferR stringUtf16= utf16String;
   size_t count= BeStringUtilities::AsManyAsPossible;

  BentleyStatus status= BeStringUtilities::WCharToUtf16(stringUtf16,string2, count);
  EXPECT_EQ(SUCCESS,status)<<status;
  
  
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Strlwr method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilStrlwr)// Failing on iOS
{

   char string[]= "HELLO";
   char* status= BeStringUtilities::Strlwr(string);
  
   EXPECT_STREQ("hello",status)<<status;
  
  
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Strupr method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilStrupr)//Failing on iOS
{
  char string[]= "hello";
  char* status= BeStringUtilities::Strupr(string);
  EXPECT_STREQ("HELLO",status)<<status;
  
  
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of CopyUtf16 method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilCopyUtf16)
{

   uint16_t string1[]=  {48,0};
   uint16_t string2[]= {27,0};
   Utf16CP inStr= string1;
  
   Utf16P outStr= string2;
   size_t outStrCapacity= BeStringUtilities::AsManyAsPossible;

   BeStringUtilities::CopyUtf16(outStr,outStrCapacity,inStr);
   //EXPECT_TRUE(outStr,inStr);
  

}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Stricmp method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilStricmp)
{
   char string1[]= "hello";
   char string2[]= "hello";

  int status= BeStringUtilities::Stricmp(string1,string2);
  EXPECT_EQ(0,status)<<status;
  
  
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Wmemcpy method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilWmemcpy)
{

  wchar_t dest[]= L"123456";
  const wchar_t src[]= L"DELETE";
 
   BeStringUtilities::Wmemcpy(dest, _countof(dest), src,  _countof(src));
   EXPECT_EQ(SUCCESS,BeStringUtilities::Wmemcpy(dest, _countof(dest), src,  _countof(src)));
	
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of Utf16Len method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringUtilUtf16Len)
{

   uint16_t string1[]=  {72,69,76,76,79,0};
   Utf16CP string3= string1;

   size_t status= BeStringUtilities::Utf16Len(string3);
   EXPECT_EQ(5,status);
  
  
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of CurrentLocaleToWChar method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringCurrentLocaleToWChar)
{
	
	char inChar[]= "Hello";
	WCharCP outWCharTest= L"Hello";
	wchar_t outWCharInitial[]= L"Hello";
	
    EXPECT_TRUE(0==wcscmp(outWCharTest,BeStringUtilities::CurrentLocaleCharToWChar(outWCharInitial,inChar,_countof(inChar) )));


}

TEST (BeStringUtilitiesTests, BeStringCurrentLocaleToWCharStatusCheck)
{
	
	char inChar[]= "Hello";
	WString outWCharInitial= L"Hello";
	
	BentleyStatus status= BeStringUtilities::CurrentLocaleCharToWChar(outWCharInitial,inChar );
    EXPECT_EQ(SUCCESS, status);

	
}

//---------------------------------------------------------------------------------------
// @betest                                     Hassan.Arshad                  10/13
// Desc: Testing of IsUri8Encoded method.
// 
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, BeStringIsUriEncoded)
{
	
	char initialStr[]= "Hello\0";
    Utf8CP str= initialStr;
	
    EXPECT_TRUE(BeStringUtilities::IsUriEncoded(str));
		
}

//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      03/14
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, TestWcsstr)
    {
    wchar_t wcs[] = L"This is a simple string";
    auto pwc = wcsstr (wcs,L"simple");
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (0==wcsncmp (pwc, L"simple", 6));

    pwc = wcsstr (wcs, L"sample");
    EXPECT_TRUE (NULL == pwc);

    pwc = wcsstr (wcs, wcs);
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (pwc == wcs);

    pwc = wcsstr (wcs, L"g");
    ASSERT_TRUE (NULL != pwc);
    EXPECT_TRUE (0==wcscmp (pwc, L"g"));    // pwc should be pointing to the g at the end of the string
    }

#include <cwctype>
//---------------------------------------------------------------------------------------
// @betest                                      Sam.Wilson                      03/14
//---------------------------------------------------------------------------------------
TEST (BeStringUtilitiesTests, StrLwr)
    {
    char asc[] = "ASCII";
    BeStringUtilities::Strlwr (asc);
    EXPECT_STREQ( asc, "ascii" );

    // Start with a non-ascii string. In this case, it has no lower-case version.
    WCharCP nonasc = L"\u20AC"; // this is the Euro symbol
    //  Convert to UTF8 and lowercase it
    Utf8String nonasc_utf8 (nonasc);    // s/ be E2 82 AC 00
    EXPECT_FALSE( nonasc_utf8.IsAscii() );
    nonasc_utf8.ToLower();
    WString nonascAfter (nonasc_utf8.c_str(), BentleyCharEncoding::Utf8);
    EXPECT_STREQ (nonascAfter.c_str(), nonasc); // s/ be a nop
    // Do the same thing using a char array
    ScopedArray<char> nonascbuf (nonasc_utf8.size() + 1);
    strcpy (nonascbuf.GetData(), nonasc_utf8.c_str());
    BeStringUtilities::Strlwr (nonascbuf.GetData());
    WString nonascAfterBuf (nonascbuf.GetData(), BentleyCharEncoding::Utf8);
    EXPECT_STREQ (nonascAfterBuf.c_str(), nonasc);

#ifdef COMMENT_OUT // *** does not work. depends on locale
    //  Now make sure that tolower and comparei agree. Use a non-ascii string that has a different lowercase version.
    wchar_t Ntilde[] = L"\u00D1";
    wchar_t ntilde[] = L"\u00F1";

    EXPECT_TRUE( std::towlower (*Ntilde) == *ntilde );

    EXPECT_TRUE(                    wcscmp  (Ntilde, ntilde) != 0 );
    EXPECT_TRUE( BeStringUtilities::Wcsicmp (Ntilde, ntilde) == 0 );
    wchar_t wbuf[_countof(Ntilde)];
    wcscpy (wbuf, Ntilde);
    BeStringUtilities::Wcslwr (wbuf);
    EXPECT_TRUE( wcscmp (ntilde, wbuf) == 0 );
    EXPECT_TRUE( wcscmp (Ntilde, wbuf) != 0 );
    EXPECT_TRUE( BeStringUtilities::Wcsicmp (Ntilde, wbuf) == 0 );

    WCharCP nonascwithcase = L"\u00D1"; // Ntilde
    WString nonascwithcaseWString (nonascwithcase);
    WString nonascwithcaseWStringLower (nonascwithcaseWString);
    nonascwithcaseWStringLower.ToLower(); // s/b \u00F1 ntilde
    EXPECT_TRUE( nonascwithcaseWString.CompareTo  (nonascwithcaseWStringLower) != 0 );
    EXPECT_TRUE( nonascwithcaseWString.CompareToI (nonascwithcaseWStringLower) == 0 );

    Utf8String nonascwithcaseUtf8String (nonascwithcase); // s/be 0xC3 0x91 
    Utf8String nonascwithcaseUtf8StringLower (nonascwithcaseUtf8String);
    nonascwithcaseUtf8StringLower.ToLower(); // s/be 0xC3 0xB1
    EXPECT_TRUE( nonascwithcaseUtf8String.CompareTo (nonascwithcaseUtf8StringLower) != 0 );
    EXPECT_TRUE( nonascwithcaseUtf8String.CompareToI(nonascwithcaseUtf8StringLower) == 0 );
#endif
    }
