/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <vector>

BEGIN_BENTLEY_NAMESPACE

template <typename T, class Alloc = std::allocator<T>> using bvector = std::vector<T, Alloc>;

END_BENTLEY_NAMESPACE
