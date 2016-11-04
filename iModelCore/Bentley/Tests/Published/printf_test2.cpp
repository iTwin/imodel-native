/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/printf_test2.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <Bentley/BeTest.h>
#include    <Bentley/BeStringUtilities.h>
#include    <Bentley/WString.h>
#if defined (ANDROID)
    #include <android/log.h>
#elif defined (__unix__)
    #include <stdio.h>
    #include <wchar.h>
#endif

static void ok (bool b, WCharCP fmt, ...)
    {
    if (b)
        return;

    wchar_t buf[512];
    va_list args;
    va_start (args, fmt);
#if defined (_WIN32)
    vswprintf (buf, fmt, args);
    va_end(args);
    FAIL() << buf;
#elif defined (__unix__)
    vswprintf (buf, _countof(buf), fmt, args);
    va_end(args);
    FAIL() << buf;
#else
    WString msg;
    msg.VSprintf (fmt, args); // *** NEEDS WORK: This uses the very library that we are trying to test, but I have no choice. There's no other way to format a message including wide chars on Android.
    Utf8String msgUtf8;
    BeStringUtilities::WCharToUtf8 (msgUtf8, msg.c_str());
    va_end(args);
    __android_log_print (ANDROID_LOG_ERROR, "print_test2", msgUtf8.c_str ());
    FAIL();
#endif
    }

/*
 * Conformance tests for *printf functions.
 *
 * Copyright 2002 Uwe Bonnes
 * Copyright 2004 Aneurin Price
 * Copyright 2005 Mike McCormack
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

/* With Visual Studio >= 2005,  BeStringUtilities::Snwprintf() takes an extra parameter unless
 * the following macro is defined.
 */
#define _CRT_NON_CONFORMING_SWPRINTFS
 
#include <stdio.h>
#include <errno.h>

static void test_sprintf( void )
{
    wchar_t buffer[100];
    const wchar_t *format;
    double pnumber=789456123;
    int r;
    wchar_t wide[] = { 'w','i','d','e',0};

    format = L"%+#23.15e";
    r = BeStringUtilities::Snwprintf(buffer,format,pnumber);
    ok(!wcscmp(buffer,L"+7.894561230000000e+008")||!wcscmp(buffer,L" +7.894561230000000e+08"),L"+#23.15e failed: '%ls'\n", buffer);
    ok( r==23, L"return count wrong\n");

    format = L"%-#23.15e";
    r = BeStringUtilities::Snwprintf(buffer,format,pnumber);
    ok(!wcscmp(buffer,L"7.894561230000000e+008 ")||!wcscmp(buffer,L"7.894561230000000e+08  "),L"-#23.15e failed: '%ls'\n", buffer);
    ok( r==23, L"return count wrong\n");

    format = L"%#23.15e";
    r = BeStringUtilities::Snwprintf(buffer,format,pnumber);
    ok(!wcscmp(buffer,L" 7.894561230000000e+008")||!wcscmp(buffer,L"  7.894561230000000e+08"),L"#23.15e failed: '%ls'\n", buffer);
    ok( r==23, L"return count wrong\n");

    format = L"%#1.1g";
    r = BeStringUtilities::Snwprintf(buffer,format,pnumber);
    ok(!wcscmp(buffer,L"8.e+008")||!wcscmp(buffer,L"8.e+08"),L"#1.1g failed: '%ls'\n", buffer);
    ok( r==7 || r==6, L"return count wrong\n");

    format = L"%lld";
    r = BeStringUtilities::Snwprintf(buffer,format,((uint64_t)0xffffffff)*0xffffffff);
    ok(!wcscmp(buffer,L"-8589934591"),L"Problem with long long\n");
    ok( r==11, L"return count wrong\n");

    format = L"%+8lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"    +100") && r==8,L"+8lld failed: '%ls'\n", buffer);

    format = L"%+.8lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"+00000100") && r==9,L"+.8lld failed: '%ls'\n", buffer);

    format = L"%+10.8lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L" +00000100") && r==10,L"+10.8lld failed: '%ls'\n", buffer);

//    format = L"%_1lld";
//    r = BeStringUtilities::Snwprintf(buffer,format,(Int64)100);
//    ok(!wcscmp(buffer,L"_1lld") /*&& r==6*/,L"_1lld failed\n");

    format = L"%-1.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"-00100") && r==6,L"-1.5lld failed: '%ls'\n", buffer);

    format = L"%5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"  100") && r==5,L"5lld failed: '%ls'\n", buffer);

    format = L"%5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L" -100") && r==5,L"5lld failed: '%ls'\n", buffer);

    format = L"%-5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"100  ") && r==5,L"-5lld failed: '%ls'\n", buffer);

    format = L"%-5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"-100 ") && r==5,L"-5lld failed: '%ls'\n", buffer);

    format = L"%-.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"00100") && r==5,L"-.5lld failed: '%ls'\n", buffer);

    format = L"%-.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"-00100") && r==6,L"-.5lld failed: '%ls'\n", buffer);

    format = L"%-8.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"00100   ") && r==8,L"-8.5lld failed: '%ls'\n", buffer);

    format = L"%-8.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"-00100  ") && r==8,L"-8.5lld failed: '%ls'\n", buffer);

    format = L"%05lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"00100") && r==5,L"05lld failed: '%ls'\n", buffer);

    format = L"%05lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"-0100") && r==5,L"05lld failed: '%ls'\n", buffer);

    format = L"% lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L" 100") && r==4,L"' lld' failed: '%ls'\n", buffer);

    format = L"% lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"-100") && r==4,L"' lld' failed: '%ls'\n", buffer);

    format = L"% 5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"  100") && r==5,L"' 5lld' failed: '%ls'\n", buffer);

    format = L"% 5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L" -100") && r==5,L"' 5lld' failed: '%ls'\n", buffer);

    format = L"% .5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L" 00100") && r==6,L"' .5lld' failed: '%ls'\n", buffer);

    format = L"% .5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"-00100") && r==6,L"' .5lld' failed: '%ls'\n", buffer);

    format = L"% 8.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"   00100") && r==8,L"' 8.5lld' failed: '%ls'\n", buffer);

    format = L"% 8.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"  -00100") && r==8,L"' 8.5lld' failed: '%ls'\n", buffer);

    format = L"%.0lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)0);
    ok(r==0,L".0lld failed: '%ls'\n", buffer);

    format = L"%#+21.18llx";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L" 0x00ffffffffffffff9c") && r==21,L"#+21.18llx failed: '%ls'\n", buffer);

    format = L"%#.25llo";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"0001777777777777777777634") && r==25,L"#.25llo failed: '%ls'\n", buffer);

    format = L"%#+24.20llo";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L" 01777777777777777777634") && r==24,L"#+24.20llo failed: '%ls'\n", buffer);

    format = L"%#+18.21llX";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"0X00000FFFFFFFFFFFFFF9C") && r==23,L"#+18.21llX failed: '%ls '\n", buffer);

    format = L"%#+20.24llo";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-100);
    ok(!wcscmp(buffer,L"001777777777777777777634") && r==24,L"#+20.24llo failed: '%ls'\n", buffer);

    format = L"%#+25.22llu";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-1);
    ok(!wcscmp(buffer,L"   0018446744073709551615") && r==25,L"#+25.22llu conversion failed: '%ls'\n", buffer);

    format = L"%#+25.22llu";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-1);
    ok(!wcscmp(buffer,L"   0018446744073709551615") && r==25,L"#+25.22llu failed: '%ls'\n", buffer);

    format = L"%#+30.25llu";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-1);
    ok(!wcscmp(buffer,L"     0000018446744073709551615") && r==30,L"#+30.25llu failed: '%ls'\n", buffer);

    format = L"%+#25.22lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-1);
    ok(!wcscmp(buffer,L"  -0000000000000000000001") && r==25,L"+#25.22lld failed: '%ls'\n", buffer);

    format = L"%#-8.5llo";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"00144   ") && r==8,L"-8.5llo failed: '%ls'\n", buffer);

    format = L"%#-+ 08.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"+00100  ") && r==8,L"'#-+ 08.5lld failed: '%ls'\n", buffer);

    format = L"%#-+ 08.5lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)100);
    ok(!wcscmp(buffer,L"+00100  ") && r==8,L"#-+ 08.5lld failed: '%ls'\n", buffer);

    format = L"%.80lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)1);
    ok(r==80,L"%ls format failed\n", format);

    format = L"% .80lld";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)1);
    ok(r==81,L"%ls format failed\n", format);

    format = L"% .80d";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(r==81,L"%ls format failed\n", format);

    format = L"%lld";
    r = BeStringUtilities::Snwprintf(buffer,format,((uint64_t)0xffffffff)*0xffffffff);
    ok( r == 1 || r == 11, L"return count wrong %d\n", r);
    if (r == 11)  /* %ll works on Vista */
        ok(!wcscmp(buffer, L"-8589934591"), L"Problem with \"ll\" interpretation '%ls'\n", buffer);
    else
        ok(!wcscmp(buffer, L"1"), L"Problem with \"ll\" interpretation '%ls'\n", buffer);

#ifdef WIP_SPRINTF_LINUX // ***these tests fail in a Linux GCC build!?
    format = L"%I";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer, L"I"), L"Problem with \"I\" interpretation\n");
    ok( r==1, L"return count wrong\n");

    format = L"%I0d";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer,L"I0d"),L"I0d failed\n");
    ok( r==3, L"return count wrong\n");

    format = L"%I32d";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    if (r == 1)
    {
        ok(!wcscmp(buffer,L"1"),L"I32d failed, got '%ls'\n",buffer);
    }
    else
    {
        /* Older versions don't grok I32 format */
        ok(r == 4 && !wcscmp(buffer,L"I32d"),L"I32d failed, got '%ls',%d\n",buffer,r);
    }
#endif

#if defined (FORMATTING_BUG)
// we return llD, r==3
    format = L"%llD";
    r = BeStringUtilities::Snwprintf(buffer,format,(int64_t)-1);
    ok(!wcscmp(buffer,L"D"),L"lld failed: %ls\n",buffer);
    ok( r==1, L"return count wrong\n");
#endif

    format = L"% d";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer, L" 1"),L"Problem with sign place-holder: '%ls'\n",buffer);
    ok( r==2, L"return count wrong\n");

    format = L"%+ d";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer, L"+1"),L"Problem with sign flags: '%ls'\n",buffer);
    ok( r==2, L"return count wrong\n");

    format = L"%ls";
    r = BeStringUtilities::Snwprintf(buffer,format,wide);
    ok(!wcscmp(buffer,L"wide"),L"Problem with wide string format\n");
    ok( r==4, L"return count wrong\n");

    format = L"%#012x";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer,L"0x0000000001"),L"Hexadecimal zero-padded \"%ls\"\n",buffer);

    format = L"%#04.8x";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer,L"0x00000001"), L"Hexadecimal zero-padded precision \"%ls\"\n",buffer);

    format = L"%#-08.2x";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer,L"0x01    "), L"Hexadecimal zero-padded not left-adjusted \"%ls\"\n",buffer);

    format = L"%#08o";
    r = BeStringUtilities::Snwprintf(buffer,format,1);
    ok(!wcscmp(buffer,L"00000001"), L"Octal zero-padded \"%ls\"\n",buffer);

#if defined (FORMATTING_BUG)
// we don't handle %p correctly
    if (sizeof(void *) == 8)
    {
        format = L"%p";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"0000000000000039"),L"Pointer formatted incorrectly \"%ls\"\n",buffer);
        ok( r==16, L"return count wrong\n");

        format = L"%#020p";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"  0X0000000000000039"),L"Pointer formatted incorrectly\n");
        ok( r==20, L"return count wrong\n");

        format = L"%Fp";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"0000000000000039"),L"Pointer formatted incorrectly \"%ls\"\n",buffer);
        ok( r==16, L"return count wrong\n");

        format = L"%#-020p";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"0X0000000000000039  "),L"Pointer formatted incorrectly\n");
        ok( r==20, L"return count wrong\n");
    }
    else
    {
        format = L"%p";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"00000039"),L"Pointer formatted incorrectly \"%ls\"\n",buffer);
        ok( r==8, L"return count wrong\n");

        format = L"%#012p";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"  0X00000039"),L"Pointer formatted incorrectly\n");
        ok( r==12, L"return count wrong\n");

        format = L"%Fp";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"00000039"),L"Pointer formatted incorrectly \"%ls\"\n",buffer);
        ok( r==8, L"return count wrong\n");

        format = L"%#-012p";
        r = BeStringUtilities::Snwprintf(buffer,format,(void *)57);
        ok(!wcscmp(buffer,L"0X00000039  "),L"Pointer formatted incorrectly\n");
        ok( r==12, L"return count wrong\n");
    }
#endif // defined (FORMATTING_BUG)

    format = L"hello";
    r = BeStringUtilities::Snwprintf(buffer, format);
    ok(!wcscmp(buffer,L"hello"), L"failed\n");
    ok( r==5, L"return count wrong\n");

    format = L"%ls";
    r = BeStringUtilities::Snwprintf(buffer, format, wide);
    ok(!wcscmp(buffer,L"wide"), L"failed\n");
    ok( r==4, L"return count wrong\n");

    format = L"%-10ls";
    r = BeStringUtilities::Snwprintf(buffer, format, wide );
    ok(!wcscmp(buffer,L"wide      "), L"failed\n");
    ok( r==10, L"return count wrong\n");

    format = L"%10ls";
    r = BeStringUtilities::Snwprintf(buffer, format, wide );
    ok(!wcscmp(buffer,L"      wide"), L"failed\n");
    ok( r==10, L"return count wrong\n");

//    format = L"%l0s";
//    r = BeStringUtilities::Snwprintf(buffer, format, wide );
//    ok(!wcscmp(buffer,L"0s"), L"failed\n");
//    ok( r==2, L"return count wrong\n");

//    format = L"%l-s";
//    r = BeStringUtilities::Snwprintf(buffer, format, wide );
//    ok(!wcscmp(buffer,L"-s"), L"failed\n");
//    ok( r==2, L"return count wrong\n");

    format = L"%ls";
    r = BeStringUtilities::Snwprintf(buffer, format, wide );
    ok(!wcscmp(buffer,L"wide"), L"failed\n");
    ok( r==4, L"return count wrong\n");

//    format = L"%Ls";
//    r = BeStringUtilities::Snwprintf(buffer, format, L"not wide" );
//    ok(!wcscmp(buffer,L"not wide"), L"failed\n");
//    ok( r==8, L"return count wrong\n");

//    format = L"%b";
//    r = BeStringUtilities::Snwprintf(buffer, format);
//    ok(!wcscmp(buffer,L"b"), L"failed\n");
//    ok( r==1, L"return count wrong\n");

    format = L"%3d";
    r = BeStringUtilities::Snwprintf(buffer, format,1234);
    ok(!wcscmp(buffer,L"1234"), L"failed\n");
    ok( r==4, L"return count wrong\n");

 //   format = L"%3h";
 //   r = BeStringUtilities::Snwprintf(buffer, format);
 //   ok(!wcscmp(buffer,L""), L"failed\n");
 //   ok( r==0, L"return count wrong\n");

 //   format = L"%j%k%m%q%r%t%v%y%z";
 //   r = BeStringUtilities::Snwprintf(buffer, format);
 //   ok(!wcscmp(buffer,L"jkmqrtvyz"), L"failed\n");
 //   ok( r==9, L"return count wrong\n");

//    format = L"asdf%n";
//    x = 0;
//    r = BeStringUtilities::Snwprintf(buffer, format, &x );
//    if (r == -1)
//    {
//        /* %n format is disabled by default on vista */
//        /* FIXME: should test with _set_printf_count_output */
//        ok(x == 0, L"should not write to x: %d\n", x);
//    }
//    else
//    {
//        ok(x == 4, L"should write to x: %d\n", x);
//        ok(!wcscmp(buffer,L"asdf"), L"failed\n");
//        ok( r==4, L"return count wrong: %d\n", r);
//    }

    format = L"%-1d";
    r = BeStringUtilities::Snwprintf(buffer, format,2);
    ok(!wcscmp(buffer,L"2"), L"failed\n");
    ok( r==1, L"return count wrong\n");

    format = L"%2.4f";
    r = BeStringUtilities::Snwprintf(buffer, format,8.6);
    ok(!wcscmp(buffer,L"8.6000"), L"failed\n");
    ok( r==6, L"return count wrong\n");

    format = L"%0f";
    r = BeStringUtilities::Snwprintf(buffer, format,0.6);
    ok(!wcscmp(buffer,L"0.600000"), L"failed\n");
    ok( r==8, L"return count wrong\n");

    format = L"%.0f";
    r = BeStringUtilities::Snwprintf(buffer, format,0.6);
    ok(!wcscmp(buffer,L"1"), L"failed\n");
    ok( r==1, L"return count wrong\n");

    format = L"%2.4e";
    r = BeStringUtilities::Snwprintf(buffer, format,8.6);
    ok(!wcscmp(buffer,L"8.6000e+000")||!wcscmp(buffer,L"8.6000e+00"), L"failed\n");
    ok( r==11||r==10, L"return count wrong\n");

    format = L"%2.4g";
    r = BeStringUtilities::Snwprintf(buffer, format,8.6);
    ok(!wcscmp(buffer,L"8.6"), L"failed\n");
    ok( r==3, L"return count wrong\n");

    format = L"%-i";
    r = BeStringUtilities::Snwprintf(buffer, format,-1);
    ok(!wcscmp(buffer,L"-1"), L"failed\n");
    ok( r==2, L"return count wrong\n");

    format = L"%-i";
    r = BeStringUtilities::Snwprintf(buffer, format,1);
    ok(!wcscmp(buffer,L"1"), L"failed\n");
    ok( r==1, L"return count wrong\n");

    format = L"%+i";
    r = BeStringUtilities::Snwprintf(buffer, format,1);
    ok(!wcscmp(buffer,L"+1"), L"failed\n");
    ok( r==2, L"return count wrong\n");

    format = L"%o";
    r = BeStringUtilities::Snwprintf(buffer, format,10);
    ok(!wcscmp(buffer,L"12"), L"failed\n");
    ok( r==2, L"return count wrong\n");

#if defined (FORMATTING_BUG)
// we don't handle %p correctly
    format = L"%p";
    r = BeStringUtilities::Snwprintf(buffer, format,0);
    if (sizeof(void *) == 8)
    {
        ok(!wcscmp(buffer,L"0000000000000000"), L"failed\n");
        ok( r==16, L"return count wrong\n");
    }
    else
    {
        ok(!wcscmp(buffer,L"00000000"), L"failed\n");
        ok( r==8, L"return count wrong\n");
    }
#endif

    format = L"%ls";
    r = BeStringUtilities::Snwprintf(buffer, format,0);
    ok(!wcscmp(buffer,L"(null)"), L"failed\n");
    ok( r==6, L"return count wrong\n");

    format = L"%ls";
    r = BeStringUtilities::Snwprintf(buffer, format,L"%%%%");
    ok(!wcscmp(buffer,L"%%%%"), L"failed\n");
    ok( r==4, L"return count wrong\n");

    format = L"%u";
    r = BeStringUtilities::Snwprintf(buffer, format,-1);
    ok(!wcscmp(buffer,L"4294967295"), L"failed\n");
    ok( r==10, L"return count wrong\n");

//    format = L"%h";
//    r = BeStringUtilities::Snwprintf(buffer, format,-1);
//    ok(!wcscmp(buffer,L""), L"failed\n");
//    ok( r==0, L"return count wrong\n");

//    format = L"%z";
//    r = BeStringUtilities::Snwprintf(buffer, format,-1);
//    ok(!wcscmp(buffer,L"z"), L"failed\n");
//    ok( r==1, L"return count wrong\n");

//    format = L"%j";
//    r = BeStringUtilities::Snwprintf(buffer, format,-1);
//    ok(!wcscmp(buffer,L"j"), L"failed\n");
//   ok( r==1, L"return count wrong\n");

#if defined (FORMATTING_BUG)
    format = L"%F";
    r = BeStringUtilities::Snwprintf(buffer, format,-1);
    ok(!wcscmp(buffer,L""), L"failed\n");
    ok( r==0, L"return count wrong\n");
#endif

//    format = L"%H";
//    r = BeStringUtilities::Snwprintf(buffer, format,-1);
//    ok(!wcscmp(buffer,L"H"), L"failed\n");
//    ok( r==1, L"return count wrong\n");

    format = L"%%0";
    r = BeStringUtilities::Snwprintf(buffer, format);
    ok(!wcscmp(buffer,L"%0"), L"failed: \"%ls\"\n", buffer);
    ok( r==2, L"return count wrong\n");
}

static void test_swprintf( void )
{
    wchar_t buffer[100];
    const wchar_t lld[] = {'%','l','l','d',0};
    double pnumber=789456123;
    const wchar_t TwentyThreePoint15e[]= {'%','+','#','2','3','.','1','5','e',0};
    const wchar_t e008[] = {'e','+','0','0','8',0};
    const wchar_t e08[] = {'e','+','0','8',0};

    BeStringUtilities::Snwprintf(buffer,TwentyThreePoint15e,pnumber);
    ok(!wcsstr(buffer,e008)||!wcsstr(buffer,e08),L"Sprintf different\n");
    BeStringUtilities::Snwprintf(buffer,lld,((uint64_t)0xffffffff)*0xffffffff);
      ok(wcslen(buffer) == 11,L"Problem with long long\n");
}

static void test_snprintf (void)
{
    struct snprintf_test {
        const wchar_t *format;
        int expected;
    };
    /* Pre-2.1 libc behaviour, not C99 compliant. */
    const struct snprintf_test tests[] = {{L"short", 5},
                                          {L"justfit", 7},
                                          {L"justfits", -1},
                                          {L"muchlonger", -1}};
    wchar_t buffer[8];
    const int bufsiz = _countof(buffer);
    unsigned int i;

    for (i = 0; i < _countof(tests); i++) {
        const wchar_t *fmt  = tests[i].format;
        const int expect    = tests[i].expected;
        const int n         = BeStringUtilities::Snwprintf (buffer, bufsiz, fmt);
        const int valid     = n < 0 ? 0 : (n == bufsiz ? n : n+1);

        ok (n == expect, L"\"%ls\": expected %d, returned %d\n",
            fmt, expect, n);
        ok (!memcmp (fmt, buffer, valid),
            L"\"%ls\": rendered \"%.*s\"\n", fmt, valid, buffer);
    };
}


#if defined (DOESNT_ADD_ANYTHING)
static void test_fprintf(void)
{
    static char file_name[] = "fprintf.tst";
    FILE *fp = fopen(file_name, L"wb");
    char buf[1024];
    int ret;

    ret = fprintf(fp, L"simple test\n");
    ok(ret == 12, L"ret = %d\n", ret);
    ret = ftell(fp);
    ok(ret == 12, L"ftell returned %d\n", ret);

    ret = fprintf(fp, L"contains%cnull\n", '\0');
    ok(ret == 14, L"ret = %d\n", ret);
    ret = ftell(fp);
    ok(ret == 26, L"ftell returned %d\n", ret);

    fclose(fp);

    fp = fopen(file_name, L"rb");
    ret = fscanf(fp, L"%[^\n] ", buf);
    ok(ret == 1, L"ret = %d\n", ret);
    ret = ftell(fp);
    ok(ret == 12, L"ftell returned %d\n", ret);
    ok(!wcscmp(buf, L"simple test"), L"buf = %ls\n", buf);

    fgets(buf, sizeof(buf), fp);
    ret = ftell(fp);
    ok(ret == 26, L"ret = %d\n", ret);
    ok(!memcmp(buf, L"contains\0null\n", 14), L"buf = %ls\n", buf);

    fclose(fp);
    unlink(file_name);
}
#endif 

#if defined (NOT_NOW)
static void test_fcvt(void)
{
    char *str;
    int dec=100, sign=100;
    
    /* Numbers less than 1.0 with different precisions */
    str = _fcvt(0.0001, 1, &dec, &sign );
    ok( 0 == wcscmp(str,L""), L"bad return '%ls'\n", str);
    ok( -3 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    str = _fcvt(0.0001, -10, &dec, &sign );
    ok( 0 == wcscmp(str,L""), L"bad return '%ls'\n", str);
    ok( -3 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    str = _fcvt(0.0001, 10, &dec, &sign );
    ok( 0 == wcscmp(str,L"1000000"), L"bad return '%ls'\n", str);
    ok( -3 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    /* Basic sign test */
    str = _fcvt(-111.0001, 5, &dec, &sign );
    ok( 0 == wcscmp(str,L"11100010"), L"bad return '%ls'\n", str);
    ok( 3 == dec, L"dec wrong %d\n", dec);
    ok( 1 == sign, L"sign wrong\n");

    str = _fcvt(111.0001, 5, &dec, &sign );
    ok( 0 == wcscmp(str,L"11100010"), L"bad return '%ls'\n", str);
    ok( 3 == dec, L"dec wrong\n");
    ok( 0 == sign, L"sign wrong\n");

    /* 0.0 with different precisions */
    str = _fcvt(0.0, 5, &dec, &sign );
    ok( 0 == wcscmp(str,L"00000"), L"bad return '%ls'\n", str);
    ok( 0 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    str = _fcvt(0.0, 0, &dec, &sign );
    ok( 0 == wcscmp(str,L""), L"bad return '%ls'\n", str);
    ok( 0 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    str = _fcvt(0.0, -1, &dec, &sign );
    ok( 0 == wcscmp(str,L""), L"bad return '%ls'\n", str);
    ok( 0 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    /* Numbers > 1.0 with 0 or -ve precision */
    str = _fcvt(-123.0001, 0, &dec, &sign );
    ok( 0 == wcscmp(str,L"123"), L"bad return '%ls'\n", str);
    ok( 3 == dec, L"dec wrong %d\n", dec);
    ok( 1 == sign, L"sign wrong\n");

    str = _fcvt(-123.0001, -1, &dec, &sign );
    ok( 0 == wcscmp(str,L"12"), L"bad return '%ls'\n", str);
    ok( 3 == dec, L"dec wrong %d\n", dec);
    ok( 1 == sign, L"sign wrong\n");

    str = _fcvt(-123.0001, -2, &dec, &sign );
    ok( 0 == wcscmp(str,L"1"), L"bad return '%ls'\n", str);
    ok( 3 == dec, L"dec wrong %d\n", dec);
    ok( 1 == sign, L"sign wrong\n");

    str = _fcvt(-123.0001, -3, &dec, &sign );
    ok( 0 == wcscmp(str,L""), L"bad return '%ls'\n", str);
    ok( 3 == dec, L"dec wrong %d\n", dec);
    ok( 1 == sign, L"sign wrong\n");

    /* Numbers > 1.0, but with rounding at the point of precision */
    str = _fcvt(99.99, 1, &dec, &sign );
    ok( 0 == wcscmp(str,L"1000"), L"bad return '%ls'\n", str);
    ok( 3 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    /* Numbers < 1.0 where rounding occurs at the point of precision */
    str = _fcvt(0.00636, 2, &dec, &sign );
    ok( 0 == wcscmp(str,L"1"), L"bad return '%ls'\n", str);
    ok( -1 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    str = _fcvt(0.00636, 3, &dec, &sign );
    ok( 0 == wcscmp(str,L"6"), L"bad return '%ls'\n", str);
    ok( -2 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    str = _fcvt(0.09999999996, 2, &dec, &sign );
    ok( 0 == wcscmp(str,L"10"), L"bad return '%ls'\n", str);
    ok( 0 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");

    str = _fcvt(0.6, 0, &dec, &sign );
    ok( 0 == wcscmp(str,L"1"), L"bad return '%ls'\n", str);
    ok( 1 == dec, L"dec wrong %d\n", dec);
    ok( 0 == sign, L"sign wrong\n");
}

/* Don't test nrdigits < 0, msvcrt on Win9x and NT4 will corrupt memory by
 * writing outside allocated memory */
static struct {
    double value;
    int nrdigits;
    const char *expstr_e;
    const char *expstr_f;
    int expdecpt_e;
    int expdecpt_f;
    int expsign;
} test_cvt_testcases[] = {
    {          45.0,   2,        "45",           "4500",          2,      2,      0 },
    /* Numbers less than 1.0 with different precisions */
    {        0.0001,   1,         "1",               "",         -3,     -3,     0 },
    {        0.0001,  10,L"1000000000",        "1000000",         -3,     -3,     0 },
    /* Basic sign test */
    {     -111.0001,   5,     "11100",       "11100010",          3,      3,     1 },
    {      111.0001,   5,     "11100",       "11100010",          3,      3,     0 },
    /* big numbers with low precision */
    {        3333.3,   2,        "33",         "333330",          4,      4,     0 },
    {999999999999.9,   3,       "100",L"999999999999900",         13,     12,     0 },
    /* 0.0 with different precisions */
    {           0.0,   5,     "00000",          "00000",          0,      0,     0 },
    {           0.0,   0,          "",               "",          0,      0,     0 },
    {           0.0,  -1,          "",               "",          0,      0,     0 },
    /* Numbers > 1.0 with 0 or -ve precision */
    {     -123.0001,   0,          "",            "123",          3,      3,     1 },
    {     -123.0001,  -1,          "",             "12",          3,      3,     1 },
    {     -123.0001,  -2,          "",              "1",          3,      3,     1 },
    {     -123.0001,  -3,          "",               "",          3,      3,     1 },
    /* Numbers > 1.0, but with rounding at the point of precision */
    {         99.99,   1,         "1",           "1000",          3,      3,     0 },
    /* Numbers < 1.0 where rounding occurs at the point of precision */
    {        0.0063,   2,        "63",              "1",         -2,     -1,     0 },
    {        0.0063,   3,        "630",             "6",         -2,     -2,     0 },
    { 0.09999999996,   2,        "10",             "10",          0,      0,     0 },
    {           0.6,   1,         "6",              "6",          0,      0,     0 },
    {           0.6,   0,          "",              "1",          1,      1,     0 },
    {           0.4,   0,          "",               "",          0,      0,     0 },
    {          0.49,   0,          "",               "",          0,      0,     0 },
    {          0.51,   0,          "",              "1",          1,      1,     0 },
    /* ask for ridiculous precision, ruin formatting this table */
    {           1.0,  30, L"100000000000000000000000000000",
                      "1000000000000000000000000000000",          1,      1,      0},
    {           123456789012345678901.0,  30, L"123456789012345680000000000000",
                      "123456789012345680000000000000000000000000000000000",         21,    21,      0},
    /* end marker */
    { 0, 0, L"END"}
};

static void test_xcvt(void)
{
    char *str;
    int i, decpt, sign, err;

    for( i = 0; wcscmp( test_cvt_testcases[i].expstr_e, L"END"); i++){
        decpt = sign = 100;
        str = _ecvt( test_cvt_testcases[i].value,
                test_cvt_testcases[i].nrdigits,
                &decpt,
                &sign);
        ok( 0 == strncmp( str, test_cvt_testcases[i].expstr_e, 15),
               "_ecvt() bad return, got \n'%ls' expected \n'%ls'\n", str,
              test_cvt_testcases[i].expstr_e);
        ok( decpt == test_cvt_testcases[i].expdecpt_e,
                "_ecvt() decimal point wrong, got %d expected %d\n", decpt,
                test_cvt_testcases[i].expdecpt_e);
        ok( sign == test_cvt_testcases[i].expsign,
                "_ecvt() sign wrong, got %d expected %d\n", sign,
                test_cvt_testcases[i].expsign);
    }
    for( i = 0; wcscmp( test_cvt_testcases[i].expstr_e, L"END"); i++){
        decpt = sign = 100;
        str = _fcvt( test_cvt_testcases[i].value,
                test_cvt_testcases[i].nrdigits,
                &decpt,
                &sign);
        ok( 0 == strncmp( str, test_cvt_testcases[i].expstr_f, 15),
               "_fcvt() bad return, got \n'%ls' expected \n'%ls'\n", str,
              test_cvt_testcases[i].expstr_f);
        ok( decpt == test_cvt_testcases[i].expdecpt_f,
                "_fcvt() decimal point wrong, got %d expected %d\n", decpt,
                test_cvt_testcases[i].expdecpt_f);
        ok( sign == test_cvt_testcases[i].expsign,
                "_fcvt() sign wrong, got %d expected %d\n", sign,
                test_cvt_testcases[i].expsign);
    }

    if (p__ecvt_s)
    {
        str = malloc(1024);
        for( i = 0; wcscmp( test_cvt_testcases[i].expstr_e, L"END"); i++){
            decpt = sign = 100;
            err = p__ecvt_s(str, 1024, test_cvt_testcases[i].value, test_cvt_testcases[i].nrdigits, &decpt, &sign);
            ok(err == 0, L"_ecvt_s() failed with error code %d\n", err);
            ok( 0 == strncmp( str, test_cvt_testcases[i].expstr_e, 15),
                   "_ecvt_s() bad return, got \n'%ls' expected \n'%ls'\n", str,
                  test_cvt_testcases[i].expstr_e);
            ok( decpt == test_cvt_testcases[i].expdecpt_e,
                    "_ecvt_s() decimal point wrong, got %d expected %d\n", decpt,
                    test_cvt_testcases[i].expdecpt_e);
            ok( sign == test_cvt_testcases[i].expsign,
                    "_ecvt_s() sign wrong, got %d expected %d\n", sign,
                    test_cvt_testcases[i].expsign);
        }
        free(str);
    }
    else
        win_skip("_ecvt_s not available\n");

    if (p__fcvt_s)
    {
        int i;

        str = malloc(1024);

        /* invalid arguments */
        err = p__fcvt_s(NULL, 0, 0.0, 0, &i, &i);
        ok(err == EINVAL, L"got %d, expected EINVAL\n", err);

        err = p__fcvt_s(str, 0, 0.0, 0, &i, &i);
        ok(err == EINVAL, L"got %d, expected EINVAL\n", err);

        str[0] = ' ';
        str[1] = 0;
        err = p__fcvt_s(str, -1, 0.0, 0, &i, &i);
        ok(err == 0, L"got %d, expected 0\n", err);
        ok(str[0] == 0, L"got %c, expected 0\n", str[0]);
        ok(str[1] == 0, L"got %c, expected 0\n", str[1]);

        err = p__fcvt_s(str, 1, 0.0, 0, NULL, &i);
        ok(err == EINVAL, L"got %d, expected EINVAL\n", err);

        err = p__fcvt_s(str, 1, 0.0, 0, &i, NULL);
        ok(err == EINVAL, L"got %d, expected EINVAL\n", err);

        for( i = 0; wcscmp( test_cvt_testcases[i].expstr_e, L"END"); i++){
            decpt = sign = 100;
            err = p__fcvt_s(str, 1024, test_cvt_testcases[i].value, test_cvt_testcases[i].nrdigits, &decpt, &sign);
            ok(err == 0, L"_fcvt_s() failed with error code %d\n", err);
            ok( 0 == strncmp( str, test_cvt_testcases[i].expstr_f, 15),
                   "_fcvt_s() bad return, got '%ls' expected '%ls'. test %d\n", str,
                  test_cvt_testcases[i].expstr_f, i);
            ok( decpt == test_cvt_testcases[i].expdecpt_f,
                    "_fcvt_s() decimal point wrong, got %d expected %d\n", decpt,
                    test_cvt_testcases[i].expdecpt_f);
            ok( sign == test_cvt_testcases[i].expsign,
                    "_fcvt_s() sign wrong, got %d expected %d\n", sign,
                    test_cvt_testcases[i].expsign);
        }
        free(str);
    }
    else
        win_skip("_fcvt_s not available\n");
}
#endif // defined (NOT_NOW)

static void test_vsnwprintf(void)
{
    const wchar_t format[] = {'%','l','s','%','l','s','%','l','s',0};
    const wchar_t one[]    = {'o','n','e',0};
    const wchar_t two[]    = {'t','w','o',0};
    const wchar_t three[]  = {'t','h','r','e','e',0};

    int ret;
    wchar_t str[32];

    ret = BeStringUtilities::Snwprintf ( str, _countof(str), format, one, two, three );

    ok( ret == 11, L"got %d expected 11\n", ret );
    ok( !wcscmp(str, L"onetwothree"), L"got %ls expected 'onetwothree'\n", str );
}

 static void test_vswprintf(void)
 {
     const wchar_t format[] = {'%','l','s',' ','%','d',0};
     const wchar_t number[] = {'n','u','m','b','e','r',0};
     const wchar_t out[] = {'n','u','m','b','e','r',' ','1','2','3',0};
     wchar_t buf[20];
 
     int ret;
 
     ret = BeStringUtilities::Snwprintf(buf, format, number, 123);
     ok(ret == 10, L"got %d, expected 10\n", ret);
     ok(!memcmp(buf, out, sizeof(out)), L"buf = %ls\n", (buf));
 
     memset(buf, 0, sizeof(buf));
     ret = BeStringUtilities::Snwprintf(buf, format, number, 123);
     ok(ret == 10, L"got %d, expected 10\n", ret);
     ok(!memcmp(buf, out, sizeof(out)), L"buf = %ls\n", (buf));
 
     memset(buf, 0, sizeof(buf));
     ret = BeStringUtilities::Snwprintf(buf, format, number, 123);
     ok(ret == 10, L"got %d, expected 10\n", ret);
     ok(!memcmp(buf, out, sizeof(out)), L"buf = %ls\n", (buf));
 
     memset(buf, 0, sizeof(buf));
     ret = BeStringUtilities::Snwprintf(buf, 20, format, number, 123);
     ok(ret == 10, L"got %d, expected 10\n", ret);
     ok(!memcmp(buf, out, sizeof(out)), L"buf = %ls\n", (buf));
 
     memset(buf, 0, sizeof(buf));
     ret = BeStringUtilities::Snwprintf(buf, 20, format, number, 123);
     ok(ret == 10, L"got %d, expected 10\n", ret);
     ok(!memcmp(buf, out, sizeof(out)), L"buf = %ls\n", (buf));
 
     memset(buf, 0, sizeof(buf));
     ret = BeStringUtilities::Snwprintf(buf, 20, format, number, 123);
     ok(ret == 10, L"got %d, expected 10\n", ret);
     ok(!memcmp(buf, out, sizeof(out)), L"buf = %ls\n", (buf));
 }
 
 TEST(printf_test,MoreTests)
 {
     test_sprintf();
     test_swprintf();
     test_snprintf();
     test_vsnwprintf();
     test_vswprintf();
 }
