/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/RefCounted.h>
#include <Bentley/Nullable.h>
#include <Bentley/DateTime.h>
#include <Bentley/ByteStream.h>
#include <ECObjects/ECSchema.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/SchemaComparer.h>
#include <regex>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaConflictHelper final
{
private:
    SchemaConflictHelper() = delete;
    ~SchemaConflictHelper() = delete;
public:

    ECOBJECTS_EXPORT static bool IsSameKindOfProperty(ECPropertyCR p1, ECPropertyCR p2);
    ECOBJECTS_EXPORT static bool ArePropertiesCompatible(ECPropertyCR p1, ECPropertyCR p2);
    ECOBJECTS_EXPORT static bool IsPropertyCompatibleWithDerivedClasses(ECClassCR ecClass, ECPropertyCR propertyToAdd);
    ECOBJECTS_EXPORT static bool CanBaseClassBeAdded(ECClassCR target, ECClassCR baseClassToAdd);
    ECOBJECTS_EXPORT static bool CanPropertyBeAdded(ECClassCR target, ECPropertyCR propertyToAdd);
    ECOBJECTS_EXPORT static bool PropertyExistsInHierarchy(ECClassCR ecClass, Utf8CP propertyName);
    ECOBJECTS_EXPORT static BentleyStatus FindUniquePropertyName(ECClassCR target, Utf8StringR propertyName);

private:
};

END_BENTLEY_ECOBJECT_NAMESPACE
