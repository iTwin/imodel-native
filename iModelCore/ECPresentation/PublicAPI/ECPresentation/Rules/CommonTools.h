/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRulesTypes.h>
#include <ECPresentation/Rules/PresentationRule.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*---------------------------------------------------------------------------------**//**
Helper class for commonly used functions
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CommonTools
{
private:
    CommonTools() {}

public:
    //! Converts a given integer to base36
    ECPRESENTATION_EXPORT static Utf8String ToBase36String(uint64_t i);

    //! Extracts briefcase ID from the given ECInstance ID
    ECPRESENTATION_EXPORT static uint64_t GetBriefcaseId(ECInstanceId id);

    //! Extracts local ID from the given ECInstance ID
    ECPRESENTATION_EXPORT static uint64_t GetLocalId(ECInstanceId id);

    //! Adds an element to the specified list sorted by elements priority
    template<typename ElementType, typename ListType> static void AddToListByPriority(ListType& list, ElementType& element)
        {
        auto iter = list.rbegin();
        for (; iter != list.rend(); iter++)
            {
            if ((*iter)->GetPriority() >= element.GetPriority())
                break;
            }
        list.insert(iter.base(), &element);
        }

    //! Removes an element from the specified list
    template<typename ElementType, typename ListType> static void RemoveFromList(ListType& list, ElementType& element)
        {
        auto iter = list.begin();
        for (; iter != list.end(); iter++)
            {
            if (*iter == &element)
                break;
            }
        if (list.end() != iter)
            list.erase(iter);
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
