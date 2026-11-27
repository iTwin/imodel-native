/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include "PropertyMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! Computes the JS-cased member name for a single EC property, including the special-casing done for system
//! properties (e.g. ECInstanceId -> "id"). @p useClassFullName controls whether ECClassId is named "classFullName"
//! instead of "className". @p useNavigationNames converts navigation ID properties to their JSON member names.
Utf8String GetJsMemberName(ECN::ECPropertyCR ecProperty, bool useClassFullName, bool useNavigationNames = false);

END_BENTLEY_SQLITE_EC_NAMESPACE
