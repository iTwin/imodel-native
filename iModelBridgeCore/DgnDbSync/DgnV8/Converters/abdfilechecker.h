/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

// comment out next line if something is wrong with abdfilechecker.cpp
//#define USEABDFILECHECKER

#ifdef USEABDFILECHECKER
bool dgnFile_isABDFile(DgnFilePtr file);
#endif