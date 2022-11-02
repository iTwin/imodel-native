/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeFile.h>

struct GTestFileOps
{
static bool ReadAsString (BeFileName &filename, Utf8String &string);
static bool WriteToFile(Utf8String &string, WCharCP nameA, WCharCP nameB, WCharCP nameC, WCharCP extension);
static bool WriteByteArrayToTextFile(bvector<Byte> &bytes, WCharCP directoryName, WCharCP nameB, WCharCP nameC, WCharCP extension);
};