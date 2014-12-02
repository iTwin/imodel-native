/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DisplayHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DisplayHandler::GetRangeCenter (ElementHandleCR elHandle, DPoint3dR origin)
    {
    DRange3dCR range = *elHandle.GetIndexRange();
    origin.Interpolate (range.low, 0.5, range.high);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2dP DisplayHandler::GetDPRange (DPoint2dP range, DRange3dCP elRange)
    {
    range[0].x = (double) elRange->low.x;
    range[0].y = (double) elRange->low.y;
    range[1].x = (double) elRange->high.x;
    range[1].y = (double) elRange->high.y;
    return range;
    }

