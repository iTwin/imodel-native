/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//#define EC_TRACE_MEMORY

#include <string>
#include <vector>
#include <map>
#include <limits>

#include <ECObjects/ECEnabler.h>
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECUnit.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECName.h>
#include <ECObjects/ECSchemaConverter.h>
#include <ECObjects/ECSchemaDownConverter.h>
#include <ECObjects/ECSchemaValidator.h>
#include <ECObjects/SupplementalSchema.h>
#include <ECObjects/ECContext.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECExpressions.h>
#include <ECObjects/ECDBuffer.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/StandaloneECRelationshipInstance.h>
#include <ECObjects/PresentationMetadataHelper.h>
#include <ECObjects/StandardCustomAttributeHelper.h>
#include <Bentley/ScopedArray.h>
#include <ECObjects/ECJsonUtilities.h>
#include <ECObjects/ECRelationshipPath.h>
#include <ECObjects/IssueReporter.h>
#include <ECObjects/DesignByContract.h>
