/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Mstn/MdlApi/rtypes.r.h>
#include <Mstn/MdlApi/rscdefs.r.h>

enum TerrainModelAppMessageLists
    {
    TerrainModelAppMessageList_Default = 1,
    };


enum TerrainModelAppMessages
    {
    MESSAGE_HandleMoveStart = 1,
    MESSAGE_FromParent,
    };
