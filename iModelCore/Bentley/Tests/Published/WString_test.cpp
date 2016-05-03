/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/WString_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>
#include <thread>
#include <map>
#include <vector>

#define VERIFY(X) ASSERT_TRUE(X)
#define TESTDATA_StringW                    L"ThisIsATest!@#$%^&*()-="

#if defined (NOT_USED)
struct AW
    {
    char const* m_asc;
    AW (char const* a) : m_asc(a) {;}
    operator char const*() const {return m_asc;}
    };
#endif

int s_dummy;

//  Make sure that all platforms interpret precision specifier correctly.
//  Note that you can only ask for 16 1/2 significant digits. Anything beyond that is undefined/platform-specific.
TEST(WStringTest,SprintfFloatPrecision)
    {
    double d = 1.1999999999999989;
    WString str;
    str.Sprintf (L"%0.16lf", d);// 1234567890123456
    ASSERT_STREQ( str.c_str(), L"1.1999999999999988" ) << L"   actually got " << str.c_str();
    
    WString strg;
    strg.Sprintf (L"%0.17lg", d);// 1234567890123456
    ASSERT_STREQ( strg.c_str(), L"1.1999999999999988" ) << L"   actually got " << str.c_str();
    }

TEST(WStringTest,emptyStrings)
    {
    // WString(), WString(L""), and WString((WCharCP)NULL) are all ways of defining the empty string.
    WString se1, se2;
    ASSERT_TRUE(se1==se2);

    WString sn1((WCharCP)NULL);
    WString sn2((WCharCP)NULL);
    ASSERT_TRUE(sn1==sn2);
    
    ASSERT_TRUE (se1 == sn1);

    WString sl (L"");
    ASSERT_TRUE (se1 == sl);

    ASSERT_TRUE (se1.empty());
    ASSERT_TRUE (sn1.empty());
    ASSERT_TRUE (sl.empty());
    }

TEST(WStringTest,append)
    {
    WString str;
    int i=1;
    str.append (1, static_cast <wchar_t> (i + L'0'));
    }

TEST(WStringTest,CompareTo)
    {
    WString str (L"abc 123 def 456");
    WCharCP str2;
    if (s_dummy != 0)
        str2 = L"can't happen";
    else
        str2 = L"abc 123 def 456 and more";
    int c = str.CompareTo (str2);
    ASSERT_TRUE (c < 0);
    }

//---------------------------------------------------------------------------------------
//                                       Jeff.Marker                 10/14
//---------------------------------------------------------------------------------------
static void initBeStringUtilities()
    {
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    BeStringUtilities::Initialize(assetsDir);
    }

TEST(WStringTest, ToLowerEmpty)
    {
    WString a;
    ASSERT_TRUE( a.empty() );
    a.ToLower ();
    ASSERT_TRUE( a.empty() );

    WString o;
    o.assign (a);
    ASSERT_TRUE( o.empty() );
    o.ToLower ();
    ASSERT_TRUE( a.empty() );
    ASSERT_TRUE( o.empty() );
    }

TEST(WStringTest, Mutate)
    {
    WString a (L"ABC");
    WString o;
    o.assign (a);
    o.ToLower ();
    ASSERT_STREQ ( o.c_str(), L"abc" );
    ASSERT_STREQ ( a.c_str(), L"ABC" );
    }

TEST(WStringTest, ToLowerToUpper)
    {
    WString str;
    str = L"StrIng";
    str.ToLower();
    ASSERT_STREQ( str.c_str(), L"string");
    str = L"StrIng";
    str.ToUpper();
    ASSERT_STREQ( str.c_str(), L"STRING");
    // To Upper Non Ascii
    WCharP nonasc = L"\u20AC"; // this is the Euro symbol
    //  Convert to UTF8 and lowercase it
    WString nonasc_wchar(nonasc);    // s/ be E2 82 AC 00
    EXPECT_STREQ(nonasc, nonasc_wchar.ToUpper().c_str()); // s/ be a nop
    }

// ******************************************************
// ******************************************************
// *** TEST BENTLEY ADDED CONSTRUCTOR
// ******************************************************
// ******************************************************

TEST(WStringTest, BentleyConstructorTest)
{
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();

    //  The following verifies that our constructors are all unambiguous when the templates are instantiated with wchar_t and char, including
    //  our Bentley-specific constructor that allows a WString to be constructed directly from an ASCII string.

    char const* asc = "abc";
    wchar_t const* uni = L"abc";

        // WString

    //  Construct from ASCII C string
    WString wa ("abc", false); VERIFY( wa == L"abc" );
    WString waUtf8 ("abc", true); VERIFY( waUtf8 == L"abc" );
    WString wa1 (asc, false);  VERIFY( wa1 == L"abc" );
#if defined (DOES_NOT_WORK)   // *** this WString ctor is templated on the argument char type. You cannot deduce a template parameter from a cast operator.
    AW aw ("abc");
    WString waw (aw);
#endif
    //  Construct from Unicode C string
    WString w (L"abc"); VERIFY( w == L"abc" );
    WString w1 (uni);   VERIFY( w1 == L"abc" ); VERIFY( w1 == uni );
    //  Copy constructor
    WString wcc (w1);   VERIFY( wcc == w1 );    VERIFY( wcc == L"abc" );
}

// ******************************************************
// ******************************************************
// *** TEST BENTLEY ADDED METHODS
// ******************************************************
// ******************************************************
TEST(WStringTest, CharToMSWChar)
    {
    // The first caller to convert strings in a process has to ensure BeStringUtilities::Initialize is called.
    initBeStringUtilities();
    
    WString str ("abc", false);
    VERIFY( str == L"abc" );
    VERIFY( str == L"abc" );
    VERIFY( str == WString("abc", false) );
    VERIFY( str.length() == 3 );

    VERIFY( str.GetMaxLocaleCharBytes() == (3 + 1)*sizeof(wchar_t) ); // 3 chars + 0
    char* asc = (char*)_alloca (str.GetMaxLocaleCharBytes());
    str.ConvertToLocaleChars (asc);
    VERIFY( 0 == strcmp(asc, "abc") );
    VERIFY( str == WString(asc, false) );

    str.AppendA ("def");
    VERIFY( str == L"abcdef" );
    VERIFY( str.length() == 6 );

    str.AppendA(nullptr);
    VERIFY(str == L"abcdef");
    VERIFY(str.length() == 6);

    }

TEST(WStringTest, Utils)
    {
    WString str (L" abc ");
    VERIFY (str.c_str() == str.c_str() );
    VERIFY (str.CompareTo(L" abc ") == 0 );
    VERIFY (str.CompareToI(L" ABC ") == 0 );
    VERIFY (str.Equals(L" abc ") );
    VERIFY (str.EqualsI(L" abc ") );
    VERIFY (str.EqualsI(L" ABC ") );
    WString cc (str);
    VERIFY( str == cc );
    VERIFY( str.length() == 5 );
    WString trimmed (str);
    trimmed.Trim ();
    VERIFY( str == cc );
    VERIFY( trimmed == L"abc" );
    VERIFY( trimmed.length() == 3 );

    WString str2 (L"a Test a");
    WString cc2 (str2);
    WString trimmed2 (str2);
    trimmed2.Trim (L"a");
    VERIFY (str2 == cc2);
    VERIFY (trimmed2 == L" Test ");
    VERIFY (trimmed2.length() == 6);

    WString str3(L"");
    VERIFY( 0 == str3.Trim().length());
    VERIFY(0 == str3.Trim(L"; .").length());

    WString newValue(L"New Test String");
    Utf16Buffer newValueUtf16;
    BeStringUtilities::WCharToUtf16(newValueUtf16, newValue.c_str(), BeStringUtilities::AsManyAsPossible);
    WString str4((Utf16CP)newValueUtf16.data());
    VERIFY(str4 == L"New Test String");

    }

TEST(WStringTest, Operators)
    {
    WString a (L"abc");
    WString z (L"zyx");
    VERIFY( a != z );
    VERIFY( a < z );
    VERIFY( a <= z );
    VERIFY( z > a );
    }

TEST(WStringTest, WStringsInContainers)
    {
    bvector<WString> v;
    v.push_back (L"abc");
    v.push_back (L"def");
    VERIFY( v.size() == 2 );
    VERIFY( v[0] == L"abc" );
    VERIFY( v[1] == L"def" );

    std::map<WString,int> wsm;
    wsm[L"def"] = 1;
    wsm[L"abc"] = 2;
    std::map<WString,int>::const_iterator it = wsm.find (L"def");
    VERIFY( it != wsm.end() );
    VERIFY( it->second == 1 );
    VERIFY( wsm[L"abc"] == 2 );
    }

// ******************************************************
// ******************************************************
// *** GCC basic_string tests
// ******************************************************
// ******************************************************

//  The following tests were adapted from unit tests that are part of GCC 
// Copyright (C) 1999, 2000, 2001, 2002, 2003, 2009
// Free Software Foundation, Inc.


TEST(WStringTest, ConstructorTest01)
{
  typedef WString::size_type csize_type;
  typedef WString::iterator citerator;
  csize_type npos = WString::npos;
  csize_type csz01;

  const wchar_t str_lit01[] = L"rodeo beach, marin";
  const WString str01(str_lit01);
  const WString str02(L"baker beach, san francisco");

  // basic_string(const WString&, size_type pos = 0, siz_type n = npos, alloc)
  csz01 = str01.size();
#ifndef NO_CPP_EXCEPTION_HANDLERS
  try {
    WString str03(str01, csz01 + 1);
    VERIFY( false );
  }         
  catch(std::out_of_range&) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }
#endif

  try {
    WString str03(str01, csz01);
    VERIFY( str03.size() == 0 );
    VERIFY( str03.size() <= str03.capacity() );
  }         
  catch(...) {
    VERIFY( false );
  }

#ifndef NO_CPP_EXCEPTION_HANDLERS
  // basic_string(const wchar_t* s, size_type n, alloc)
  csz01 = str01.max_size();
  // NB: As strlen(str_lit01) != csz01, this test is undefined. It
  // should not crash, but what gets constructed is a bit arbitrary.
  try {
    WString str03(str_lit01, csz01 + 1);
    VERIFY( true );
  }         
  catch(std::length_error&) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }
  
  // NB: As strlen(str_lit01) != csz01, this test is undefined. It
  // should not crash, but what gets constructed is a bit arbitrary.
  // The "maverick's" of all string objects.
  try {
    WString str04(str_lit01, npos); 
    VERIFY( true );
  }         
  catch(std::length_error&) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }
#endif

#if defined (BEIJING_WIP_WSTRING) // this will blow up unless we change allocator to throw
  // Build a maxsize - 1 lengthed string consisting of all A's
  try {
    WString str03(csz01 - 1, 'A');
    VERIFY( str03.size() == csz01 - 1 );
    VERIFY( str03.size() <= str03.capacity() );
  }         
  // NB: bad_alloc is regrettable but entirely kosher for
  // out-of-memory situations.
  catch(std::length_error&) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }
#endif

  // basic_string(const wchar_t* s, const allocator& a = allocator())
  WString str04(str_lit01);
  VERIFY( str01 == str04 );


#if defined (BEIJING_WIP_WSTRING) // this will blow up unless we change allocator to throw
  // basic_string(size_type n, char c, const allocator& a = allocator())
  csz01 = str01.max_size();
  try {
    WString str03(csz01 + 1, L'z');
    VERIFY( false );
  }         
  catch(std::length_error&) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }

  try {
    WString str04(npos, L'b'); // the "maverick's" of all string objects.
    VERIFY( false );
  }         
  catch(std::length_error&) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }

  try {
    WString str03(csz01 - 1, L'z');
    VERIFY( str03.size() != 0 );
    VERIFY( str03.size() <= str03.capacity() );
  }         
  // NB: bad_alloc is regrettable but entirely kosher for
  // out-of-memory situations.
  catch(std::length_error&) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }
#endif


  // template<typename _InputIter>
  //   basic_string(_InputIter begin, _InputIter end, const allocator& a)
  WString str06(str01.begin(), str01.end());
  VERIFY( str06 == str01 );
}


TEST(WStringTest, ConstructorTest02)
{
  // template<typename _InputIter>
  //   basic_string(_InputIter begin, _InputIter end, const allocator& a)
  // where _InputIter is integral [21.3.1 para 15]
  WString s(10, 0);
  VERIFY( s.size() == 10 );
}

TEST(WStringTest, ConstructorTest03)
{
  const wchar_t* with_nulls = L"This contains \0 a zero Byte.";

  // These are tests to see how basic_string handles data with NUL
  // bytes.  Obviously basic_string(char*) will halt at the first one, but
  // nothing else should.
  WString s1 (with_nulls, 28);
  VERIFY( s1.size() == 28 );
  WString s2 (s1);
  VERIFY( s2.size() == 28 );

#if defined (BEIJING_WIP_WSTRING) // wcslen does not throw
  // Not defined, but libstdc++ throws an exception.
  const wchar_t* bogus = 0;
  try 
    {
      WString str1(bogus);
      VERIFY( false );
    }         
  catch(std::exception&) 
    {
      VERIFY( true );
    }

  // Not defined, but libstdc++ throws an exception.
  try 
    {
      WString str2(bogus, 5);
      VERIFY( false );
    }         
  catch(std::exception&) 
    {
      VERIFY( true );
    }
#endif
}

// http://gcc.gnu.org/ml/libstdc++/2002-06/msg00025.html
TEST(WStringTest, ConstructorTest04)
{
  WString str01(L"portofino");

  WString::reverse_iterator i1 = str01.rbegin();
  WString::reverse_iterator i2 = str01.rend();
  WString str02(i1, i2);
  VERIFY( str02 == L"onifotrop" );
}

//#ifdef DGNV10FORMAT_CHANGES_WIP
//// libstdc++/8347
//TEST(WStringTest, ConstructorTest05)
//{
//  bool test = true;
//
//  std::vector<wchar_t> empty;
//  WString empty2(empty.begin(), empty.end());
//
//  // libstdc++/8716 (same underlying situation, same fix)
//  wchar_t const * s = NULL;
//  WString zero_length_built_with_NULL(s,0);
//}
//#endif

// libstdc++/42261
TEST(WStringTest, ConstructorTest99)
{
  const WString s(WString::size_type(6), WString::value_type(L'f'));
  VERIFY( s == L"ffffff" );
}


TEST(WStringTest, ElementAccess01)
{
  typedef WString::size_type csize_type;
  typedef WString::const_reference cref;
  typedef WString::reference ref;
  csize_type csz01, csz02;

  const WString str01(L"tamarindo, costa rica");
  WString str02(L"41st street beach, capitola, california");
  WString str03;

  // const_reference operator[] (size_type pos) const;
  csz01 = str01.size();
  cref cref1 = str01[csz01 - 1];
  VERIFY( cref1 == L'a' );
  cref cref2 = str01[csz01];
  VERIFY( cref2 == wchar_t() );

  // reference operator[] (size_type pos);
  csz02 = str02.size();
  ref ref1 = str02[csz02 - 1];
  VERIFY( ref1 == L'a' );
  ref ref2 = str02[1];
  VERIFY( ref2 == L'1' );

  // const_reference at(size_type pos) const;
  csz01 = str01.size();
  cref cref3 = str01.at(csz01 - 1);
  VERIFY( cref3 == L'a' );
#ifndef NO_CPP_EXCEPTION_HANDLERS
  try {
    str01.at(csz01);
    VERIFY( false ); // Should not get here, as exception thrown.
  }
  catch(std::out_of_range& ) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }
#endif

  // reference at(size_type pos);
  csz01 = str02.size();
  ref ref3 = str02.at(csz02 - 1);
  VERIFY( ref3 == L'a' );
#ifndef NO_CPP_EXCEPTION_HANDLERS
  try {
    str02.at(csz02);
    VERIFY( false ); // Should not get here, as exception thrown.
  }
  catch(std::out_of_range& ) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }
#endif
}

// Do a quick sanity check on known problems with element access and
// ref-counted strings. These should all pass, regardless of the
// underlying string implementation, of course.
TEST(WStringTest, ElementAccess02)
{
  bool test = true;
  typedef WString::size_type csize_type;
  typedef WString::iterator siterator;
  typedef WString::reverse_iterator sriterator;
  csize_type csz01, csz02;
  siterator it1;
  sriterator rit1;  

  WString str01(L"montara beach, half moon bay");
  const WString str02(L"ocean beach, san francisco");
  WString str03;

  // 21.3 p 5

  // References, pointers, and iterators referring to the elements of
  // a basic_string may be invalidated by the following uses of that
  // basic_string object:

  // ...

  // Susequent to any of the above uses except the forms of insert()
  // and erase() which return iterators, the first call to non-const
  // member functions operator[](), at(), begin(), rbegin(), end(), or
  // rend()

  str03 = str01;
  it1 = str01.begin();
  *it1 = L'x';
  VERIFY( str01[0] == L'x' );
  VERIFY( str03[0] == L'm' );

  str03 = str01; 
  csz01 = str01.size();
  rit1 = str01.rbegin(); // NB: Pointing at one-past the end, so ...
  *rit1 = L'z';      // ... but it's taken care of here 
  VERIFY( str01[csz01 - 1] == L'z' );
  VERIFY( str03[csz01 - 1] == L'y' );

  str03 = str01;
  csz01 = str01.size();
  WString::reference r1 = str01.at(csz01 - 2);
  VERIFY( str03 == str01 );
  r1 = L'd';
  VERIFY( str01[csz01 - 2] == L'd' );
  VERIFY( str03[csz01 - 2] == L'a' );

  str03 = str01; 
  csz01 = str01.size();
  WString::reference r2 = str01[csz01 - 3];
  VERIFY( str03 == str01 );
  r2 = L'w'; 
  VERIFY( str01[csz01 - 3] == L'w' );
  VERIFY( str03[csz01 - 3] == L'b' );

  str03 = str01;
  csz02 = str01.size();
  it1 = str01.end();
  VERIFY( str03 == str01 );
  --it1;
  *it1 = L'q'; 
  VERIFY( str01[csz02 - 1] == L'q' );
  VERIFY( str03[csz02 - 1] == L'z' );

  str03 = str01;
  rit1 = str01.rend();
  VERIFY( str03 == str01 );
  --rit1;     
  *rit1 = L'p'; 
  VERIFY( str01[0] == L'p' );
  VERIFY( str03[0] == L'x' );

  // need to also test for const begin/const end
  VERIFY(test);
}

// Do another sanity check, this time for member functions that return
// iterators, namely insert and erase.
TEST(WStringTest, ElementAccess03)
{
  typedef WString::size_type csize_type;
  typedef WString::iterator siterator;
  typedef WString::reverse_iterator sriterator;
  sriterator rit1;  

  const WString str01(L"its beach, santa cruz");

  WString str02 = str01;
  WString str05 = str02; // optional, so that begin below causes a mutate
  WString::iterator p = str02.insert(str02.begin(), L' ');
  WString str03 = str02;
  VERIFY( str03 == str02 );
  *p = L'!';
  VERIFY( *str03.c_str() == L' ' );
  str03[0] = L'@';
  VERIFY( str02[0] == L'!' );
  VERIFY( *p == L'!' );
  VERIFY( str02 != str05 );
  VERIFY( str02 != str03 );

  WString str10 = str01;
  WString::iterator p2 = str10.insert(str10.begin(), L'a');
  WString str11 = str10;
  *p2 = L'e';
  VERIFY( str11 != str10 );

  WString str06 = str01;
  WString str07 = str06; // optional, so that begin below causes a mutate
  p = str06.erase(str06.begin());
  WString str08 = str06;
  VERIFY( str08 == str06 );
  *p = L'!';
  VERIFY( *str08.c_str() == L't' );
  str08[0] = L'@';
  VERIFY( str06[0] == L'!' );
  VERIFY( *p == L'!' );
  VERIFY( str06 != str07 );
  VERIFY( str06 != str08 );

  WString str12 = str01;
  p2 = str12.erase(str12.begin(), str12.begin() + str12.size() - 1);
  WString str13 = str12;
  *p2 = L'e';
  VERIFY( str12 != str13 );
}

// http://gcc.gnu.org/ml/libstdc++/2004-01/msg00184.html
TEST(WStringTest, ElementAccess04)
{
  for (int i = 0; i < 2000; ++i)
    {
      WString str_01;

      for (int j = 0; j < i; ++j)
    str_01 += L'a';

      str_01.reserve(i + 10);

      const WString str_02(str_01);
      VERIFY( str_02[i] == L'\0' );
    }
}

// as per 21.3.4
TEST(WStringTest, ElementAccess05)
{
  {
    WString empty;
    wchar_t c = empty[0];
    VERIFY( c == wchar_t() );
  }

  {
    const WString empty;
    wchar_t c = empty[0];
    VERIFY( c == wchar_t() );
  }
}

TEST(WStringTest, Find01)
{
  typedef WString::size_type csize_type;
  typedef WString::const_reference cref;
  typedef WString::reference ref;
  csize_type npos = WString::npos;
  csize_type csz01, csz02;

  const wchar_t str_lit01[] = L"mave";
  const WString str01(L"mavericks, santa cruz");
  WString str02(str_lit01);
  WString str03(L"s, s");
  WString str04;

  // size_type find(const wstring&, size_type pos = 0) const;
  csz01 = str01.find(str01);
  VERIFY( csz01 == 0 );
  csz01 = str01.find(str01, 4);
  VERIFY( csz01 == npos );
  csz01 = str01.find(str02, 0);
  VERIFY( csz01 == 0 );
  csz01 = str01.find(str02, 3);
  VERIFY( csz01 == npos );
  csz01 = str01.find(str03, 0);
  VERIFY( csz01 == 8 );
  csz01 = str01.find(str03, 3);
  VERIFY( csz01 == 8 );
  csz01 = str01.find(str03, 12);
  VERIFY( csz01 == npos );

  // An empty string consists of no characters
  // therefore it should be found at every point in a string,
  // except beyond the end
  csz01 = str01.find(str04, 0);
  VERIFY( csz01 == 0 );
  csz01 = str01.find(str04, 5);
  VERIFY( csz01 == 5 );
  csz01 = str01.find(str04, str01.size());
  VERIFY( csz01 == str01.size() ); 
  csz01 = str01.find(str04, str01.size()+1);
  VERIFY( csz01 == npos ); 
  
  // size_type find(const wchar_t* s, size_type pos, size_type n) const;
  csz01 = str01.find(str_lit01, 0, 3);
  VERIFY( csz01 == 0 );
  csz01 = str01.find(str_lit01, 3, 0);
  VERIFY( csz01 == 3 );

  // size_type find(const wchar_t* s, size_type pos = 0) const;
  csz01 = str01.find(str_lit01);
  VERIFY( csz01 == 0 );
  csz01 = str01.find(str_lit01, 3);
  VERIFY( csz01 == npos );

  // size_type find(wchar_t c, size_type pos = 0) const;
  csz01 = str01.find(L'z');
  csz02 = str01.size() - 1;
  VERIFY( csz01 == csz02 );
  csz01 = str01.find(L'/');
  VERIFY( csz01 == npos );
}

// 21.3.6.2 basic_string rfind
TEST(WStringTest, rfind01)
{
  typedef WString::size_type csize_type;
  typedef WString::const_reference cref;
  typedef WString::reference ref;
  csize_type npos = WString::npos;
  csize_type csz01, csz02;

  const wchar_t str_lit01[] = L"mave";
  const WString str01(L"mavericks, santa cruz");
  WString str02(str_lit01);
  WString str03(L"s, s");
  WString str04;

  // size_type rfind(const wstring&, size_type pos = 0) const;
  csz01 = str01.rfind(str01);
  VERIFY( csz01 == 0 );
  csz01 = str01.rfind(str01, 4);
  VERIFY( csz01 == 0 );
  csz01 = str01.rfind(str02,3);
  VERIFY( csz01 == 0 );
  csz01 = str01.rfind(str02);
  VERIFY( csz01 == 0 );
  csz01 = str01.rfind(str03);
  VERIFY( csz01 == 8 );
  csz01 = str01.rfind(str03, 3);
  VERIFY( csz01 == npos );
  csz01 = str01.rfind(str03, 12);
  VERIFY( csz01 == 8 );

  // An empty string consists of no characters
  // therefore it should be found at every point in a string,
  // except beyond the end
  csz01 = str01.rfind(str04, 0);
  VERIFY( csz01 == 0 );
  csz01 = str01.rfind(str04, 5);
  VERIFY( csz01 == 5 );
  csz01 = str01.rfind(str04, str01.size());
  VERIFY( csz01 == str01.size() );
  csz01 = str01.rfind(str04, str01.size()+1);
  VERIFY( csz01 == str01.size() );

  // size_type rfind(const wchar_t* s, size_type pos, size_type n) const;
  csz01 = str01.rfind(str_lit01, 0, 3);
  VERIFY( csz01 == 0 );
  csz01 = str01.rfind(str_lit01, 3, 0);
  VERIFY( csz01 == 3 );

  // size_type rfind(const wchar_t* s, size_type pos = 0) const;
  csz01 = str01.rfind(str_lit01);
  VERIFY( csz01 == 0 );
  csz01 = str01.rfind(str_lit01, 3);
  VERIFY( csz01 == 0 );

  // size_type rfind(wchar_t c, size_type pos = 0) const;
  csz01 = str01.rfind(L'z');
  csz02 = str01.size() - 1;
  VERIFY( csz01 == csz02 );
  csz01 = str01.rfind(L'/');
  VERIFY( csz01 == npos );
}

TEST(WStringTest, Insert01)
{
  typedef WString::size_type csize_type;
  typedef WString::iterator citerator;
  csize_type csz01, csz02;

  const WString str01(L"rodeo beach, marin");
  const WString str02(L"baker beach, san francisco");
  WString str03;

#if defined (BEIJING_WIP_WSTRING) // allocator does not throw
  // wstring& insert(size_type p1, const wstring& str, size_type p2, size_type n)
  // requires:
  //   1) p1 <= size()
  //   2) p2 <= str.size()
  //   3) rlen = min(n, str.size() - p2)
  // throws:
  //   1) out_of_range if p1 > size() || p2 > str.size()
  //   2) length_error if size() >= npos - rlen
  // effects:
  // replaces *this with new wstring of length size() + rlen such that
  // nstr[0]  to nstr[p1] == thisstr[0] to thisstr[p1]
  // nstr[p1 + 1] to nstr[p1 + rlen] == str[p2] to str[p2 + rlen]
  // nstr[p1 + 1 + rlen] to nstr[...] == thisstr[p1 + 1] to thisstr[...]  
  str03 = str01; 
  csz01 = str03.size();
  csz02 = str02.size();
  try {
    str03.insert(csz01 + 1, str02, 0, 5);
    VERIFY( false );
  }         
  catch(std::out_of_range& fail) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }

  str03 = str01; 
  csz01 = str03.size();
  csz02 = str02.size();
  try {
    str03.insert(0, str02, csz02 + 1, 5);
    VERIFY( false );
  }         
  catch(std::out_of_range& fail) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }

  csz01 = str01.max_size();
  try {
    WString str04(csz01, L'b'); 
    str03 = str04; 
    csz02 = str02.size();
    try {
      str03.insert(0, str02, 0, 5);
      VERIFY( false );
    }         
    catch(std::length_error& fail) {
      VERIFY( true );
    }
    catch(...) {
      VERIFY( false );
    }
  }
  catch(std::bad_alloc& failure){
    VERIFY( true ); 
  }
  catch(std::exception& failure){
    VERIFY( false );
  }
#endif

  str03 = str01; 
  csz01 = str03.size();
  csz02 = str02.size();
  str03.insert(13, str02, 0, 12); 
  VERIFY( str03 == L"rodeo beach, baker beach,marin" );

  str03 = str01; 
  csz01 = str03.size();
  csz02 = str02.size();
  str03.insert(0, str02, 0, 12); 
  VERIFY( str03 == L"baker beach,rodeo beach, marin" );

  str03 = str01; 
  csz01 = str03.size();
  csz02 = str02.size();
  str03.insert(csz01, str02, 0, csz02); 
  VERIFY( str03 == L"rodeo beach, marinbaker beach, san francisco" );

  // wstring& insert(size_type __p, const wstring& wstr);
  // insert(p1, str, 0, npos)
  str03 = str01; 
  csz01 = str03.size();
  csz02 = str02.size();
  str03.insert(csz01, str02); 
  VERIFY( str03 == L"rodeo beach, marinbaker beach, san francisco" );

  str03 = str01; 
  csz01 = str03.size();
  csz02 = str02.size();
  str03.insert(0, str02); 
  VERIFY( str03 == L"baker beach, san franciscorodeo beach, marin" );

  // wstring& insert(size_type __p, const wchar_t* s, size_type n);
  // insert(p1, wstring(s,n))
  str03 = str02; 
  csz01 = str03.size();
  str03.insert(0, L"-break at the bridge", 20); 
  VERIFY( str03 == L"-break at the bridgebaker beach, san francisco" );

  // wstring& insert(size_type __p, const wchar_t* s);
  // insert(p1, wstring(s))
  str03 = str02; 
  str03.insert(0, L"-break at the bridge"); 
  VERIFY( str03 == L"-break at the bridgebaker beach, san francisco" );

  // wstring& insert(size_type __p, size_type n, wchar_t c)
  // insert(p1, wstring(n,c))
  str03 = str02; 
  csz01 = str03.size();
  str03.insert(csz01, 5, L'z'); 
  VERIFY( str03 == L"baker beach, san franciscozzzzz" );

  // iterator insert(iterator p, wchar_t c)
  // inserts a copy of c before the character referred to by p
  str03 = str02; 
  citerator cit01 = str03.begin();
  str03.insert(cit01, L'u'); 
  VERIFY( str03 == L"ubaker beach, san francisco" );

  // iterator insert(iterator p, size_type n,  wchar_t c)
  // inserts n copies of c before the character referred to by p
  str03 = str02; 
  cit01 = str03.begin();
  str03.insert(cit01, 5, L'u'); 
  VERIFY( str03 == L"uuuuubaker beach, san francisco" );

  // template<inputit>
  //   void 
  //   insert(iterator p, inputit first, inputit, last)
  // ISO-14882: defect #7 part 1 clarifies this member function to be:
  // insert(p - begin(), wstring(first,last))
  str03 = str02; 
  csz01 = str03.size();
  str03.insert(str03.begin(), str01.begin(), str01.end()); 
  VERIFY( str03 == L"rodeo beach, marinbaker beach, san francisco" );

  str03 = str02; 
  csz01 = str03.size();
  str03.insert(str03.end(), str01.begin(), str01.end()); 
  VERIFY( str03 == L"baker beach, san franciscorodeo beach, marin" );
}

TEST(WStringTest, Operations01)
{
  WString empty;

  // data() for size == 0 is non-NULL.
  VERIFY( empty.size() == 0 );
  const WString::value_type* p = empty.data();
  VERIFY( p != NULL );
}

TEST(WStringTest, Replace01)
{
  typedef WString::size_type csize_type;
  typedef WString::const_reference cref;
  typedef WString::reference ref;

  const wchar_t str_lit01[] = L"ventura, california";
  const WString str01(str_lit01);
  WString str02(L"del mar, california");
  WString str03(L" and ");
  WString str05;

  // wstring& replace(size_type pos, size_type n, const wstring& string)
  // wstring& replace(size_type pos1, size_type n1, const wstring& string,
  //                 size_type pos2, size_type n2)
  // wstring& replace(size_type pos, size_type n1, const wchar_t* s, size_type n2)
  // wstring& replace(size_type pos, size_type n1, const wchar_t* s)
  // wstring& replace(size_type pos, size_type n1, size_type n2, wchar_t c)
  // wstring& replace(iterator it1, iterator it2, const wstring& str)
  // wstring& replace(iterator it1, iterator it2, const wchar_t* s, size_type n)
  // wstring& replace(iterator it1, iterator it2, const wchar_t* s)
  // wstring& replace(iterator it1, iterator it2, size_type n, char c)
  // template<typename InputIter>
  //   wstring& replace(iterator it1, iterator it2, InputIter j1, InputIter j2)

  // with mods, from tstring.cc, from jason merrill, et. al.
  WString X = L"Hello";
  WString x = X;

  wchar_t ch = x[0];
  VERIFY( ch == L'H' );

  WString z = x.substr(2, 3);
  VERIFY( z == L"llo" );

  x.replace(2, 2, L"r");
  VERIFY( x == L"Hero" );

  x = X;
  x.replace(0, 1, L"j");
  VERIFY( x == L"jello" );

  wchar_t ar[] = { L'H', L'e', L'l', L'l', L'o' };
  x.replace(std::find(x.begin(), x.end(), L'l'), 
        std::find(x.rbegin(), x.rend(), L'l').base(), ar, 
        ar + sizeof(ar) / sizeof(ar[0]));
  VERIFY( x == L"jeHelloo" );
}

TEST(WStringTest, Substr01)
{
  typedef WString::size_type csize_type;
  typedef WString::const_reference cref;
  typedef WString::reference ref;
  csize_type csz01;

  const wchar_t str_lit01[] = L"rockaway, pacifica";
  const WString str01(str_lit01);
  WString str02;

  // basic_string<charT, _Traits, _Alloc>
  //  substr(size_type pos = 0, size_type n = npos) const;
  csz01 = str01.size();
  str02 = str01.substr(0, 1);
  VERIFY( str02 == L"r" );
  str02 = str01.substr(10);
  VERIFY( str02 == L"pacifica" );

#ifndef NO_CPP_EXCEPTION_HANDLERS
  try {
    str02 = str01.substr(csz01 + 1);
    VERIFY( false ); 
  }
  catch(std::out_of_range& ) {
    VERIFY( true );
  }
  catch(...) {
    VERIFY( false );
  }

 try {
    str02 = str01.substr(csz01);
    VERIFY( str02.size() == 0 );
  }
  catch(std::out_of_range& ) {
    VERIFY( false );
  }
  catch(...) {
    VERIFY( false );
  }
#endif
}



//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (WStringTest, WStringSubstr)
{
    WString w1 = TESTDATA_StringW;
    WString w2 = w1.substr (7, 4);

    ASSERT_EQ (0, wcscmp (L"Test", w2.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (WStringTest, WStringInit)
{
    WString w = TESTDATA_StringW;
    
    ASSERT_EQ (w.length (), wcslen (w.c_str ()));
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Shaun.Sewall                    08/11
//---------------------------------------------------------------------------------------
TEST (WStringTest, WStringOperators)
{
    WString a (L"abc");
    WString z (L"zyx");
    
    ASSERT_TRUE (a != z);
    ASSERT_TRUE (a <  z);
    ASSERT_TRUE (a <= z);
    ASSERT_TRUE (z >  a);
    ASSERT_TRUE (z >= a);
    SUCCEED ();
}

//---------------------------------------------------------------------------------------
// @betest                                      Krischan.Eberle                    06/13
//---------------------------------------------------------------------------------------
TEST (WStringTest, WStringLength)
    {
    //ensure that length () and size () count the characters not the bytes.
    WString a (L"abc");
    ASSERT_EQ (a.length (), a.size ());
    ASSERT_EQ (3, a.length ());
    ASSERT_EQ (3, wcslen (a.c_str ()));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, EndsWith_EmptyStrings_False)
    {
    EXPECT_FALSE(WString(L"").EndsWith(L""));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, EndsWith_StringWithEmptyEnding_False)
    {
    EXPECT_FALSE(WString(L"ABC").EndsWith(L""));
    EXPECT_FALSE(WString(L"ABC").EndsWith(nullptr));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, EndsWith_StringWithEnding_True)
    {
    EXPECT_TRUE(WString(L"ABC").EndsWith(L"C"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, EndsWith_StringWithBegining_False)
    {
    EXPECT_FALSE(WString(L"ABC").EndsWith(L"A"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, EndsWith_EqualStrings_True)
    {
    EXPECT_TRUE(WString(L"ABC").EndsWith(L"ABC"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, EndsWith_DifferentCaseStrings)
    {
    EXPECT_FALSE(WString(L"ABC").EndsWith(L"abc"));

    EXPECT_TRUE(WString(L"ABC").EndsWithI(L"abc"));
    
    EXPECT_FALSE(WString(L"ABC").EndsWithI(nullptr));

    EXPECT_FALSE(WString(L"ABC").EndsWithI(L"abcd"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                 02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, EndsWith_EndingLongerThanString_False)
    {
    EXPECT_FALSE(WString(L"AAA").EndsWith(L"AAAA"));
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          01/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, StartsWith)
    {
    EXPECT_FALSE(WString(L"").StartsWith(L""));
    EXPECT_FALSE(WString(L"ABC").StartsWith(L""));
    EXPECT_TRUE(WString(L"ABC").StartsWith(L"A"));
    EXPECT_FALSE(WString(L"ABC").StartsWith(L"C"));
    EXPECT_TRUE(WString(L"ABC").StartsWith(L"ABC"));
    EXPECT_FALSE(WString(L"AAA").StartsWith(L"AAAA"));

    // Check Case
    EXPECT_FALSE(WString(L"ABC").StartsWith(L"abc"));
    EXPECT_TRUE(WString(L"ABC").StartsWithI(L"abc"));
    EXPECT_FALSE(WString(L"ABC").StartsWithI(L""));
    EXPECT_FALSE(WString(L"ABC").StartsWithI(nullptr));
    EXPECT_FALSE(WString(L"ABC").StartsWithI(L"abcd"));
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          01/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, Contains)
    {
    // Check WCharCP
    EXPECT_TRUE(WString(L"").Contains(L""));
    EXPECT_FALSE(WString(L"").Contains(L"ABCDEF1232"));
    EXPECT_TRUE(WString(L"ABC").Contains(L""));
    EXPECT_TRUE(WString(L"ABC").Contains(L"A"));
    EXPECT_FALSE(WString(L"ABC").Contains(L"ABD"));
    EXPECT_TRUE(WString(L"ABC").Contains(L"ABC"));
    EXPECT_FALSE(WString(L"AAA").Contains(L"AAAA"));
    
    // Check WString
    EXPECT_TRUE(WString(L"").Contains(WString(L"")));
    EXPECT_FALSE(WString(L"").Contains(WString(L"ABCDEF1232")));
    EXPECT_TRUE(WString(L"ABC").Contains(WString(L"")));
    EXPECT_TRUE(WString(L"ABC").Contains(WString(L"A")));
    EXPECT_FALSE(WString(L"ABC").Contains(WString(L"ABD")));
    EXPECT_TRUE(WString(L"ABC").Contains(WString(L"ABC")));
    EXPECT_FALSE(WString(L"AAA").Contains(WString(L"AAAA")));
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          01/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, FindI)
    {
    EXPECT_EQ(0, WString(L"abc").FindI(L"abc"));
    EXPECT_EQ(0, WString(L"ABC").FindI(L"abc"));
    EXPECT_EQ(0, WString(L"AbC").FindI(L"ABC"));

    EXPECT_EQ(0, WString(L"").FindI(L""));
#ifdef BENTLEY_WIN32
    EXPECT_EQ(std::string::npos, WString(L"").FindI(L"abc"));
#endif 
    EXPECT_EQ(0, WString(L"ababa").FindI(L"ab"));
    EXPECT_EQ(1, WString(L"ababa").FindI(L"ba"));
    EXPECT_EQ(2, WString(L"ababc").FindI(L"abc"));

    EXPECT_EQ(0, WString(L"121").FindI(L"1"));
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          01/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, ContainsI)
    {
    // Check WCharCP
    EXPECT_FALSE(WString(L"ABC").Contains(L"abc"));
    EXPECT_TRUE(WString(L"ABC").ContainsI(L"abc"));
    EXPECT_TRUE(WString(L"ABC").ContainsI(L"ABC"));
        
    // Check WString
    EXPECT_FALSE(WString(L"ABC").Contains(WString(L"abc")));
    EXPECT_TRUE(WString(L"ABC").ContainsI(WString(L"abc")));
    EXPECT_TRUE(WString(L"ABC").ContainsI(WString(L"ABC")));
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, ReplaceAll)
    {
    WString str(L"");
    EXPECT_EQ(0, str.ReplaceAll(L"", L""));
    EXPECT_STREQ(L"",str.c_str());

    str.assign(L"");
    EXPECT_EQ(0, str.ReplaceAll(L"", L"A"));
    EXPECT_STREQ(L"", str.c_str());

    str.assign(L"ABC");
    EXPECT_EQ(1, str.ReplaceAll(L"A", L""));
    EXPECT_STREQ(L"BC", str.c_str());

    str.assign(L"ABC");
    EXPECT_EQ(1, str.ReplaceAll(L"A", L"C"));
    EXPECT_STREQ(L"CBC", str.c_str());

    str.assign(L"ABCABC ");
    EXPECT_EQ(2, str.ReplaceAll(L"AB", L""));
    EXPECT_STREQ(L"CC ", str.c_str());

    str.assign(L"ABCABC");
    EXPECT_EQ(2, str.ReplaceAll(L"AB", L"11223344"));
    EXPECT_STREQ(L"11223344C11223344C", str.c_str());

    str.assign(L"AAA");
    EXPECT_EQ(0, str.ReplaceAll(L"AAAA", L""));
    EXPECT_STREQ(L"AAA", str.c_str());

    // Check Case
    str.assign(L"ABC");
    EXPECT_EQ(0, str.ReplaceAll(L"abc", L"def"));
    EXPECT_STREQ(L"ABC", str.c_str());
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, ReplaceI)
    {
    WString str(L"");
    EXPECT_TRUE( str.ReplaceI(L"", L""));
    EXPECT_STREQ(L"",str.c_str());

    str.assign(L"");
    EXPECT_TRUE(str.ReplaceI(L"", L"A"));
    EXPECT_STREQ(L"A", str.c_str());

    str.assign(L"ABC");
    EXPECT_TRUE(str.ReplaceI(L"A", L""));
    EXPECT_STREQ(L"BC", str.c_str());

    str.assign(L"ABC");
    EXPECT_TRUE(str.ReplaceI(L"A", L"C"));
    EXPECT_STREQ(L"CBC", str.c_str());

    str.assign(L"ABC");
    EXPECT_TRUE(str.ReplaceI(L"a", L"C"));
    EXPECT_STREQ(L"CBC", str.c_str());

    str.assign(L"ABCABC ");
    EXPECT_TRUE(str.ReplaceI(L"Ab", L""));
    EXPECT_STREQ(L"CABC ", str.c_str());

    str.assign(L"ABCABC");
    EXPECT_TRUE(str.ReplaceI(L"AB", L"11223344"));
    EXPECT_STREQ(L"11223344CABC", str.c_str());

    str.assign(L"ABC");
    EXPECT_FALSE(str.ReplaceI(L"ABCD", L""));
    EXPECT_STREQ(L"ABC", str.c_str());

    str.assign(L"ABC");
    EXPECT_FALSE(str.ReplaceI(L"ABCD", L""));
    EXPECT_STREQ(L"ABC", str.c_str());

    str.assign(L"ABC");
    EXPECT_FALSE(str.ReplaceI(L"ABCD", NULL));
    EXPECT_STREQ(L"ABC", str.c_str());
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, PadRight)
    {
    WString str(L"");
    EXPECT_STREQ(L"", str.PadRight(0, '0').c_str());

    str.assign(L"");
    EXPECT_STREQ(L"00000", str.PadRight(5, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"ABC00", str.PadRight(5, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"ABC", str.PadRight(3, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"ABC", str.PadRight(1, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"ABCD", str.PadRight(4, 'D').c_str());
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, PadLeft)
    {
    WString str(L"");
    EXPECT_STREQ(L"", str.PadLeft(0, '0').c_str());

    str.assign(L"");
    EXPECT_STREQ(L"00000", str.PadLeft(5, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"00ABC", str.PadLeft(5, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"ABC", str.PadLeft(3, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"ABC", str.PadLeft(1, '0').c_str());

    str.assign(L"ABC");
    EXPECT_STREQ(L"DABC", str.PadLeft(4, 'D').c_str());
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          02/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, Assign)
    {
    WString str4(L"SomeValue");
    str4.AssignA("SomeOtherValue");
    VERIFY(str4 == L"SomeOtherValue");
    str4.AssignA(nullptr);
    VERIFY(str4 == L"");

    WString str5(L"Test String");
    WString newValue(L"New Test String");
    Utf16Buffer newValueUtf16;
    BeStringUtilities::WCharToUtf16(newValueUtf16, newValue.c_str(), BeStringUtilities::AsManyAsPossible);
    str5.AssignUtf16(newValueUtf16.data());
    EXPECT_STREQ(str5.c_str(), L"New Test String");

    str5.AssignUtf16(nullptr);
    VERIFY(str5 == L"");
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          03/16
//---------------------------------------------------------------------------------------
TEST(AStringTest, Util)
    {
    AString str (L" abc ");
    VERIFY (str.c_str() == str.c_str() );
    AString cc (str);
    VERIFY( str == cc );
    VERIFY( str.length() == 5 );
    }

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          04/16
//---------------------------------------------------------------------------------------
static void VerifyVSPrintf (WCharCP expectedStr, WCharCP fmt, ...)
    {
    va_list args;
    va_start(args, fmt);
    WString str;
    EXPECT_TRUE( SUCCESS == str.VSprintf(fmt, args));
    va_end(args);
    EXPECT_TRUE(str.Equals(expectedStr))<<"Expected: "<<expectedStr<<"\nActual: "<<str.c_str();
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          04/16
//---------------------------------------------------------------------------------------
TEST(WStringTest, VSprintf)
    {
    VerifyVSPrintf(L"Test", L"Test");
    VerifyVSPrintf(L"Test no. 5", L"%ls no. %d", L"Test", 5);
    }
