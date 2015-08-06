/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/MdlCnv.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGN_NAMESPACE

struct DataConvert
{
public:
DGNPLATFORM_EXPORT static void IPointToDPoint (DPoint3dR dPointP, Point3dCR iPointP);
DGNPLATFORM_EXPORT static long RoundDoubleToLong (double inDouble);
DGNPLATFORM_EXPORT static unsigned long RoundDoubleToULong (double inDouble);

DGNPLATFORM_EXPORT static void Points3dTo2d (DPoint2dP outP, DPoint3dCP inP, int numPts);
DGNPLATFORM_EXPORT static void Points2dTo3d (DPoint3dP outP, DPoint2dCP inP, int numPts, double zElev);
DGNPLATFORM_EXPORT static void RotationToQuaternion (double* quaternion, double rotationAngle);

DGNPLATFORM_EXPORT static void SwapWord (uint32_t* input);
DGNPLATFORM_EXPORT static void SwapWord (  int32_t* input);

DGNPLATFORM_EXPORT static void SwapWordArray (void* pntr, size_t numpoints);

DGNPLATFORM_EXPORT static void DoubleFromFileFormatArray (double* pntr, size_t numdoubles);
DGNPLATFORM_EXPORT static void DoubleToFileFormatArray (double* pntr, size_t numdoubles);

DGNPLATFORM_EXPORT static void ReverseUInt64 (uint64_t&, uint64_t);

DGNPLATFORM_EXPORT static long FromScanFormat(unsigned long usInput);
DGNPLATFORM_EXPORT static unsigned long ToScanFormat(long sinput);
};

END_BENTLEY_DGN_NAMESPACE


