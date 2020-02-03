/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

template<typename T> void FreeItemMemory(T* item);

template<typename T> inline size_t GetSizeInMemory(T* item);
