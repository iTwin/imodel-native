/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Performance/ProfileBreak.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#ifdef BENTLEY_WIN32
    #define PROFILEBREAK(message) wprintf (message); wprintf (L"\n"); getchar ();
#else
    #define PROFILEBREAK(...)
#endif
