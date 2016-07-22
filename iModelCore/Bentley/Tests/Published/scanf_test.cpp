/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/scanf_test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <Bentley/BeTest.h>
#include    <Bentley/BeStringUtilities.h>
#include    <Bentley/WString.h>

static void ok (bool b, CharCP fmt, ...)
    {
    if (b)
        return;

    char buf[512];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    FAIL() << buf;
    va_end(args);
    }

/*
 * Unit test suite for *BE_STRING_UTILITIES_UTF8_SSCANF functions.
 *
 * Copyright 2002 Uwe Bonnes
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>

static void test_sscanf( void )
{
    char buffer[100], buffer1[100];
    int result, ret;
    char c;
    void *ptr;
    static const char pname[]=" St. Petersburg, Florida\n";
    int hour=21,min=59,sec=20;
    int  number,number_so_far;

    /* check EOF */
    strcpy(buffer,"");
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%d", &result);
    ok( ret == EOF || ret == 0,"BE_STRING_UTILITIES_UTF8_SSCANF returns %x instead of %x\n", ret, EOF );

    /* check %p */
    ok( BE_STRING_UTILITIES_UTF8_SSCANF("000000000046F170", "%p", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == (void *)0x46F170,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %x\n", ptr, 0x46F170 );

    ok( BE_STRING_UTILITIES_UTF8_SSCANF("0046F171", "%p", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == (void *)0x46F171,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %x\n", ptr, 0x46F171 );

    ok( BE_STRING_UTILITIES_UTF8_SSCANF("46F172", "%p", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == (void *)0x46F172,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %x\n", ptr, 0x46F172 );

#if defined (WIP_FORMAT_P_BUGS)
    // *** BUG: BE_STRING_UTILITIES_UTF8_SSCANF should not allow the 0x prefix?!
    ok( BE_STRING_UTILITIES_UTF8_SSCANF("0x46F173", "%p", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == NULL,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %x\n", ptr, 0 );

    // *** BUG: BE_STRING_UTILITIES_UTF8_SSCANF reads 000000000046F172 instead of FFFFFFFFFFB90E8C
    ok( BE_STRING_UTILITIES_UTF8_SSCANF("-46F174", "%p", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == (void *)(ULONG_PTR)-0x46f174,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %p\n",
        ptr, (void *)(ULONG_PTR)-0x46f174 );

    // *** BUG: BE_STRING_UTILITIES_UTF8_SSCANF reads 000000000046F172 instead of 46f175
    ok( BE_STRING_UTILITIES_UTF8_SSCANF("+46F175", "%p", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == (void *)0x46F175,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %x\n", ptr, 0x46F175 );

    /* check %p with no hex digits */
    ok( BE_STRING_UTILITIES_UTF8_SSCANF("1233", "%p", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == (void *)0x1233,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %x\n", ptr, 0x1233 );

    // *** BUG: BE_STRING_UTILITIES_UTF8_SSCANF reads 0000000000001233 instead of 1234
    ok( BE_STRING_UTILITIES_UTF8_SSCANF("1234", "%P", &ptr) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( ptr == (void *)0x1234,"BE_STRING_UTILITIES_UTF8_SSCANF reads %p instead of %x\n", ptr, 0x1234 );
#endif

    /* check %x */
    strcpy(buffer,"0x519");
    ok( BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%x", &result) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n"  );
    ok( result == 0x519,"BE_STRING_UTILITIES_UTF8_SSCANF reads %x instead of %x\n", result, 0x519 );

    strcpy(buffer,"0x51a");
    ok( BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%x", &result) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n" );
    ok( result == 0x51a ,"BE_STRING_UTILITIES_UTF8_SSCANF reads %x instead of %x\n", result, 0x51a );

    strcpy(buffer,"0x51g");
    ok( BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%x", &result) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n" );
    ok( result == 0x51, "BE_STRING_UTILITIES_UTF8_SSCANF reads %x instead of %x\n", result, 0x51 );

#if defined (WIP_FORMAT_X_BUGS)
    // *** BUG: We reject -1 because we are looking for a (positive) hex value only
    result = 0;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF("-1", "%x", &result);
    ok(ret == 1, "Wrong number of arguments read: %d (expected 1)\n", ret);
    ok(result == -1, "Read %d, expected -1\n", result);
#endif

#if defined (WIP_FORMAT_X_BUGS)
    // *** BUG: We fail to treat %<anything> as matching <anything>. The problem is that we don't read from the input stream when we hit an unrecognized char following %
    /* check % followed by any char */
    {
    char format[20];
    strcpy(buffer,"\"%12@");
    strcpy(format,"%\"%%%d%@");  /* work around gcc format check */
    ok( BE_STRING_UTILITIES_UTF8_SSCANF(buffer, format, &result) == 1, "BE_STRING_UTILITIES_UTF8_SSCANF failed\n" );
    ok( result == 12, "BE_STRING_UTILITIES_UTF8_SSCANF reads %x instead of %x\n", result, 12 );
    }
#endif

    /* Check float */
    {
    float res1= -82.6267f, res2= 27.76f, res11, res12;
    ret = sprintf(buffer,"%f %f",res1, res2);
    ok( ret == 20, "expected 20, got %u\n", ret);
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer,"%f%f",&res11, &res12);
    ok( ret == 2, "expected 2, got %u\n", ret);
    ok( (res11 == res1) && (res12 == res2), "Error reading floats\n");
    }

    /* check strings */
    ret = sprintf(buffer," %s", pname);
    ok( ret == 26, "expected 26, got %u\n", ret);
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer,"%*c%[^\n]",buffer1);
    ok( ret == 1, "Error with format \"%s\"\n","%*c%[^\n]");
    ok( strncmp(pname,buffer1,strlen(buffer1)) == 0, "Error with \"%s\" \"%s\"\n",pname, buffer1);

/* WIP_LINUX -- these tests fail when using native sscanf from GCC on Linux!?!
    ret = BE_STRING_UTILITIES_UTF8_SSCANF("abcefgdh","%*[a-cg-e]%c",&buffer[0]);
    ok( ret == 1, "Error with format \"%s\"\n","%*[a-cg-e]%c");
    ok( buffer[0] == 'd', "Error with \"abcefgdh\" \"%c\"\n", buffer[0]);

    ret = BE_STRING_UTILITIES_UTF8_SSCANF("abcefgdh","%*[a-cd-dg-e]%c",&buffer[0]);
    ok( ret == 1, "Error with format \"%s\"\n","%*[a-cd-dg-e]%c");
    ok( buffer[0] == 'h', "Error with \"abcefgdh\" \"%c\"\n", buffer[0]);
*/

    /* check digits */
    ret = sprintf(buffer,"%d:%d:%d",hour,min,sec);
    ok( ret == 8, "expected 8, got %u\n", ret);
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer,"%d%n",&number,&number_so_far);
    ok(ret == 1 , "problem with format arg \"%%d%%n\"\n");
    ok(number == hour,"Read wrong arg %d instead of %d\n",number, hour);
    ok(number_so_far == 2,"Read wrong arg for \"%%n\" %d instead of 2\n",number_so_far);

    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer+2,"%*c%n",&number_so_far);
    ok(ret == 0 , "problem with format arg \"%%*c%%n\"\n");
    ok(number_so_far == 1,"Read wrong arg for \"%%n\" %d instead of 2\n",number_so_far);

    /* Check %i according to bug 1878 */
    strcpy(buffer,"123");
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%i", &result);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(result == 123, "Wrong number read\n");
    result = 0;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF("-1", "%i", &result);
    ok(ret == 1, "Wrong number of arguments read: %d (expected 1)\n", ret);
    ok(result == -1, "Read %d, expected -1\n", result);
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%d", &result);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(result == 123, "Wrong number read\n");
    result = 0;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF("-1", "%d", &result);
    ok(ret == 1, "Wrong number of arguments read: %d (expected 1)\n", ret);
    ok(result == -1, "Read %d, expected -1\n", result);

    /* Check %i for octal and hexadecimal input */
    result = 0;
    strcpy(buffer,"017");
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%i", &result);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(result == 15, "Wrong number read\n");
    result = 0;
    strcpy(buffer,"0x17");
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%i", &result);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(result == 23, "Wrong number read\n");

#if defined (WIP_FORMAT_O_BUGS)
    // *** BUG: We don't recognize a - number as octal
    /* %o */
    result = 0;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF("-1", "%o", &result);
    ok(ret == 1, "Wrong number of arguments read: %d (expected 1)\n", ret);
    ok(result == -1, "Read %d, expected -1\n", result);
#endif

#if defined (WIP_FORMAT_U_BUGS)
    // *** BUG: We don't recognize a - number as unsigned
    /* %u */
    result = 0;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF("-1", "%u", &result);
    ok(ret == 1, "Wrong number of arguments read: %d (expected 1)\n", ret);
    ok(result == -1, "Read %d, expected -1\n", result);
#endif

    /* Check %c */
    strcpy(buffer,"a");
    c = 0x55;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%c", &c);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(c == 'a', "Field incorrect: '%c'\n", c);

    strcpy(buffer," a");
    c = 0x55;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%c", &c);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(c == ' ', "Field incorrect: '%c'\n", c);

#if !defined (ANDROID)
    strcpy(buffer,"18:59");
    c = 0x55;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%d:%d%c", &hour, &min, &c);
    ok(ret == 2, "Wrong number of arguments read: %d\n", ret);
    ok(hour == 18, "Field 1 incorrect: %d\n", hour);
    ok(min == 59, "Field 2 incorrect: %d\n", min);
    ok(c == 0x55, "Field 3 incorrect: 0x%02x\n", c);
#endif

    /* Check %n (also whitespace in format strings and %s) */
    buffer[0]=0; buffer1[0]=0;
    ret = BE_STRING_UTILITIES_UTF8_SSCANF("abc   def", "%s %n%s", buffer, &number_so_far, buffer1);
    ok(strcmp(buffer, "abc")==0, "First %%s read incorrectly: %s\n", buffer);
    ok(strcmp(buffer1,"def")==0, "Second %%s read incorrectly: %s\n", buffer1);
    ok(number_so_far==6, "%%n yielded wrong result: %d\n", number_so_far);
    ok(ret == 2, "%%n shouldn't count as a conversion: %d\n", ret);

    /* Check where %n matches to EOF in buffer */
    strcpy(buffer, "3:45");
    ret = BE_STRING_UTILITIES_UTF8_SSCANF(buffer, "%d:%d%n", &hour, &min, &number_so_far);
    ok(ret == 2, "Wrong number of arguments read: %d\n", ret);
    ok(number_so_far == 4, "%%n yielded wrong result: %d\n", number_so_far);
}

#if defined (sscanf_s_NOT_SUPPORTED)

static void test_sscanf_s(void)
{
    int i, ret;
    char buf[100];
/* 
    int (__cdecl *psscanf_s)(const char*,const char*,...);
    HMODULE hmod = GetModuleHandleA("msvcrt.dll");

    psscanf_s = (void*)GetProcAddress(hmod, "sscanf_s");
    if(!psscanf_s) {
        win_skip("sscanf_s not available\n");
        return;
    }
*/
#define psscanf_s BE_STRING_UTILITIES_UTF8_SSCANF

    ret = psscanf_s("123", "%d", &i);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(i == 123, "i = %d\n", i);

    ret = psscanf_s("123", "%s", buf, 100);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(!strcmp("123", buf), "buf = %s\n", buf);

    ret = psscanf_s("123", "%s", buf, 3);
    ok(ret == 0, "Wrong number of arguments read: %d\n", ret);
    ok(buf[0]=='\0', "buf = %s\n", buf);

    buf[0] = 'a';
    ret = psscanf_s("123", "%3c", buf, 2);
    ok(ret == 0, "Wrong number of arguments read: %d\n", ret);
    ok(buf[0]=='\0', "buf = %s\n", buf);

    i = 1;
    ret = psscanf_s("123 123", "%s %d", buf, 2, &i);
    ok(ret == 0, "Wrong number of arguments read: %d\n", ret);
    ok(i==1, "i = %d\n", i);

    i = 1;
    ret = psscanf_s("123 123", "%d %s", &i, buf, 2);
    ok(ret == 1, "Wrong number of arguments read: %d\n", ret);
    ok(i==123, "i = %d\n", i);
}

#endif

static void test_swscanf( void )
{
    wchar_t buffer[100];
    int result, ret;
    static const wchar_t formatd[] = {'%','d',0};

    /* check WEOF */
    /* WEOF is an unsigned short -1 but BE_STRING_UTILITIES_SWSCANF returns int
       so it should be sign-extended */
    buffer[0] = 0;
    ret = BE_STRING_UTILITIES_SWSCANF(buffer, formatd, &result);
    ok( ret == EOF || ret == 0, "BE_STRING_UTILITIES_SWSCANF returns %x instead of %x\n", ret, EOF );
}

TEST(scanf_test,Tests)
{
    test_sscanf();
#if defined (sscanf_s_NOT_SUPPORTED)
    test_sscanf_s();
#endif
    test_swscanf();
}

TEST(scanf_test,WStringConversions)
    {
    WCharCP winput = L"ABC";
     CharCP ainput =  "ABC";
    wchar_t woutput[80];
     char   aoutput[80];

    ASSERT_TRUE( 1 == BE_STRING_UTILITIES_SWSCANF (winput, L"%ls", woutput) );
    ASSERT_STREQ( woutput, winput );

    //ASSERT_TRUE( 1 == BE_STRING_UTILITIES_SWSCANF (winput, L"%hs", aoutput) );
    //ASSERT_TRUE( 0 == strcmp(aoutput,ainput) );

    //ASSERT_TRUE( 1 == BE_STRING_UTILITIES_UTF8_SSCANF (ainput, "%ls", woutput) );
    //ASSERT_STREQ( woutput, winput );

    ASSERT_TRUE( 1 == BE_STRING_UTILITIES_UTF8_SSCANF (ainput, "%s", aoutput) );
    ASSERT_TRUE( 0 == strcmp(aoutput,ainput) );
    }

TEST(scanf_test,WStringMixes)
    {
    int v;
    ASSERT_TRUE( 1 == BE_STRING_UTILITIES_SWSCANF (L"SC99", L"SC%d", &v) );
    ASSERT_EQ(v,99);
    }
