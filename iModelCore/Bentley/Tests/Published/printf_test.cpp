/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/printf_test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <Bentley/BeTest.h>
#include    <Bentley/BeStringUtilities.h>
#include    <Bentley/WString.h>

#if defined (_MSC_VER)
#pragma warning (disable:4723) // divide by zero
#endif

/* Test of POSIX compatible vsnprintf() and snprintf() functions.
   Copyright (C) 2007 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* Written by Bruno Haible <bruno@clisp.org>, 2007.  */

/* The Compaq (ex-DEC) C 6.4 compiler chokes on the expression 0.0 / returnZero().  */
#if defined(__DECC) || defined(_WIN32)
static double
NaN ()
{
  static double zero = 0.0;
  return zero / zero;
}
#else
# define NaN() (0.0 / returnZero())
#endif

#if !defined (ANDROID) && !defined(_X86_)
/* Windows x86 compiler /fp:fast will clip a -0.0 arg to 0.0 arg  */
static bool _canTakeNegZeroArg (double minus_zero) 
    {
    double plus_zero = 0.0;
    return memcmp (&plus_zero, &minus_zero, sizeof (double)) != 0;
    }

/* The SGI MIPS floating-point format does not distinguish 0.0 and -0.0.  */
static int
have_minus_zero ()
{
  if (!_canTakeNegZeroArg (-0.0))
    return false;
  static double plus_zero = 0.0;
  static double minus_zero = -0.0;
  return memcmp (&plus_zero, &minus_zero, sizeof (double)) != 0;
}
#endif

/* Representation of an 80-bit 'long double' as an initializer for a sequence
   of 'unsigned int' words.  */
#ifdef WORDS_BIGENDIAN
# define LDBL80_WORDS(exponent,manthi,mantlo) \
    { ((unsigned int) (exponent) << 16) | ((unsigned int) (manthi) >> 16), \
      ((unsigned int) (manthi) << 16) | (unsigned int) (mantlo) >> 16),    \
      (unsigned int) (mantlo) << 16                                        \
    }
#else
# define LDBL80_WORDS(exponent,manthi,mantlo) \
    { mantlo, manthi, exponent }
#endif

static bool
strmatch (const char *pattern, const char *string)
{
  if (strlen (pattern) != strlen (string))
    return false;
  for (; *pattern != '\0'; pattern++, string++)
    if (*pattern != '*' && *string != *pattern)
      return false;
  return true;
}

/* Test whether string[start_index..end_index-1] is a valid textual
   representation of NaN.  */
static int
strisnan (const char *string, size_t start_index, size_t end_index, int uppercase)
{
#if defined (_WIN32)
#if _MSC_VER >= 1900
	return strstr(string, "nan(ind)") != NULL;
#else
	return strstr (string, "#IND") != NULL;
#endif
#else
  return strstr (string, "NAN") != NULL || strstr (string, "nan") != NULL || strstr (string, "NaN") != NULL;
#endif
}

static bool strisinf (const char* string, char const* endswith, int uppercase)
{
size_t n = strlen (string);
size_t xn = strlen (endswith);
if (strcmp (string+(n-xn), endswith) != 0)
 return false;
#if defined (_WIN32)
#if _MSC_VER >= 1900
  return strstr(string, "inf") != NULL;
#else
  return strstr (string, "#INF") != NULL;
#endif
#else
  return strstr (string, "inf") != NULL || strstr (string, "INF") != NULL || strstr (string, "Inf") != NULL;
#endif
}
	  
#define my_snprintf BeStringUtilities::Snprintf

static double returnZero() {return 0.0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(printf_test, Strings)
{
// --- wstrings ---
if (true)
{
wchar_t buf[256];
BeStringUtilities::Snwprintf (buf, L"%d", 123);
EXPECT_TRUE( wcscmp (buf, L"123") == 0 );

BeStringUtilities::Snwprintf (buf, L"%ls", L"abc");
EXPECT_TRUE( wcscmp (buf, L"abc") == 0 );

// Now try some combinations
BeStringUtilities::Snwprintf (buf, L"%d %ls %d", 1, L"abc", 2);
EXPECT_TRUE( wcscmp (buf, L"1 abc 2") == 0 );

BeStringUtilities::Snwprintf (buf, L"%d %d %ls", 1, 2, L"abc");
EXPECT_TRUE( wcscmp (buf, L"1 2 abc") == 0 );
}

// --- astrings ---
if (true)
{
char buf[256];
BeStringUtilities::Snprintf (buf, "%d", 123);
EXPECT_TRUE( strcmp (buf, "123") == 0 );

BeStringUtilities::Snprintf (buf, "%hs", "abc");
EXPECT_TRUE( strcmp (buf, "abc") == 0 );
}
}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(printf_test, Numbers)
{
  char buf[8];
  int size;

  /* Test return value convention.  */

  for (size = 0; size <= 8; size++)
    {
      int retval;

      memcpy (buf, "DEADBEEF", 8);
      retval = my_snprintf (buf, size, "%d", 12345);
      EXPECT_TRUE ((size<=5)? retval == -1: retval == 5);        // modified to follow Microsoft return value rule for truncated strings
      if (size < 6)
	{
	  if (size > 0)
	    {
	      EXPECT_TRUE (memcmp (buf, "12345", size - 1) == 0);
	      EXPECT_TRUE (buf[size - 1] == '\0');
	    }
	  EXPECT_TRUE (memcmp (buf + size, "DEADBEEF" + size, 8 - size) == 0);
	}
      else
	{
	  EXPECT_TRUE (strcmp (buf, "12345") == 0);                 // modified to follow Microsoft secure buffer filling rule
//	  EXPECT_TRUE (memcmp (buf, "12345\0EF", 8) == 0);
	}
    }

  /* Test support of size specifiers as in C99.  */
#if defined (WIP_NOT_IMPLEMENTED)
  {
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%ju %d", (uintmax_t) 12345671, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12345671 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  {
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%zu %d", (size_t) 12345672, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12345672 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  {
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%tu %d", (ptrdiff_t) 12345673, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12345673 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  {
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", (long double) 1.5, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.5 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_NOT_IMPLEMENTED)
  /* Test the support of the 'a' and 'A' conversion specifier for hexadecimal
     output of floating-point numbers.  */

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%a %d", 3.1416015625, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.922p+1 33") == 0
	    || strcmp (result, "0x3.244p+0 33") == 0
	    || strcmp (result, "0x6.488p-1 33") == 0
	    || strcmp (result, "0xc.91p-2 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%A %d", -3.1416015625, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0X1.922P+1 33") == 0
	    || strcmp (result, "-0X3.244P+0 33") == 0
	    || strcmp (result, "-0X6.488P-1 33") == 0
	    || strcmp (result, "-0XC.91P-2 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%a %d", 0.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x0p+0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%a %d", -0.0, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0x0p+0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%a %d", 1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "inf 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%a %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%a %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding near the decimal point.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.0a %d", 1.5, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x2p+0 33") == 0
	    || strcmp (result, "0x3p-1 33") == 0
	    || strcmp (result, "0x6p-2 33") == 0
	    || strcmp (result, "0xcp-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 0.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.0a %d", 1.51, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x2p+0 33") == 0
	    || strcmp (result, "0x3p-1 33") == 0
	    || strcmp (result, "0x6p-2 33") == 0
	    || strcmp (result, "0xcp-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 1.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.1a %d", 1.51, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.8p+0 33") == 0
	    || strcmp (result, "0x3.0p-1 33") == 0
	    || strcmp (result, "0x6.1p-2 33") == 0
	    || strcmp (result, "0xc.1p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 2.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.2a %d", 1.51, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.83p+0 33") == 0
	    || strcmp (result, "0x3.05p-1 33") == 0
	    || strcmp (result, "0x6.0ap-2 33") == 0
	    || strcmp (result, "0xc.14p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 3.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.3a %d", 1.51, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.829p+0 33") == 0
	    || strcmp (result, "0x3.052p-1 33") == 0
	    || strcmp (result, "0x6.0a4p-2 33") == 0
	    || strcmp (result, "0xc.148p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding can turn a ...FFF into a ...000.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.3a %d", 1.49999, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.800p+0 33") == 0
	    || strcmp (result, "0x3.000p-1 33") == 0
	    || strcmp (result, "0x6.000p-2 33") == 0
	    || strcmp (result, "0xc.000p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding can turn a ...FFF into a ...000.
       This shows a MacOS X 10.3.9 (Darwin 7.9) bug.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.1a %d", 1.999, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.0p+1 33") == 0
	    || strcmp (result, "0x2.0p+0 33") == 0
	    || strcmp (result, "0x4.0p-1 33") == 0
	    || strcmp (result, "0x8.0p-2 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%10a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "  0x1.cp+0 33") == 0
	    || strcmp (result, "  0x3.8p-1 33") == 0
	    || strcmp (result, "    0x7p-2 33") == 0
	    || strcmp (result, "    0xep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.10a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.c000000000p+0 33") == 0
	    || strcmp (result, "0x3.8000000000p-1 33") == 0
	    || strcmp (result, "0x7.0000000000p-2 33") == 0
	    || strcmp (result, "0xe.0000000000p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Large precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.50a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.c0000000000000000000000000000000000000000000000000p+0 33") == 0
	    || strcmp (result, "0x3.80000000000000000000000000000000000000000000000000p-1 33") == 0
	    || strcmp (result, "0x7.00000000000000000000000000000000000000000000000000p-2 33") == 0
	    || strcmp (result, "0xe.00000000000000000000000000000000000000000000000000p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-10a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.cp+0   33") == 0
	    || strcmp (result, "0x3.8p-1   33") == 0
	    || strcmp (result, "0x7p-2     33") == 0
	    || strcmp (result, "0xep-3     33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "+0x1.cp+0 33") == 0
	    || strcmp (result, "+0x3.8p-1 33") == 0
	    || strcmp (result, "+0x7p-2 33") == 0
	    || strcmp (result, "+0xep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, " 0x1.cp+0 33") == 0
	    || strcmp (result, " 0x3.8p-1 33") == 0
	    || strcmp (result, " 0x7p-2 33") == 0
	    || strcmp (result, " 0xep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.cp+0 33") == 0
	    || strcmp (result, "0x3.8p-1 33") == 0
	    || strcmp (result, "0x7.p-2 33") == 0
	    || strcmp (result, "0xe.p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#a %d", 1.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.p+0 33") == 0
	    || strcmp (result, "0x2.p-1 33") == 0
	    || strcmp (result, "0x4.p-2 33") == 0
	    || strcmp (result, "0x8.p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%010a %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x001.cp+0 33") == 0
	    || strcmp (result, "0x003.8p-1 33") == 0
	    || strcmp (result, "0x00007p-2 33") == 0
	    || strcmp (result, "0x0000ep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%010a %d", 1.0 / returnZero(), 33, 44, 55);
    /* "0000000inf 33" is not a valid result; see
       <http://lists.gnu.org/archive/html/bug-gnulib/2007-04/msg00107.html> */
    EXPECT_TRUE (strcmp (result, "       inf 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050a %d", NaN (), 33, 44, 55);
    /* "0000000nan 33" is not a valid result; see
       <http://lists.gnu.org/archive/html/bug-gnulib/2007-04/msg00107.html> */
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", 3.1416015625L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.922p+1 33") == 0
	    || strcmp (result, "0x3.244p+0 33") == 0
	    || strcmp (result, "0x6.488p-1 33") == 0
	    || strcmp (result, "0xc.91p-2 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LA %d", -3.1416015625L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0X1.922P+1 33") == 0
	    || strcmp (result, "-0X3.244P+0 33") == 0
	    || strcmp (result, "-0X6.488P-1 33") == 0
	    || strcmp (result, "-0XC.91P-2 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", 0.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x0p+0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", -0.0L, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0x0p+0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", 1.0L / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "inf 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", -1.0L / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", 0.0L / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#if CHECK_PRINTF_SAFE && ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_))
  { /* Quiet NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0xC3333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  {
    /* Signalling NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  /* The isnanl function should recognize Pseudo-NaNs, Pseudo-Infinities,
     Pseudo-Zeroes, Unnormalized Numbers, and Pseudo-Denormals, as defined in
       Intel IA-64 Architecture Software Developer's Manual, Volume 1:
       Application Architecture.
       Table 5-2 "Floating-Point Register Encodings"
       Figure 5-6 "Memory to Floating-Point Register Data Translation"
   */
  { /* Pseudo-NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Infinity.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Zero.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Unnormalized number.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Denormal.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%La %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Rounding near the decimal point.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.0La %d", 1.5L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x2p+0 33") == 0
	    || strcmp (result, "0x3p-1 33") == 0
	    || strcmp (result, "0x6p-2 33") == 0
	    || strcmp (result, "0xcp-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 0.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.0La %d", 1.51L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x2p+0 33") == 0
	    || strcmp (result, "0x3p-1 33") == 0
	    || strcmp (result, "0x6p-2 33") == 0
	    || strcmp (result, "0xcp-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 1.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.1La %d", 1.51L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.8p+0 33") == 0
	    || strcmp (result, "0x3.0p-1 33") == 0
	    || strcmp (result, "0x6.1p-2 33") == 0
	    || strcmp (result, "0xc.1p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 2.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.2La %d", 1.51L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.83p+0 33") == 0
	    || strcmp (result, "0x3.05p-1 33") == 0
	    || strcmp (result, "0x6.0ap-2 33") == 0
	    || strcmp (result, "0xc.14p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding with precision 3.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.3La %d", 1.51L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.829p+0 33") == 0
	    || strcmp (result, "0x3.052p-1 33") == 0
	    || strcmp (result, "0x6.0a4p-2 33") == 0
	    || strcmp (result, "0xc.148p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding can turn a ...FFF into a ...000.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.3La %d", 1.49999L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.800p+0 33") == 0
	    || strcmp (result, "0x3.000p-1 33") == 0
	    || strcmp (result, "0x6.000p-2 33") == 0
	    || strcmp (result, "0xc.000p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Rounding can turn a ...FFF into a ...000.
       This shows a MacOS X 10.3.9 (Darwin 7.9) bug and a
       glibc 2.4 bug <http://sourceware.org/bugzilla/show_bug.cgi?id=2908>.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.1La %d", 1.999L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.0p+1 33") == 0
	    || strcmp (result, "0x2.0p+0 33") == 0
	    || strcmp (result, "0x4.0p-1 33") == 0
	    || strcmp (result, "0x8.0p-2 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%10La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "  0x1.cp+0 33") == 0
	    || strcmp (result, "  0x3.8p-1 33") == 0
	    || strcmp (result, "    0x7p-2 33") == 0
	    || strcmp (result, "    0xep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.10La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.c000000000p+0 33") == 0
	    || strcmp (result, "0x3.8000000000p-1 33") == 0
	    || strcmp (result, "0x7.0000000000p-2 33") == 0
	    || strcmp (result, "0xe.0000000000p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Large precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.50La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.c0000000000000000000000000000000000000000000000000p+0 33") == 0
	    || strcmp (result, "0x3.80000000000000000000000000000000000000000000000000p-1 33") == 0
	    || strcmp (result, "0x7.00000000000000000000000000000000000000000000000000p-2 33") == 0
	    || strcmp (result, "0xe.00000000000000000000000000000000000000000000000000p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-10La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.cp+0   33") == 0
	    || strcmp (result, "0x3.8p-1   33") == 0
	    || strcmp (result, "0x7p-2     33") == 0
	    || strcmp (result, "0xep-3     33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "+0x1.cp+0 33") == 0
	    || strcmp (result, "+0x3.8p-1 33") == 0
	    || strcmp (result, "+0x7p-2 33") == 0
	    || strcmp (result, "+0xep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, " 0x1.cp+0 33") == 0
	    || strcmp (result, " 0x3.8p-1 33") == 0
	    || strcmp (result, " 0x7p-2 33") == 0
	    || strcmp (result, " 0xep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.cp+0 33") == 0
	    || strcmp (result, "0x3.8p-1 33") == 0
	    || strcmp (result, "0x7.p-2 33") == 0
	    || strcmp (result, "0xe.p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#La %d", 1.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x1.p+0 33") == 0
	    || strcmp (result, "0x2.p-1 33") == 0
	    || strcmp (result, "0x4.p-2 33") == 0
	    || strcmp (result, "0x8.p-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%010La %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0x001.cp+0 33") == 0
	    || strcmp (result, "0x003.8p-1 33") == 0
	    || strcmp (result, "0x00007p-2 33") == 0
	    || strcmp (result, "0x0000ep-3 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%010La %d", 1.0L / returnZero(), 33, 44, 55);
    /* "0000000inf 33" is not a valid result; see
       <http://lists.gnu.org/archive/html/bug-gnulib/2007-04/msg00107.html> */
    EXPECT_TRUE (strcmp (result, "       inf 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050La %d", 0.0L / returnZero(), 33, 44, 55);
    /* "0000000nan 33" is not a valid result; see
       <http://lists.gnu.org/archive/html/bug-gnulib/2007-04/msg00107.html> */
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif //defined (WIP_NOT_IMPLEMENTED)

  /* Test the support of the %f format directive.  */

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", 12.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", 1234567.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234567.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small and large positive numbers.  */
    static struct { double value; const char *string; } data[] =
      {
	{ 1.234321234321234e-37, "0.000000" },
	{ 1.234321234321234e-36, "0.000000" },
	{ 1.234321234321234e-35, "0.000000" },
	{ 1.234321234321234e-34, "0.000000" },
	{ 1.234321234321234e-33, "0.000000" },
	{ 1.234321234321234e-32, "0.000000" },
	{ 1.234321234321234e-31, "0.000000" },
	{ 1.234321234321234e-30, "0.000000" },
	{ 1.234321234321234e-29, "0.000000" },
	{ 1.234321234321234e-28, "0.000000" },
	{ 1.234321234321234e-27, "0.000000" },
	{ 1.234321234321234e-26, "0.000000" },
	{ 1.234321234321234e-25, "0.000000" },
	{ 1.234321234321234e-24, "0.000000" },
	{ 1.234321234321234e-23, "0.000000" },
	{ 1.234321234321234e-22, "0.000000" },
	{ 1.234321234321234e-21, "0.000000" },
	{ 1.234321234321234e-20, "0.000000" },
	{ 1.234321234321234e-19, "0.000000" },
	{ 1.234321234321234e-18, "0.000000" },
	{ 1.234321234321234e-17, "0.000000" },
	{ 1.234321234321234e-16, "0.000000" },
	{ 1.234321234321234e-15, "0.000000" },
	{ 1.234321234321234e-14, "0.000000" },
	{ 1.234321234321234e-13, "0.000000" },
	{ 1.234321234321234e-12, "0.000000" },
	{ 1.234321234321234e-11, "0.000000" },
	{ 1.234321234321234e-10, "0.000000" },
	{ 1.234321234321234e-9, "0.000000" },
	{ 1.234321234321234e-8, "0.000000" },
	{ 1.234321234321234e-7, "0.000000" },
	{ 1.234321234321234e-6, "0.000001" },
	{ 1.234321234321234e-5, "0.000012" },
	{ 1.234321234321234e-4, "0.000123" },
	{ 1.234321234321234e-3, "0.001234" },
	{ 1.234321234321234e-2, "0.012343" },
	{ 1.234321234321234e-1, "0.123432" },
	{ 1.234321234321234, "1.234321" },
	{ 1.234321234321234e1, "12.343212" },
	{ 1.234321234321234e2, "123.432123" },
	{ 1.234321234321234e3, "1234.321234" },
	{ 1.234321234321234e4, "12343.212343" },
	{ 1.234321234321234e5, "123432.123432" },
	{ 1.234321234321234e6, "1234321.234321" },
	{ 1.234321234321234e7, "12343212.343212" },
	{ 1.234321234321234e8, "123432123.432123" },
	{ 1.234321234321234e9, "1234321234.321234" },
	{ 1.234321234321234e10, "12343212343.2123**" },
	{ 1.234321234321234e11, "123432123432.123***" },
	{ 1.234321234321234e12, "1234321234321.23****" },
	{ 1.234321234321234e13, "12343212343212.3*****" },
	{ 1.234321234321234e14, "123432123432123.******" },
	{ 1.234321234321234e15, "1234321234321234.000000" },
	{ 1.234321234321234e16, "123432123432123**.000000" },
	{ 1.234321234321234e17, "123432123432123***.000000" },
	{ 1.234321234321234e18, "123432123432123****.000000" },
	{ 1.234321234321234e19, "123432123432123*****.000000" },
	{ 1.234321234321234e20, "123432123432123******.000000" },
	{ 1.234321234321234e21, "123432123432123*******.000000" },
	{ 1.234321234321234e22, "123432123432123********.000000" },
	{ 1.234321234321234e23, "123432123432123*********.000000" },
	{ 1.234321234321234e24, "123432123432123**********.000000" },
	{ 1.234321234321234e25, "123432123432123***********.000000" },
	{ 1.234321234321234e26, "123432123432123************.000000" },
	{ 1.234321234321234e27, "123432123432123*************.000000" },
	{ 1.234321234321234e28, "123432123432123**************.000000" },
	{ 1.234321234321234e29, "123432123432123***************.000000" },
	{ 1.234321234321234e30, "123432123432123****************.000000" },
	{ 1.234321234321234e31, "123432123432123*****************.000000" },
	{ 1.234321234321234e32, "123432123432123******************.000000" },
	{ 1.234321234321234e33, "123432123432123*******************.000000" },
	{ 1.234321234321234e34, "123432123432123********************.000000" },
	{ 1.234321234321234e35, "123432123432123*********************.000000" },
	{ 1.234321234321234e36, "123432123432123**********************.000000" }
      };
    size_t k;
    for (k = 0; k < _countof (data); k++)
      {
	char result[100];
	int retval =
	  my_snprintf (result, sizeof (result), "%f", data[k].value);
	EXPECT_TRUE (strmatch (data[k].string, result));
	EXPECT_TRUE (retval == strlen (result));
      }
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", -0.03125, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0.031250 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", 0.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", -0.0, 33, 44, 55);

    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", 1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", false));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0
	    || strcmp (result, "-infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%f %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%10f %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "  1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-10f %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "1.750000   33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+f %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "+1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% f %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, " 1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#f %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.f %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "2. 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015f %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "00001234.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015f %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -inf 33") == 0
	    || strcmp (result, "      -infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* FLAG_ZERO with NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050f %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.f %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", 12.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", 1234567.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234567.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small and large positive numbers.  */
    static struct { long double value; const char *string; } data[] =
      {
	{ 1.234321234321234e-37L, "0.000000" },
	{ 1.234321234321234e-36L, "0.000000" },
	{ 1.234321234321234e-35L, "0.000000" },
	{ 1.234321234321234e-34L, "0.000000" },
	{ 1.234321234321234e-33L, "0.000000" },
	{ 1.234321234321234e-32L, "0.000000" },
	{ 1.234321234321234e-31L, "0.000000" },
	{ 1.234321234321234e-30L, "0.000000" },
	{ 1.234321234321234e-29L, "0.000000" },
	{ 1.234321234321234e-28L, "0.000000" },
	{ 1.234321234321234e-27L, "0.000000" },
	{ 1.234321234321234e-26L, "0.000000" },
	{ 1.234321234321234e-25L, "0.000000" },
	{ 1.234321234321234e-24L, "0.000000" },
	{ 1.234321234321234e-23L, "0.000000" },
	{ 1.234321234321234e-22L, "0.000000" },
	{ 1.234321234321234e-21L, "0.000000" },
	{ 1.234321234321234e-20L, "0.000000" },
	{ 1.234321234321234e-19L, "0.000000" },
	{ 1.234321234321234e-18L, "0.000000" },
	{ 1.234321234321234e-17L, "0.000000" },
	{ 1.234321234321234e-16L, "0.000000" },
	{ 1.234321234321234e-15L, "0.000000" },
	{ 1.234321234321234e-14L, "0.000000" },
	{ 1.234321234321234e-13L, "0.000000" },
	{ 1.234321234321234e-12L, "0.000000" },
	{ 1.234321234321234e-11L, "0.000000" },
	{ 1.234321234321234e-10L, "0.000000" },
	{ 1.234321234321234e-9L, "0.000000" },
	{ 1.234321234321234e-8L, "0.000000" },
	{ 1.234321234321234e-7L, "0.000000" },
	{ 1.234321234321234e-6L, "0.000001" },
	{ 1.234321234321234e-5L, "0.000012" },
	{ 1.234321234321234e-4L, "0.000123" },
	{ 1.234321234321234e-3L, "0.001234" },
	{ 1.234321234321234e-2L, "0.012343" },
	{ 1.234321234321234e-1L, "0.123432" },
	{ 1.234321234321234L, "1.234321" },
	{ 1.234321234321234e1L, "12.343212" },
	{ 1.234321234321234e2L, "123.432123" },
	{ 1.234321234321234e3L, "1234.321234" },
	{ 1.234321234321234e4L, "12343.212343" },
	{ 1.234321234321234e5L, "123432.123432" },
	{ 1.234321234321234e6L, "1234321.234321" },
	{ 1.234321234321234e7L, "12343212.343212" },
	{ 1.234321234321234e8L, "123432123.432123" },
	{ 1.234321234321234e9L, "1234321234.321234" },
	{ 1.234321234321234e10L, "12343212343.2123**" },
	{ 1.234321234321234e11L, "123432123432.123***" },
	{ 1.234321234321234e12L, "1234321234321.23****" },
	{ 1.234321234321234e13L, "12343212343212.3*****" },
	{ 1.234321234321234e14L, "123432123432123.******" },
	{ 1.234321234321234e15L, "1234321234321234.000000" },
	{ 1.234321234321234e16L, "123432123432123**.000000" },
	{ 1.234321234321234e17L, "123432123432123***.000000" },
	{ 1.234321234321234e18L, "123432123432123****.000000" },
	{ 1.234321234321234e19L, "123432123432123*****.000000" },
	{ 1.234321234321234e20L, "123432123432123******.000000" },
	{ 1.234321234321234e21L, "123432123432123*******.000000" },
	{ 1.234321234321234e22L, "123432123432123********.000000" },
	{ 1.234321234321234e23L, "123432123432123*********.000000" },
	{ 1.234321234321234e24L, "123432123432123**********.000000" },
	{ 1.234321234321234e25L, "123432123432123***********.000000" },
	{ 1.234321234321234e26L, "123432123432123************.000000" },
	{ 1.234321234321234e27L, "123432123432123*************.000000" },
	{ 1.234321234321234e28L, "123432123432123**************.000000" },
	{ 1.234321234321234e29L, "123432123432123***************.000000" },
	{ 1.234321234321234e30L, "123432123432123****************.000000" },
	{ 1.234321234321234e31L, "123432123432123*****************.000000" },
	{ 1.234321234321234e32L, "123432123432123******************.000000" },
	{ 1.234321234321234e33L, "123432123432123*******************.000000" },
	{ 1.234321234321234e34L, "123432123432123********************.000000" },
	{ 1.234321234321234e35L, "123432123432123*********************.000000" },
	{ 1.234321234321234e36L, "123432123432123**********************.000000" }
      };
    size_t k;
    for (k = 0; k < _countof (data); k++)
      {
	char result[100];
	int retval =
	  my_snprintf (result, sizeof (result), "%Lf", data[k].value);
	EXPECT_TRUE (strmatch (data[k].string, result));
	EXPECT_TRUE (retval == strlen (result));
      }
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", -0.03125L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0.031250 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", 0.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", -0.0L, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", 1.0L / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", true));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", -1.0L / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0
	    || strcmp (result, "-infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* NaN.  */
    static long double zero = 0.0L;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", zero / zero, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif
#if CHECK_PRINTF_SAFE && ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_))
  { /* Quiet NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0xC3333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  {
    /* Signalling NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  /* The isnanl function should recognize Pseudo-NaNs, Pseudo-Infinities,
     Pseudo-Zeroes, Unnormalized Numbers, and Pseudo-Denormals, as defined in
       Intel IA-64 Architecture Software Developer's Manual, Volume 1:
       Application Architecture.
       Table 5-2 "Floating-Point Register Encodings"
       Figure 5-6 "Memory to Floating-Point Register Data Translation"
   */
  { /* Pseudo-NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Infinity.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Zero.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Unnormalized number.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Denormal.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lf %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%10Lf %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "  1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-10Lf %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "1.750000   33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+Lf %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "+1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% Lf %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, " 1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#Lf %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "1.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.Lf %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (result != NULL);
    EXPECT_TRUE (strcmp (result, "2. 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015Lf %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "00001234.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015Lf %d", -1.0L / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -inf 33") == 0
	    || strcmp (result, "      -infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* FLAG_ZERO with NaN.  */
    static long double zero = 0.0L;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050Lf %d", zero / zero, 33, 44, 55);
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.Lf %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_NOT_IMPLEMENTED)
  /* Test the support of the %F format directive.  */

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", 12.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", 1234567.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234567.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", -0.03125, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0.031250 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", 0.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", -0.0, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", 1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", true));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-INF 33") == 0
	    || strcmp (result, "-INFINITY 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%F %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 1)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015F %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "00001234.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015F %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -INF 33") == 0
	    || strcmp (result, "      -INFINITY 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.F %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", 12.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12.750000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", 1234567.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234567.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", -0.03125L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0.031250 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", 0.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", -0.0L, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", 1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", true));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", -1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-INF 33") == 0
	    || strcmp (result, "-INFINITY 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* NaN.  */
    static long double zero = 0.0L;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%LF %d", zero / zero, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 1)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015LF %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "00001234.000000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015LF %d", -1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -INF 33") == 0
	    || strcmp (result, "      -INFINITY 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.LF %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1234 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif // defined (WIP_NOT_IMPLEMENTED)


  /* Test the support of the %e format directive.  */

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", 12.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.275000e+01 33") == 0
	    || strcmp (result, "1.275000e+001 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", 1234567.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.234567e+06 33") == 0
	    || strcmp (result, "1.234567e+006 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small and large positive numbers.  */
    static struct { double value; const char *string; } data[] =
      {
	{ 1.234321234321234e-37, "1.234321e-37" },
	{ 1.234321234321234e-36, "1.234321e-36" },
	{ 1.234321234321234e-35, "1.234321e-35" },
	{ 1.234321234321234e-34, "1.234321e-34" },
	{ 1.234321234321234e-33, "1.234321e-33" },
	{ 1.234321234321234e-32, "1.234321e-32" },
	{ 1.234321234321234e-31, "1.234321e-31" },
	{ 1.234321234321234e-30, "1.234321e-30" },
	{ 1.234321234321234e-29, "1.234321e-29" },
	{ 1.234321234321234e-28, "1.234321e-28" },
	{ 1.234321234321234e-27, "1.234321e-27" },
	{ 1.234321234321234e-26, "1.234321e-26" },
	{ 1.234321234321234e-25, "1.234321e-25" },
	{ 1.234321234321234e-24, "1.234321e-24" },
	{ 1.234321234321234e-23, "1.234321e-23" },
	{ 1.234321234321234e-22, "1.234321e-22" },
	{ 1.234321234321234e-21, "1.234321e-21" },
	{ 1.234321234321234e-20, "1.234321e-20" },
	{ 1.234321234321234e-19, "1.234321e-19" },
	{ 1.234321234321234e-18, "1.234321e-18" },
	{ 1.234321234321234e-17, "1.234321e-17" },
	{ 1.234321234321234e-16, "1.234321e-16" },
	{ 1.234321234321234e-15, "1.234321e-15" },
	{ 1.234321234321234e-14, "1.234321e-14" },
	{ 1.234321234321234e-13, "1.234321e-13" },
	{ 1.234321234321234e-12, "1.234321e-12" },
	{ 1.234321234321234e-11, "1.234321e-11" },
	{ 1.234321234321234e-10, "1.234321e-10" },
	{ 1.234321234321234e-9, "1.234321e-09" },
	{ 1.234321234321234e-8, "1.234321e-08" },
	{ 1.234321234321234e-7, "1.234321e-07" },
	{ 1.234321234321234e-6, "1.234321e-06" },
	{ 1.234321234321234e-5, "1.234321e-05" },
	{ 1.234321234321234e-4, "1.234321e-04" },
	{ 1.234321234321234e-3, "1.234321e-03" },
	{ 1.234321234321234e-2, "1.234321e-02" },
	{ 1.234321234321234e-1, "1.234321e-01" },
	{ 1.234321234321234, "1.234321e+00" },
	{ 1.234321234321234e1, "1.234321e+01" },
	{ 1.234321234321234e2, "1.234321e+02" },
	{ 1.234321234321234e3, "1.234321e+03" },
	{ 1.234321234321234e4, "1.234321e+04" },
	{ 1.234321234321234e5, "1.234321e+05" },
	{ 1.234321234321234e6, "1.234321e+06" },
	{ 1.234321234321234e7, "1.234321e+07" },
	{ 1.234321234321234e8, "1.234321e+08" },
	{ 1.234321234321234e9, "1.234321e+09" },
	{ 1.234321234321234e10, "1.234321e+10" },
	{ 1.234321234321234e11, "1.234321e+11" },
	{ 1.234321234321234e12, "1.234321e+12" },
	{ 1.234321234321234e13, "1.234321e+13" },
	{ 1.234321234321234e14, "1.234321e+14" },
	{ 1.234321234321234e15, "1.234321e+15" },
	{ 1.234321234321234e16, "1.234321e+16" },
	{ 1.234321234321234e17, "1.234321e+17" },
	{ 1.234321234321234e18, "1.234321e+18" },
	{ 1.234321234321234e19, "1.234321e+19" },
	{ 1.234321234321234e20, "1.234321e+20" },
	{ 1.234321234321234e21, "1.234321e+21" },
	{ 1.234321234321234e22, "1.234321e+22" },
	{ 1.234321234321234e23, "1.234321e+23" },
	{ 1.234321234321234e24, "1.234321e+24" },
	{ 1.234321234321234e25, "1.234321e+25" },
	{ 1.234321234321234e26, "1.234321e+26" },
	{ 1.234321234321234e27, "1.234321e+27" },
	{ 1.234321234321234e28, "1.234321e+28" },
	{ 1.234321234321234e29, "1.234321e+29" },
	{ 1.234321234321234e30, "1.234321e+30" },
	{ 1.234321234321234e31, "1.234321e+31" },
	{ 1.234321234321234e32, "1.234321e+32" },
	{ 1.234321234321234e33, "1.234321e+33" },
	{ 1.234321234321234e34, "1.234321e+34" },
	{ 1.234321234321234e35, "1.234321e+35" },
	{ 1.234321234321234e36, "1.234321e+36" }
      };
    size_t k;
    for (k = 0; k < _countof (data); k++)
      {
	char result[100];
	int retval =
	  my_snprintf (result, sizeof (result), "%e", data[k].value);
	const char *expected = data[k].string;
	EXPECT_TRUE (result != NULL);
	EXPECT_TRUE (strcmp (result, expected) == 0
		/* Some implementations produce exponents with 3 digits.  */
		|| (strlen (result) == strlen (expected) + 1
		    && memcmp (result, expected, strlen (expected) - 2) == 0
		    && result[strlen (expected) - 2] == '0'
		    && strcmp (result + strlen (expected) - 1,
			       expected + strlen (expected) - 2)
		       == 0));
	EXPECT_TRUE (retval == strlen (result));
      }
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", -0.03125, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-3.125000e-02 33") == 0
	    || strcmp (result, "-3.125000e-002 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", 0.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0.000000e+00 33") == 0
	    || strcmp (result, "0.000000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", -0.0, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0.000000e+00 33") == 0
	      || strcmp (result, "-0.000000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", 1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", true));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0
	    || strcmp (result, "-infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%e %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%15e %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "   1.750000e+00 33") == 0
	    || strcmp (result, "  1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-15e %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.750000e+00    33") == 0
	    || strcmp (result, "1.750000e+000   33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+e %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "+1.750000e+00 33") == 0
	    || strcmp (result, "+1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% e %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, " 1.750000e+00 33") == 0
	    || strcmp (result, " 1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#e %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.750000e+00 33") == 0
	    || strcmp (result, "1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.e %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "2.e+00 33") == 0
	    || strcmp (result, "2.e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.e %d", 9.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.e+01 33") == 0
	    || strcmp (result, "1.e+001 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015e %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0001.234000e+03 33") == 0
	    || strcmp (result, "001.234000e+003 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015e %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -inf 33") == 0
	    || strcmp (result, "      -infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* FLAG_ZERO with NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050e %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.e %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1e+03 33") == 0
	    || strcmp (result, "1e+003 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", 12.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.275000e+01 33") == 0
              || strcmp (result, "1.275000e+001 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", 1234567.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.234567e+06 33") == 0
              || strcmp (result, "1.234567e+006 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small and large positive numbers.  */
    static struct { long double value; const char *string; const char *altstring;} data[] =
      {
	{ 1.234321234321234e-37L,   "1.234321e-37", "1.234321e-037"},
	{ 1.234321234321234e-36L,   "1.234321e-36", "1.234321e-036"},
	{ 1.234321234321234e-35L,   "1.234321e-35", "1.234321e-035"},
	{ 1.234321234321234e-34L,   "1.234321e-34", "1.234321e-034"},
	{ 1.234321234321234e-33L,   "1.234321e-33", "1.234321e-033"},
	{ 1.234321234321234e-32L,   "1.234321e-32", "1.234321e-032"},
	{ 1.234321234321234e-31L,   "1.234321e-31", "1.234321e-031"},
	{ 1.234321234321234e-30L,   "1.234321e-30", "1.234321e-030"},
	{ 1.234321234321234e-29L,   "1.234321e-29", "1.234321e-029"},
	{ 1.234321234321234e-28L,   "1.234321e-28", "1.234321e-028"},
	{ 1.234321234321234e-27L,   "1.234321e-27", "1.234321e-027"},
	{ 1.234321234321234e-26L,   "1.234321e-26", "1.234321e-026"},
	{ 1.234321234321234e-25L,   "1.234321e-25", "1.234321e-025"},
	{ 1.234321234321234e-24L,   "1.234321e-24", "1.234321e-024"},
	{ 1.234321234321234e-23L,   "1.234321e-23", "1.234321e-023"},
	{ 1.234321234321234e-22L,   "1.234321e-22", "1.234321e-022"},
	{ 1.234321234321234e-21L,   "1.234321e-21", "1.234321e-021"},
	{ 1.234321234321234e-20L,   "1.234321e-20", "1.234321e-020"},
	{ 1.234321234321234e-19L,   "1.234321e-19", "1.234321e-019"},
	{ 1.234321234321234e-18L,   "1.234321e-18", "1.234321e-018"},
	{ 1.234321234321234e-17L,   "1.234321e-17", "1.234321e-017"},
	{ 1.234321234321234e-16L,   "1.234321e-16", "1.234321e-016"},
	{ 1.234321234321234e-15L,   "1.234321e-15", "1.234321e-015"},
	{ 1.234321234321234e-14L,   "1.234321e-14", "1.234321e-014"},
	{ 1.234321234321234e-13L,   "1.234321e-13", "1.234321e-013"},
	{ 1.234321234321234e-12L,   "1.234321e-12", "1.234321e-012"},
	{ 1.234321234321234e-11L,   "1.234321e-11", "1.234321e-011"},
	{ 1.234321234321234e-10L,   "1.234321e-10", "1.234321e-010"},
	{ 1.234321234321234e-9L,    "1.234321e-09", "1.234321e-009"},
	{ 1.234321234321234e-8L,    "1.234321e-08", "1.234321e-008"},
	{ 1.234321234321234e-7L,    "1.234321e-07", "1.234321e-007"},
	{ 1.234321234321234e-6L,    "1.234321e-06", "1.234321e-006"},
	{ 1.234321234321234e-5L,    "1.234321e-05", "1.234321e-005"},
	{ 1.234321234321234e-4L,    "1.234321e-04", "1.234321e-004"},
	{ 1.234321234321234e-3L,    "1.234321e-03", "1.234321e-003"},
	{ 1.234321234321234e-2L,    "1.234321e-02", "1.234321e-002"},
	{ 1.234321234321234e-1L,    "1.234321e-01", "1.234321e-001"},
	{ 1.234321234321234L,       "1.234321e+00", "1.234321e+000"},
	{ 1.234321234321234e1L,     "1.234321e+01", "1.234321e+001"},
	{ 1.234321234321234e2L,     "1.234321e+02", "1.234321e+002"},
	{ 1.234321234321234e3L,     "1.234321e+03", "1.234321e+003"},
	{ 1.234321234321234e4L,     "1.234321e+04", "1.234321e+004"},
	{ 1.234321234321234e5L,     "1.234321e+05", "1.234321e+005"},
	{ 1.234321234321234e6L,     "1.234321e+06", "1.234321e+006"},
	{ 1.234321234321234e7L,     "1.234321e+07", "1.234321e+007"},
	{ 1.234321234321234e8L,     "1.234321e+08", "1.234321e+008"},
	{ 1.234321234321234e9L,     "1.234321e+09", "1.234321e+009"},
	{ 1.234321234321234e10L,    "1.234321e+10", "1.234321e+010"},
	{ 1.234321234321234e11L,    "1.234321e+11", "1.234321e+011"},
	{ 1.234321234321234e12L,    "1.234321e+12", "1.234321e+012"},
	{ 1.234321234321234e13L,    "1.234321e+13", "1.234321e+013"},
	{ 1.234321234321234e14L,    "1.234321e+14", "1.234321e+014"},
	{ 1.234321234321234e15L,    "1.234321e+15", "1.234321e+015"},
	{ 1.234321234321234e16L,    "1.234321e+16", "1.234321e+016"},
	{ 1.234321234321234e17L,    "1.234321e+17", "1.234321e+017"},
	{ 1.234321234321234e18L,    "1.234321e+18", "1.234321e+018"},
	{ 1.234321234321234e19L,    "1.234321e+19", "1.234321e+019"},
	{ 1.234321234321234e20L,    "1.234321e+20", "1.234321e+020"},
	{ 1.234321234321234e21L,    "1.234321e+21", "1.234321e+021"},
	{ 1.234321234321234e22L,    "1.234321e+22", "1.234321e+022"},
	{ 1.234321234321234e23L,    "1.234321e+23", "1.234321e+023"},
	{ 1.234321234321234e24L,    "1.234321e+24", "1.234321e+024"},
	{ 1.234321234321234e25L,    "1.234321e+25", "1.234321e+025"},
	{ 1.234321234321234e26L,    "1.234321e+26", "1.234321e+026"},
	{ 1.234321234321234e27L,    "1.234321e+27", "1.234321e+027"},
	{ 1.234321234321234e28L,    "1.234321e+28", "1.234321e+028"},
	{ 1.234321234321234e29L,    "1.234321e+29", "1.234321e+029"},
	{ 1.234321234321234e30L,    "1.234321e+30", "1.234321e+030"},
	{ 1.234321234321234e31L,    "1.234321e+31", "1.234321e+031"},
	{ 1.234321234321234e32L,    "1.234321e+32", "1.234321e+032"},
	{ 1.234321234321234e33L,    "1.234321e+33", "1.234321e+033"},
	{ 1.234321234321234e34L,    "1.234321e+34", "1.234321e+034"},
	{ 1.234321234321234e35L,    "1.234321e+35", "1.234321e+035"},
	{ 1.234321234321234e36L,    "1.234321e+36", "1.234321e+036"}
      };
    size_t k;
    for (k = 0; k < _countof (data); k++)
      {
	char result[100];
	int retval =
	  my_snprintf (result, sizeof (result), "%Le", data[k].value);
	EXPECT_TRUE (strcmp (result, data[k].string) == 0 || strcmp (result, data[k].altstring) == 0);
	EXPECT_TRUE (retval == strlen (result));
      }
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", -0.03125L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-3.125000e-02 33") == 0 || strcmp (result, "-3.125000e-002 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", 0.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0.000000e+00 33") == 0 || strcmp (result, "0.000000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", -0.0L, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0.000000e+00 33") == 0 || strcmp (result, "-0.000000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", 1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", true));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", -1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0
	    || strcmp (result, "-infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* NaN.  */
    static long double zero = 0.0L;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", zero / zero, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif
#if CHECK_PRINTF_SAFE && ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_))
  { /* Quiet NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0xC3333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  {
    /* Signalling NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  /* The isnanl function should recognize Pseudo-NaNs, Pseudo-Infinities,
     Pseudo-Zeroes, Unnormalized Numbers, and Pseudo-Denormals, as defined in
       Intel IA-64 Architecture Software Developer's Manual, Volume 1:
       Application Architecture.
       Table 5-2 "Floating-Point Register Encodings"
       Figure 5-6 "Memory to Floating-Point Register Data Translation"
   */
  { /* Pseudo-NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Infinity.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Zero.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Unnormalized number.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Denormal.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Le %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%15Le %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "   1.750000e+00 33") == 0 || strcmp (result, "  1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-15Le %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.750000e+00    33") == 0 || strcmp (result, "1.750000e+000   33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+Le %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "+1.750000e+00 33") == 0 || strcmp (result, "+1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% Le %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, " 1.750000e+00 33") == 0 || strcmp (result, " 1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#Le %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.750000e+00 33") == 0 || strcmp (result, "1.750000e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.Le %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "2.e+00 33") == 0 || strcmp (result, "2.e+000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.Le %d", 9.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.e+01 33") == 0 || strcmp (result, "1.e+001 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015Le %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0001.234000e+03 33") == 0 || strcmp (result, "001.234000e+003 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015Le %d", -1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -inf 33") == 0
	    || strcmp (result, "      -infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* FLAG_ZERO with NaN.  */
    static long double zero = 0.0L;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050Le %d", zero / zero, 33, 44, 55);
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.Le %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1e+03 33") == 0 || strcmp (result, "1e+003 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  /* Test the support of the %g format directive.  */

  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", 12.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", 1234567.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.23457e+06 33") == 0 || strcmp (result, "1.23457e+006 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small and large positive numbers.  */
    static struct { double value; const char *string; } data[] =
      {
	{ 1.234321234321234e-37, "1.23432e-37" },
	{ 1.234321234321234e-36, "1.23432e-36" },
	{ 1.234321234321234e-35, "1.23432e-35" },
	{ 1.234321234321234e-34, "1.23432e-34" },
	{ 1.234321234321234e-33, "1.23432e-33" },
	{ 1.234321234321234e-32, "1.23432e-32" },
	{ 1.234321234321234e-31, "1.23432e-31" },
	{ 1.234321234321234e-30, "1.23432e-30" },
	{ 1.234321234321234e-29, "1.23432e-29" },
	{ 1.234321234321234e-28, "1.23432e-28" },
	{ 1.234321234321234e-27, "1.23432e-27" },
	{ 1.234321234321234e-26, "1.23432e-26" },
	{ 1.234321234321234e-25, "1.23432e-25" },
	{ 1.234321234321234e-24, "1.23432e-24" },
	{ 1.234321234321234e-23, "1.23432e-23" },
	{ 1.234321234321234e-22, "1.23432e-22" },
	{ 1.234321234321234e-21, "1.23432e-21" },
	{ 1.234321234321234e-20, "1.23432e-20" },
	{ 1.234321234321234e-19, "1.23432e-19" },
	{ 1.234321234321234e-18, "1.23432e-18" },
	{ 1.234321234321234e-17, "1.23432e-17" },
	{ 1.234321234321234e-16, "1.23432e-16" },
	{ 1.234321234321234e-15, "1.23432e-15" },
	{ 1.234321234321234e-14, "1.23432e-14" },
	{ 1.234321234321234e-13, "1.23432e-13" },
	{ 1.234321234321234e-12, "1.23432e-12" },
	{ 1.234321234321234e-11, "1.23432e-11" },
	{ 1.234321234321234e-10, "1.23432e-10" },
	{ 1.234321234321234e-9, "1.23432e-09" },
	{ 1.234321234321234e-8, "1.23432e-08" },
	{ 1.234321234321234e-7, "1.23432e-07" },
	{ 1.234321234321234e-6, "1.23432e-06" },
	{ 1.234321234321234e-5, "1.23432e-05" },
	{ 1.234321234321234e-4, "0.000123432" },
	{ 1.234321234321234e-3, "0.00123432" },
	{ 1.234321234321234e-2, "0.0123432" },
	{ 1.234321234321234e-1, "0.123432" },
	{ 1.234321234321234, "1.23432" },
	{ 1.234321234321234e1, "12.3432" },
	{ 1.234321234321234e2, "123.432" },
	{ 1.234321234321234e3, "1234.32" },
	{ 1.234321234321234e4, "12343.2" },
	{ 1.234321234321234e5, "123432" },
	{ 1.234321234321234e6, "1.23432e+06" },
	{ 1.234321234321234e7, "1.23432e+07" },
	{ 1.234321234321234e8, "1.23432e+08" },
	{ 1.234321234321234e9, "1.23432e+09" },
	{ 1.234321234321234e10, "1.23432e+10" },
	{ 1.234321234321234e11, "1.23432e+11" },
	{ 1.234321234321234e12, "1.23432e+12" },
	{ 1.234321234321234e13, "1.23432e+13" },
	{ 1.234321234321234e14, "1.23432e+14" },
	{ 1.234321234321234e15, "1.23432e+15" },
	{ 1.234321234321234e16, "1.23432e+16" },
	{ 1.234321234321234e17, "1.23432e+17" },
	{ 1.234321234321234e18, "1.23432e+18" },
	{ 1.234321234321234e19, "1.23432e+19" },
	{ 1.234321234321234e20, "1.23432e+20" },
	{ 1.234321234321234e21, "1.23432e+21" },
	{ 1.234321234321234e22, "1.23432e+22" },
	{ 1.234321234321234e23, "1.23432e+23" },
	{ 1.234321234321234e24, "1.23432e+24" },
	{ 1.234321234321234e25, "1.23432e+25" },
	{ 1.234321234321234e26, "1.23432e+26" },
	{ 1.234321234321234e27, "1.23432e+27" },
	{ 1.234321234321234e28, "1.23432e+28" },
	{ 1.234321234321234e29, "1.23432e+29" },
	{ 1.234321234321234e30, "1.23432e+30" },
	{ 1.234321234321234e31, "1.23432e+31" },
	{ 1.234321234321234e32, "1.23432e+32" },
	{ 1.234321234321234e33, "1.23432e+33" },
	{ 1.234321234321234e34, "1.23432e+34" },
	{ 1.234321234321234e35, "1.23432e+35" },
	{ 1.234321234321234e36, "1.23432e+36" }
      };
    size_t k;
    for (k = 0; k < _countof (data); k++)
      {
	char result[100];
	int retval =
	  my_snprintf (result, sizeof (result), "%g", data[k].value);
	const char *expected = data[k].string;
	EXPECT_TRUE (strcmp (result, expected) == 0
		/* Some implementations produce exponents with 3 digits.  */
		|| (expected[strlen (expected) - 4] == 'e'
		    && strlen (result) == strlen (expected) + 1
		    && memcmp (result, expected, strlen (expected) - 2) == 0
		    && result[strlen (expected) - 2] == '0'
		    && strcmp (result + strlen (expected) - 1,
			       expected + strlen (expected) - 2)
		       == 0));
	EXPECT_TRUE (retval == strlen (result));
      }
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", -0.03125, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0.03125 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", 0.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", -0.0, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", 1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", true));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0
	    || strcmp (result, "-infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%g %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif
  
  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%10g %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "      1.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-10g %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.75       33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+g %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "+1.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% g %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, " 1.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#g %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.75000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.g %d", 1.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "2. 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.g %d", 9.75, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.e+01 33") == 0
	    || strcmp (result, "1.e+001 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%010g %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0000001234 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015g %d", -1.0 / returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -inf 33") == 0
	    || strcmp (result, "      -infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* FLAG_ZERO with NaN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050g %d", NaN (), 33, 44, 55);
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.g %d", 1234.0, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1e+03 33") == 0
	    || strcmp (result, "1e+003 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_NOT_IMPLEMENTED)
  { /* A positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", 12.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "12.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* A larger positive number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", 1234567.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.23457e+06 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Small and large positive numbers.  */
    static struct { long double value; const char *string; } data[] =
      {
	{ 1.234321234321234e-37L, "1.23432e-37" },
	{ 1.234321234321234e-36L, "1.23432e-36" },
	{ 1.234321234321234e-35L, "1.23432e-35" },
	{ 1.234321234321234e-34L, "1.23432e-34" },
	{ 1.234321234321234e-33L, "1.23432e-33" },
	{ 1.234321234321234e-32L, "1.23432e-32" },
	{ 1.234321234321234e-31L, "1.23432e-31" },
	{ 1.234321234321234e-30L, "1.23432e-30" },
	{ 1.234321234321234e-29L, "1.23432e-29" },
	{ 1.234321234321234e-28L, "1.23432e-28" },
	{ 1.234321234321234e-27L, "1.23432e-27" },
	{ 1.234321234321234e-26L, "1.23432e-26" },
	{ 1.234321234321234e-25L, "1.23432e-25" },
	{ 1.234321234321234e-24L, "1.23432e-24" },
	{ 1.234321234321234e-23L, "1.23432e-23" },
	{ 1.234321234321234e-22L, "1.23432e-22" },
	{ 1.234321234321234e-21L, "1.23432e-21" },
	{ 1.234321234321234e-20L, "1.23432e-20" },
	{ 1.234321234321234e-19L, "1.23432e-19" },
	{ 1.234321234321234e-18L, "1.23432e-18" },
	{ 1.234321234321234e-17L, "1.23432e-17" },
	{ 1.234321234321234e-16L, "1.23432e-16" },
	{ 1.234321234321234e-15L, "1.23432e-15" },
	{ 1.234321234321234e-14L, "1.23432e-14" },
	{ 1.234321234321234e-13L, "1.23432e-13" },
	{ 1.234321234321234e-12L, "1.23432e-12" },
	{ 1.234321234321234e-11L, "1.23432e-11" },
	{ 1.234321234321234e-10L, "1.23432e-10" },
	{ 1.234321234321234e-9L, "1.23432e-09" },
	{ 1.234321234321234e-8L, "1.23432e-08" },
	{ 1.234321234321234e-7L, "1.23432e-07" },
	{ 1.234321234321234e-6L, "1.23432e-06" },
	{ 1.234321234321234e-5L, "1.23432e-05" },
	{ 1.234321234321234e-4L, "0.000123432" },
	{ 1.234321234321234e-3L, "0.00123432" },
	{ 1.234321234321234e-2L, "0.0123432" },
	{ 1.234321234321234e-1L, "0.123432" },
	{ 1.234321234321234L, "1.23432" },
	{ 1.234321234321234e1L, "12.3432" },
	{ 1.234321234321234e2L, "123.432" },
	{ 1.234321234321234e3L, "1234.32" },
	{ 1.234321234321234e4L, "12343.2" },
	{ 1.234321234321234e5L, "123432" },
	{ 1.234321234321234e6L, "1.23432e+06" },
	{ 1.234321234321234e7L, "1.23432e+07" },
	{ 1.234321234321234e8L, "1.23432e+08" },
	{ 1.234321234321234e9L, "1.23432e+09" },
	{ 1.234321234321234e10L, "1.23432e+10" },
	{ 1.234321234321234e11L, "1.23432e+11" },
	{ 1.234321234321234e12L, "1.23432e+12" },
	{ 1.234321234321234e13L, "1.23432e+13" },
	{ 1.234321234321234e14L, "1.23432e+14" },
	{ 1.234321234321234e15L, "1.23432e+15" },
	{ 1.234321234321234e16L, "1.23432e+16" },
	{ 1.234321234321234e17L, "1.23432e+17" },
	{ 1.234321234321234e18L, "1.23432e+18" },
	{ 1.234321234321234e19L, "1.23432e+19" },
	{ 1.234321234321234e20L, "1.23432e+20" },
	{ 1.234321234321234e21L, "1.23432e+21" },
	{ 1.234321234321234e22L, "1.23432e+22" },
	{ 1.234321234321234e23L, "1.23432e+23" },
	{ 1.234321234321234e24L, "1.23432e+24" },
	{ 1.234321234321234e25L, "1.23432e+25" },
	{ 1.234321234321234e26L, "1.23432e+26" },
	{ 1.234321234321234e27L, "1.23432e+27" },
	{ 1.234321234321234e28L, "1.23432e+28" },
	{ 1.234321234321234e29L, "1.23432e+29" },
	{ 1.234321234321234e30L, "1.23432e+30" },
	{ 1.234321234321234e31L, "1.23432e+31" },
	{ 1.234321234321234e32L, "1.23432e+32" },
	{ 1.234321234321234e33L, "1.23432e+33" },
	{ 1.234321234321234e34L, "1.23432e+34" },
	{ 1.234321234321234e35L, "1.23432e+35" },
	{ 1.234321234321234e36L, "1.23432e+36" }
      };
    size_t k;
    for (k = 0; k < _countof (data); k++)
      {
	char result[100];
	int retval =
	  my_snprintf (result, sizeof (result), "%Lg", data[k].value);
	EXPECT_TRUE (strcmp (result, data[k].string) == 0);
	EXPECT_TRUE (retval == strlen (result));
      }
  }

  { /* A negative number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", -0.03125L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-0.03125 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* Positive zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", 0.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if !defined (ANDROID) && !defined(_X86_)
  { /* Negative zero.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", -0.0L, 33, 44, 55);
    if (have_minus_zero ())
      EXPECT_TRUE (strcmp (result, "-0 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Positive infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", 1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strisinf (result, " 33", true));
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* Negative infinity.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", -1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "-inf 33") == 0
	    || strcmp (result, "-infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* NaN.  */
    static long double zero = 0.0L;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", zero / zero, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif
#if CHECK_PRINTF_SAFE && ((defined __ia64 && LDBL_MANT_DIG == 64) || (defined __x86_64__ || defined __amd64__) || (defined __i386 || defined __i386__ || defined _I386 || defined _M_IX86 || defined _X86_))
  { /* Quiet NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0xC3333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  {
    /* Signalling NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  /* The isnanl function should recognize Pseudo-NaNs, Pseudo-Infinities,
     Pseudo-Zeroes, Unnormalized Numbers, and Pseudo-Denormals, as defined in
       Intel IA-64 Architecture Software Developer's Manual, Volume 1:
       Application Architecture.
       Table 5-2 "Floating-Point Register Encodings"
       Figure 5-6 "Memory to Floating-Point Register Data Translation"
   */
  { /* Pseudo-NaN.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x40000001, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Infinity.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0xFFFF, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Zero.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4004, 0x00000000, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Unnormalized number.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x4000, 0x63333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
  { /* Pseudo-Denormal.  */
    static union { unsigned int word[4]; long double value; } x =
      { LDBL80_WORDS (0x0000, 0x83333333, 0x00000000) };
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%Lg %d", x.value, 33, 44, 55);
    EXPECT_TRUE (strlen (result) >= 3 + 3
	    && strisnan (result, 0, strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Width.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%10Lg %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "      1.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_LEFT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%-10Lg %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.75       33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SHOWSIGN.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%+Lg %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "+1.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_SPACE.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "% Lg %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, " 1.75 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#Lg %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.75000 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.Lg %d", 1.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "2. 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ALT.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%#.Lg %d", 9.75L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1.e+01 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  { /* FLAG_ZERO with finite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%010Lg %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "0000001234 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

#if defined (WIP_isinf)
  { /* FLAG_ZERO with infinite number.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%015Lg %d", -1.0L/ returnZero(), 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "           -inf 33") == 0
	    || strcmp (result, "      -infinity 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

#if !defined(BENTLEYCONFIG_OS_APPLE) // have not found a reliable way to produce NaN on these platforms.
  { /* FLAG_ZERO with NaN.  */
    static long double zero = 0.0L;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%050Lg %d", zero / zero, 33, 44, 55);
    EXPECT_TRUE (strlen (result) == 50 + 3
	    && strisnan (result, strspn (result, " "), strlen (result) - 3, 0)
	    && strcmp (result + strlen (result) - 3, " 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif

  { /* Precision.  */
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%.Lg %d", 1234.0L, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "1e+03 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }
#endif // defined (WIP_NOT_IMPLEMENTED)

#if defined (NOT_IMPLEMENTED) // http://msdn.microsoft.com/en-US/library/ms175782(v=vs.80).aspx
  /* Test the support of the %n format directive.  */

  {
    int count = -1;
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%d %n", 123, &count, 33, 44, 55);
    EXPECT_TRUE (strcmp (result, "123 ") == 0);
    EXPECT_TRUE (retval == strlen (result));
    EXPECT_TRUE (count == 4);
  }
#endif

#if defined (WIP_NOT_IMPLEMENTED)
  /* Test the support of the POSIX/XSI format strings with positions.  */

  {
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%2$d %1$d", 33, 55);
    EXPECT_TRUE (strcmp (result, "55 33") == 0);
    EXPECT_TRUE (retval == strlen (result));
  }

  /* Test the support of the grouping flag.  */

  {
    char result[100];
    int retval =
      my_snprintf (result, sizeof (result), "%'d %d", 1234567, 99);
    EXPECT_TRUE (result[strlen (result) - 1] == '9');
    EXPECT_TRUE (retval == strlen (result));
  }
#endif // defined (WIP_NOT_IMPLEMENTED)
}

#if defined (NOT_YET)
//---------------------------------------------------------------------------------------
// For localization purposes, it is important to be able to reorder parameters in the translated string
// @bsimethod                                                   Shaun.Sewall    11/2013
//---------------------------------------------------------------------------------------
TEST(printf_test, FormatStringsWithPositions)
    {
    char buffer[256];

    // Test: sprintf
    int retval = sprintf (buffer, "%1$d %2$d %3$d", 1, 2, 3);
    EXPECT_TRUE (0 == strcmp (buffer, "1 2 3"));
    EXPECT_EQ (retval, strlen (buffer));

    retval = sprintf (buffer, "%3$d %2$d %1$d", 1, 2, 3);
    EXPECT_TRUE (0 == strcmp (buffer, "3 2 1"));
    EXPECT_EQ (retval, strlen (buffer));

    retval = sprintf (buffer, "%2$d %2$d %1$d", 1, 2);
    EXPECT_TRUE (0 == strcmp (buffer, "2 2 1"));
    EXPECT_EQ (retval, strlen (buffer));

    // Test: BeStringUtilities::Snprintf
    int retval = BeStringUtilities::Snprintf (buffer, sizeof (buffer), "%1$d %2$d %3$d", 1, 2, 3);
    EXPECT_TRUE (0 == strcmp (buffer, "1 2 3"));
    EXPECT_EQ (retval, strlen (buffer));

    retval = BeStringUtilities::Snprintf (buffer, sizeof (buffer), "%3$d %2$d %1$d", 1, 2, 3);
    EXPECT_TRUE (0 == strcmp (buffer, "3 2 1"));
    EXPECT_EQ (retval, strlen (buffer));

    retval = BeStringUtilities::Snprintf (buffer, sizeof (buffer), "%2$d %2$d %1$d", 1, 2);
    EXPECT_TRUE (0 == strcmp (buffer, "2 2 1"));
    EXPECT_EQ (retval, strlen (buffer));

    // Test: Utf8PrintfString
    Utf8PrintfString s1 ("%1$d %2$d %3$d", 1, 2, 3);
    EXPECT_TRUE (0 == strcmp (s1.c_str(), "1 2 3"));

    Utf8PrintfString s2 ("%3$d %2$d %1$d", 1, 2, 3);
    EXPECT_TRUE (0 == strcmp (s1.c_str(), "3 2 1"));

    Utf8PrintfString s3 ("%2$d %2$d %1$d", 1, 2);
    EXPECT_TRUE (0 == strcmp (s1.c_str(), "2 2 1"));

    // TODO: Need to test wchar_t versions (which are expected to fail on Android)
    }
#endif // NOT_YET
