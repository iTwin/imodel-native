/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifdef BENTLEY_WIN32
    #define PROFILEBREAK(message) wprintf (message); wprintf (L"\n"); getchar ();
#else
    #define PROFILEBREAK(...)
#endif
