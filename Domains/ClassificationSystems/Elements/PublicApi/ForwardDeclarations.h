/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <ClassificationSystems/Domain/ClassificationSystemsMacros.h>

//---------------------------------------------------------------------------------------
// Forward declarations for element refcounted typedefs
//---------------------------------------------------------------------------------------
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(Classification)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationGroup)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationSystem)
CLASSIFICATIONSYSTEMS_REFCOUNTED_PTR_AND_TYPEDEFS(ClassificationTable)

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

//---------------------------------------------------------------------------------------
// Forward declarations for elements
//---------------------------------------------------------------------------------------
struct Classification;
struct ClassificationGroup;
struct ClassificationSystem;
struct ClassificationTable;

END_CLASSIFICATIONSYSTEMS_NAMESPACE