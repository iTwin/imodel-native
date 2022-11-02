/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Rules/PresentationRulesTypes.h>
#include "../../Helpers/TestHelpers.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRulesTests : ECPresentationTest
    {
    static Utf8String ToPrettyString(BeXmlDomCR xml)
        {
        static uint64_t options = BeXmlDom::ToStringOption::TO_STRING_OPTION_Formatted
            | BeXmlDom::ToStringOption::TO_STRING_OPTION_Indent
            | BeXmlDom::ToStringOption::TO_STRING_OPTION_OmitXmlDeclaration;
        Utf8String str;
        xml.ToString(str, (BeXmlDom::ToStringOption)options);
        return str;
        }
    static Utf8String ToPrettyString(JsonValueCR json)
        {
        return json.toStyledString();
        }
    };

END_ECPRESENTATIONTESTS_NAMESPACE
