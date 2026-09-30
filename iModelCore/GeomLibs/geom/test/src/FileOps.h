/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Geom/GeomApi.h>
#include <Bentley/BeFileName.h>

struct GTestFileOps
{
static bool ReadAsBytes(BeFileNameCR filename, bvector<Byte> &bytes);
static bool ReadAsString(BeFileNameCR filename, Utf8String &string);
static bool ReadAsString(char const *filenameChar, Utf8String &string);
static bool WriteToFile(bvector<Byte>& bytes, WCharCP directoryName, WCharCP nameB, WCharCP nameC, WCharCP extension);
static bool WriteToFile(Utf8String &string, WCharCP nameA, WCharCP nameB, WCharCP nameC, WCharCP extension);
static bool WriteByteArrayToTextFile(bvector<Byte> &bytes, WCharCP directoryName, WCharCP nameB, WCharCP nameC, WCharCP extension);
static bool FlatBufferFileToGeometry(BeFileNameCR, bvector<IGeometryPtr>&);
static bool JsonFileToGeometry(BeFileNameCR filename, bvector<IGeometryPtr> &geometry);
static bool JsonFileToGeometry(char const *filenameChar, bvector<IGeometryPtr> &geometry);
};