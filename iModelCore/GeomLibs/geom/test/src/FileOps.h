/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeFile.h>

struct GTestFileOps
{
static bool ReadAsString (BeFileName &filename, Utf8String &string);
static bool WriteToFile(Utf8String &string, WCharCP nameA, WCharCP nameB, WCharCP nameC, WCharCP extension);
};