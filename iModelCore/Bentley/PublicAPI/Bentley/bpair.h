/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>

BEGIN_BENTLEY_NAMESPACE

template <typename T1, typename T2> using bpair = std::pair<T1,T2>;

template <typename T1, typename T2> auto make_bpair(T1 t1, T2 t2) { return std::make_pair(t1,t2); }

END_BENTLEY_NAMESPACE
