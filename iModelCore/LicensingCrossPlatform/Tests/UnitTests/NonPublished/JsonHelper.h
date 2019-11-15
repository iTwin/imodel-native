/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include <json/value.h>

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

Json::Value ReadJsonFile(BeFileNameCR filename);

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE