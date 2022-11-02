/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/SchemaConflictHelper.h>
#include <numeric>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaConflictHelper::IsSameKindOfProperty(ECPropertyCR p1, ECPropertyCR p2)
    {
    return (p1.GetIsPrimitiveArray() == p2.GetIsPrimitiveArray()) &&
       (p1.GetIsPrimitive() == p2.GetIsPrimitive()) &&
       (p1.GetIsStructArray() == p2.GetIsStructArray()) &&
       (p1.GetIsStruct() == p2.GetIsStruct()) &&
       (p1.GetIsNavigation() == p2.GetIsNavigation());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaConflictHelper::ArePropertiesCompatible(ECPropertyCR p1, ECPropertyCR p2)
    {
    if(!IsSameKindOfProperty(p1, p2))
        return false;

    Utf8String errMsg; //TODO: Do something with the error message? It's not really an "error" in this context as we are only checking
    if (!p2._CanOverride(p1, errMsg))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaConflictHelper::IsPropertyCompatibleWithDerivedClasses(ECClassCR ecClass, ECPropertyCR propertyToAdd)
    {
    if(!ecClass.HasDerivedClasses())
        return true;

    for (ECN::ECClassP derivedClass : ecClass.GetDerivedClasses())
        {
        ECPropertyCP propertyOverride = derivedClass->GetPropertyP(propertyToAdd.GetName().c_str(), false);
        if(propertyOverride != nullptr && !ArePropertiesCompatible(propertyToAdd, *propertyOverride))
            return false;

        if(!IsPropertyCompatibleWithDerivedClasses(*derivedClass, propertyToAdd))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaConflictHelper::CanPropertyBeAdded(ECClassCR target, ECPropertyCR propertyToAdd)
    {
    ECPropertyCP existingProperty = target.GetPropertyP(propertyToAdd.GetName().c_str(), true);
    if(existingProperty != nullptr && !ArePropertiesCompatible(*existingProperty, propertyToAdd))
        return false;

    //we only need to run this check if existingProperty is nullptr, because if the property already exists on base,
    //we can be sure the derived classes don't conflict with it.
    if(existingProperty == nullptr && !IsPropertyCompatibleWithDerivedClasses(target, propertyToAdd))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaConflictHelper::CanBaseClassBeAdded(ECClassCR target, ECClassCR baseClassToAdd)
    {
    for (const auto property : baseClassToAdd.GetProperties(true))
        {
        if(!CanPropertyBeAdded(target, *property))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaConflictHelper::PropertyExistsInHierarchy(ECClassCR ecClass, Utf8CP propertyName)
    {
    if(ecClass.GetPropertyP(propertyName, true) != nullptr)
        return true;

    std::function<bool(ECClassCR node)> propertyExistsInDerivedClasses; //declaration first necessary, so we can recurse into it
    propertyExistsInDerivedClasses = [&propertyName, &propertyExistsInDerivedClasses](ECClassCR node)
        {
        if(!node.HasDerivedClasses())
            return false;

        for (ECN::ECClassP derivedClass : node.GetDerivedClasses())
            {
            if(derivedClass->GetPropertyP(propertyName, false) != nullptr)
                return true;

            if(propertyExistsInDerivedClasses(*derivedClass))
                return true;
            }

        return false;
        };

    return propertyExistsInDerivedClasses(ecClass);
    }

static std::regex const nameRegex("^(.+)_(\\d{0,4})$", std::regex::optimize);

/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SchemaConflictHelper::FindUniquePropertyName(ECClassCR target, Utf8StringR propertyName)
    {
    if(!PropertyExistsInHierarchy(target, propertyName.c_str()))
        return BentleyStatus::SUCCESS;

    Utf8String rawPropertyName(propertyName); // "raw" means without any suffix

    uint32_t appendedNumber = 1;

    std::smatch match;
    std::string str(propertyName);

    if (std::regex_search(str, match, nameRegex) && match.size() >= 3)
        {
        rawPropertyName = match.str(1).c_str();
        std::string numberStr = match.str(2);
        if(!numberStr.empty())
            appendedNumber = std::stoul(match.str(2)) + 1;
        }

    for(int i = 0; i < 50; i++) //only 50 attempts, if that doesn't work, we cancel.
        {
        Utf8PrintfString candidate("%s_%d", rawPropertyName.c_str(), appendedNumber);
        if(!PropertyExistsInHierarchy(target, candidate.c_str()))
            {
            propertyName = candidate;
            return BentleyStatus::SUCCESS;
            }

        appendedNumber++;
        }

    return BentleyStatus::ERROR;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
