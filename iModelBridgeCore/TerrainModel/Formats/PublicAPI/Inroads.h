/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Core/DTMDefs.h>

BENTLEYDTMFORMATS_EXPORT DTMStatusInt bcdtmFormatInroads_exportBclibDtmToInroadsDtmFile (BC_DTM_OBJ *dtmP, const wchar_t *dtmFileNameP, const wchar_t *nameP, const wchar_t *descriptionP);
