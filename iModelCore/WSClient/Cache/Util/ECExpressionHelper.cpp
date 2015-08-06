/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/ECExpressionHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECExpressionHelper.h>

#include <regex>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECPropertyCP> ECExpressionHelper::GetRequiredProperties(Utf8StringCR ecExpression, ECClassCR ecClass)
    {
    bset<ECPropertyCP> properties;

    // REGEXP approach taken from .NET implementation of EC.
    // Matches .PropertyName
    Utf8String propertyAccessorSubExpression = R"(\s*\.\s*\w+\s*(?!\())";
    // Matches 'this' plus propertyAccessorSubExpression
    Utf8String propertyAccessorExpression = R"(\bthis)" + propertyAccessorSubExpression;

    std::smatch match;
    std::regex e(propertyAccessorExpression.c_str());

    std::string str(ecExpression.c_str());
    while (std::regex_search(str, match, e))
        {
        Utf8String propertyName = match.str(0).substr(5).c_str();
        ECPropertyCP ecProperty = ecClass.GetPropertyP(propertyName.c_str());
        if (nullptr != ecProperty)
            {
            properties.insert(ecProperty);
            }
        str = match.suffix().str();
        }

    return properties;
    }
