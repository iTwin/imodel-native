/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ISelectProvider.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ISelectProvider.h>
#include <WebServices/Cache/Util/ECExpressionHelper.h>
#include <WebServices/Cache/Util/ECCustomAttributeHelper.h>

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ISelectProvider::SelectProperties> ISelectProvider::GetSelectProperties(ECClassCR ecClass) const
    {
    return std::make_shared<SelectProperties>();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int ISelectProvider::GetSortPriority(ECClassCR ecClass) const
    {
    return 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ISelectProvider::SortProperties ISelectProvider::GetSortProperties(ECClassCR ecClass) const
    {
    return SortProperties();
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ISelectProvider::SelectProperties::SelectProperties() :
m_selectAll(true),
m_selectInstanceId(true)
    {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ISelectProvider::SelectProperties::AddProperty(ECPropertyCP ecProperty)
    {
    m_selectAll = false;

    if (nullptr == ecProperty ||
        std::find(m_ecProperties.begin(), m_ecProperties.end(), ecProperty) != m_ecProperties.end())
        {
        return;
        }

    m_ecProperties.push_back(ecProperty);

    // Add properties that are required for calculating this property value
    // Required for caching property into ECDb as it re-calculates such value
    Utf8String ecExpression = ECCustomAttributeHelper::GetString(ecProperty, "CalculatedECPropertySpecification", "ECExpression");
    if (!ecExpression.empty())
        {
        for (auto requiredProperty : ECExpressionHelper::GetRequiredProperties(ecExpression, ecProperty->GetClass()))
            {
            AddProperty(requiredProperty);
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ISelectProvider::SelectProperties::AddExtendedProperty(Utf8StringCR extendedProperty)
    {
    if (extendedProperty.empty() ||
        std::find(m_extendedProperties.begin(), m_extendedProperties.end(), extendedProperty) != m_extendedProperties.end())
        {
        return;
        }

    m_extendedProperties.push_back(extendedProperty);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<ECPropertyCP>& ISelectProvider::SelectProperties::GetProperties() const
    {
    return m_ecProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& ISelectProvider::SelectProperties::GetExtendedProperties() const
    {
    return m_extendedProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ISelectProvider::SelectProperties::GetSelectAll() const
    {
    return m_selectAll;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ISelectProvider::SelectProperties::GetSelectInstanceId() const
    {
    return m_selectInstanceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ISelectProvider::SelectProperties::SetSelectAll(bool selectAll)
    {
    if (selectAll)
        {
        m_ecProperties.clear();
        }
    m_selectAll = selectAll;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ISelectProvider::SelectProperties::SetSelectInstanceId(bool selectInstanceId)
    {
    m_selectInstanceId = selectInstanceId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ISelectProvider::SortProperty::SortProperty(ECPropertyCR ecProperty, bool ascending) :
m_ecProperty(&ecProperty),
m_ascending(ascending)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCR ISelectProvider::SortProperty::GetProperty() const
    {
    return *m_ecProperty;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ISelectProvider::SortProperty::GetSortAscending() const
    {
    return m_ascending;
    }
