/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include "PropertyMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//! Computes the JS-cased member name for a single EC property, mirroring the special-casing done for system
//! properties (e.g. ECInstanceId -> "id") when rendering column values to JSON (see ECSqlRowAdaptor::RenderRow).
Utf8String GetJsMemberName(ECN::ECPropertyCR ecProperty);

END_BENTLEY_SQLITE_EC_NAMESPACE
