/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Converters/abdfilechecker.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// comment out next line if something is wrong with abdfilechecker.cpp
#define USEABDFILECHECKER

#ifdef USEABDFILECHECKER
bool dgnFile_isABDFile(DgnFilePtr file);
#endif