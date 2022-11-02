/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#define POLYCOFF_MAX_COFFS 9
typedef struct
    {
    double coff[POLYCOFF_MAX_COFFS];
    int numCoff;
    } PolyCoffs;


