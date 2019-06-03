/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRule.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Helper class for commonly used functions
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CommonTools
{
private:
    CommonTools() {}

public:    
    //! Combines class name with element Id of Base36 format.
    ECPRESENTATION_EXPORT static Utf8String GetDefaultDisplayLabel(Utf8StringCR className, uint64_t id);

    //! Combines class name with element Id of Base36 format.
    ECPRESENTATION_EXPORT static Utf8String GetDefaultDisplayLabel(ECN::IECInstanceCR);

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
