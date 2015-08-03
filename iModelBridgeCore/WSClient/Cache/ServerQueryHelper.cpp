/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/ServerQueryHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/ServerQueryHelper.h>

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Cache/Util/ECCustomAttributeHelper.h>
#include <WebServices/Cache/Util/ECExpressionHelper.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ServerQueryHelper::ServerQueryHelper (const ISelectProvider& selectProvider) :
m_selectProvider (selectProvider)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bset<ECPropertyCP> ServerQueryHelper::GetRequiredProperties (ECPropertyCP ecProperty)
    {
    bset<ECPropertyCP> properties;
    if (nullptr == ecProperty)
        {
        return properties;
        }
    Utf8String ecExpression = ECCustomAttributeHelper::GetString (ecProperty, L"CalculatedECPropertySpecification", L"ECExpression");
    if (ecExpression.empty ())
        {
        return properties;
        }

    return ECExpressionHelper::GetRequiredProperties (ecExpression, ecProperty->GetClass ());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> ServerQueryHelper::GetAllSelectedProperties (const ECSchemaList& ecSchemas) const
    {
    bset<Utf8String> properties;

    for (ECSchemaCP ecSchema : ecSchemas)
        {
        for (ECClassCP ecClass : ecSchema->GetClasses ())
            {
            auto selectProperties = m_selectProvider.GetSelectProperties (*ecClass);
            if (nullptr == selectProperties)
                {
                continue;
                }

            for (ECPropertyCP ecProperty : selectProperties->GetProperties ())
                {
                auto relatedProperties = GetRequiredProperties (ecProperty);

                for (ECPropertyCP ecProperty : relatedProperties)
                    {
                    properties.insert (Utf8String (ecProperty->GetName ()));
                    }

                properties.insert (Utf8String (ecProperty->GetName ()));
                }
            }
        }

    return properties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ServerQueryHelper::GetSelect (ECClassCR ecClass) const
    {
    auto selectProperties = m_selectProvider.GetSelectProperties (ecClass);
    if (nullptr == selectProperties)
        {
        return "$id";
        }

    if (!selectProperties->GetSelectAll () && selectProperties->GetProperties ().empty ())
        {
        return "$id";
        }

    bset<ECPropertyCP> allSelectedProperties (selectProperties->GetProperties ().begin (), selectProperties->GetProperties ().end ());
    for (ECPropertyCP ecProperty : selectProperties->GetProperties ())
        {
        auto requiredProperties = GetRequiredProperties (ecProperty);
        allSelectedProperties.insert (requiredProperties.begin (), requiredProperties.end ());
        }

    Utf8String select;
    for (ECPropertyCP ecProperty : allSelectedProperties)
        {
        if (!select.empty ())
            {
            select += ',';
            }
        select += Utf8String (ecProperty->GetName ());
        }
    return select;
    }
