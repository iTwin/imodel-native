/*---------------------------------------------------------------------------------------------
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|  See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

// This exists so that Mike's special audit tool can easily identify the underlying openssl version of this library.

#ifdef _MSC_VER

// Under MSVC, this adds the string as a new PE section that DumpBin can easily isolate.
// This supposedly originated in a 'VersionSegment.h' file, but I don't see it...
#pragma const_seg("BSIVer")
#pragma const_seg()
static __declspec (allocate("BSIVer")) char szSourceFileVersionString[] = "#@!~BeOpenSSL 1.1.1s; OpenSSL 1.1.1s, 1.1.1s~!@#";

#endif
