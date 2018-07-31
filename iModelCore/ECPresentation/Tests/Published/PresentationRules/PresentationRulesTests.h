/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/PresentationRulesTests.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h>
#include "../../NonPublished/RulesEngine/TestHelpers.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
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