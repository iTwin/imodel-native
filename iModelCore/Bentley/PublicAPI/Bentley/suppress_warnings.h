/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/// @cond BENTLEY_SDK_Internal

#if defined (_WIN32)

#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4706) // assignment within conditional expression
#pragma warning(disable:4127) // conditional expression is constant

// This warning detects the case where you assign a signed *constant* to an unsigned variable. This comes up when assigning a numerical constant like this:
//  UInt32 v = -1;
// Since an enum is just like a constant, this case also comes up when assigning an enum value to an unsigned variable, like this:
//  enum {EVALUE_One = -1};
//  UInt32 v = EVALUE_One;
// (In portable C/C++, an enum is always promoted to a signed int before assigning or comparing to another integral type.)
// It's safe to suppress this warning, because the input value is never changed. It's just a bit-wise assignment.
#pragma warning (disable: 4245) // 'initializing' : conversion from 'int' to 'unsigned ...', signed/unsigned mismatch
// The compiler would issue an error if the constant value would overflow the target variable.

// warning 4389 detects the case of equating signed and unsigned ints (of the same size). That comes up when comparing a bit field with a bool.
// It's safe to ignore this warning, because the comparison does not entail changing (e.g., sign-extending) either value.
#pragma warning (disable: 4389)  // '==' and '!=': signed/unsigned mismatch
// NB: It's NOT safe to ignore warning 4018. That warning detects the case of comparing signed vs. unsigned for less than or greater than. That's very dangerous!
// NB: It's NOT safe to ignore ANY warning pertaining to signed vs. unsigned ints of different sizes!

// MSVC extensions that we allow because they catch coding errors. We use macros to make these platform-specific
// -----------------------------------------------------------------------------------------------------------------------
#pragma warning(disable:4201) // nonstandard extension used: nameless struct -- GCC also supports this extension

// Use the UNREACHABLE_CODE macro to mark code that you know is unreachable. This may be necessary when adding a return statement to avoid a compiler warning.
#if !defined (BENTLEY_WARNINGS_HIGHEST_LEVEL)
#define UNREACHABLE_CODE(stmt)  stmt
#else
#define UNREACHABLE_CODE(stmt)
#endif

#endif

/// @endcond BENTLEY_SDK_Internal
