/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#ifdef BENTLEY_WIN32
    #define PROFILEBREAK(message) wprintf (message); wprintf (L"\n"); getchar ();
#else
    #define PROFILEBREAK(...)
#endif
