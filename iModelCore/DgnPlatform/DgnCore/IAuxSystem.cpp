/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/IAuxSystem.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          IAuxCoordSys::_GetTransparency
(
bool                isFill,
ACSDisplayOptions   options
) const
    {
    UInt32      transparency;

    if (isFill)
        transparency = (ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized) ? 215 : 150);
    else
        transparency = (ACSDisplayOptions::None != (options & ACSDisplayOptions::Deemphasized) ? 125 : 50);

    return transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/07
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP       IAuxCoordSys::_GetAxisLabel
(
UInt32          axis,
WCharP        axisLabel,
UInt32          length
) const
    {
    switch (axis)
        {
        case 0:
            wcscpy (axisLabel, L"X");
            break;
        case 1:
            wcscpy (axisLabel, L"Y");
            break;
        case 2:
            wcscpy (axisLabel, L"Z");
            break;
        default:
            *axisLabel = 0;
            break;
        }

    return axisLabel;
    }
