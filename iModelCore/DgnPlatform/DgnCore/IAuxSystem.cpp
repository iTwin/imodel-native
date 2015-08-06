/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/IAuxSystem.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGN

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        IAuxCoordSys::_GetTransparency
(
bool                isFill,
ACSDisplayOptions   options
) const
    {
    uint32_t    transparency;

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
uint32_t        axis,
WCharP        axisLabel,
uint32_t        length
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
