/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#define POLYCOFF_MAX_COFFS 9
typedef struct
    {
    double coff[POLYCOFF_MAX_COFFS];
    int numCoff;
    } PolyCoffs;


