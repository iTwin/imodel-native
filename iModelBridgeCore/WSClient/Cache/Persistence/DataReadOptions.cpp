/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Persistence/DataReadOptions.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Persistence/DataReadOptions.h>

#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/Util/ECDbHelper.h>
#include <WebServices/Cache/Util/ECCustomAttributeHelper.h>
#include <WebServices/Cache/Util/ECExpressionHelper.h>

#include "../Logging.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions::DataReadOptions() :
m_selectAll(true)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsClassKeyValid(Utf8StringCR classKey)
    {
    if (classKey.empty())
        {
        return false;
        }
    if (Utf8String::npos == classKey.find('.'))
        {
        return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DataReadOptions::SelectedClass>::iterator DataReadOptions::EnsureClassIsSelected(Utf8StringCR classKey)
    {
    if (!IsClassKeyValid(classKey))
        {
        BeAssert(false);
        return m_selected.end();
        }

    bvector<SelectedClass>::iterator found = m_selected.end();
    for (auto it = m_selected.begin(); it != m_selected.end(); ++it)
        {
        if (it->GetClassKey().Equals(classKey))
            {
            found = it;
            break;
            }
        }

    if (found == m_selected.end())
        {
        m_selected.push_back(classKey);
        found = m_selected.end();
        found--;
        }

    m_selectAll = false;
    return found;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectAllClasses()
    {
    m_selectAll = true;
    m_selected.clear();
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClass(Utf8StringCR classKey)
    {
    EnsureClassIsSelected(classKey);
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClass(ECClassCR ecClass)
    {
    SelectClass(ECDbHelper::ECClassKeyFromClass(ecClass));
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataReadOptions::AreAllClassesSelected() const
    {
    return m_selectAll;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<DataReadOptions::SelectedClass>& DataReadOptions::GetSelected() const
    {
    return m_selected;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<DataReadOptions::OrderedClass>& DataReadOptions::GetOrderBy() const
    {
    return m_orderBy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClassWithAllProperties(Utf8StringCR classKey)
    {
    auto it = EnsureClassIsSelected(classKey);
    if (it != m_selected.end())
        {
        it->SelectAllProperties();
        }
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClassWithAllProperties(ECClassCR ecClass)
    {
    SelectClassWithAllProperties(ECDbHelper::ECClassKeyFromClass(ecClass));
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClassWithNoProperties(Utf8StringCR classKey)
    {
    auto it = EnsureClassIsSelected(classKey);
    if (it != m_selected.end())
        {
        it->SelectNoProperties();
        }
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClassWithNoProperties(ECClassCR ecClass)
    {
    SelectClassWithNoProperties(ECDbHelper::ECClassKeyFromClass(ecClass));
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClassAndProperty(Utf8StringCR classKey, Utf8StringCR propertyName)
    {
    auto it = EnsureClassIsSelected(classKey);
    if (!propertyName.empty() && it != m_selected.end())
        {
        it->SelectProperty(propertyName);
        }
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::SelectClassAndProperty(ECPropertyCR ecProperty)
    {
    SelectClassAndProperty(ECDbHelper::ECClassKeyFromClass(ecProperty.GetClass()), Utf8String(ecProperty.GetName()));
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::AddOrderByClassAndProperties(Utf8StringCR classKey, const bvector<OrderedProperty>& properties)
    {
    if (IsClassKeyValid(classKey))
        {
        m_orderBy.push_back(OrderedClass(classKey, properties));
        }
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions& DataReadOptions::AddOrderByClassAndProperties(ECClassCR ecClass, const bvector<OrderedProperty>& properties)
    {
    AddOrderByClassAndProperties(ECDbHelper::ECClassKeyFromClass(ecClass), properties);
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<DataReadOptions::SelectProperties> DataReadOptions::GetSelectProperties(ECClassCR ecClass) const
    {
    auto selectedClass = GetSelectedClass(ecClass);
    if (nullptr != selectedClass)
        {
        auto selectProperties = std::make_shared<DataReadOptions::SelectProperties>();
        selectProperties->SetSelectAll(selectedClass->AreAllPropertiesSelected());
        selectProperties->SetSelectInstanceId(true);

        for (Utf8StringCR selectedProperty : selectedClass->GetSelectedProperties())
            {
            ECPropertyCP ecProperty = ecClass.GetPropertyP(selectedProperty.c_str());
            if (nullptr == ecProperty)
                {
                selectProperties->AddExtendedProperty(selectedProperty);
                }
            else
                {
                selectProperties->AddProperty(ecProperty);
                }
            }
        return selectProperties;
        }

    if (AreAllClassesSelected())
        {
        return std::make_shared<DataReadOptions::SelectProperties>();;
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int DataReadOptions::GetSortPriority(ECClassCR ecClass) const
    {
    Utf8String classKey = ECDbHelper::ECClassKeyFromClass(ecClass);
    auto it = std::find_if(m_orderBy.begin(), m_orderBy.end(), [&] (const OrderedClass& orderedClass)
        {
        return orderedClass.GetClassKey().Equals(classKey);
        });

    if (it == m_orderBy.end())
        {
        return 0;
        }

    int priority = 0 - (int) (it - m_orderBy.begin());
    return priority;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions::SortProperties DataReadOptions::GetSortProperties(ECClassCR ecClass) const
    {
    Utf8String classKey = ECDbHelper::ECClassKeyFromClass(ecClass);
    auto it = std::find_if(m_orderBy.begin(), m_orderBy.end(), [&] (const OrderedClass& orderedClass)
        {
        return orderedClass.GetClassKey().Equals(classKey);
        });

    DataReadOptions::SortProperties sortProperties;
    if (it == m_orderBy.end())
        {
        return sortProperties;
        }

    for (const OrderedProperty& orderedProperty : it->GetProperties())
        {
        ECPropertyCP ecProperty = ecClass.GetPropertyP(orderedProperty.GetName().c_str());
        if (nullptr == ecProperty)
            {
            continue;
            }

        sortProperties.push_back(SortProperty(*ecProperty, orderedProperty.IsOrderAscending()));
        }

    return sortProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const DataReadOptions::SelectedClass* DataReadOptions::GetSelectedClass(Utf8StringCR classKey) const
    {
    if (m_selectAll)
        {
        return nullptr;
        }
    for (const SelectedClass& selectedClass : m_selected)
        {
        if (selectedClass.GetClassKey().Equals(classKey))
            {
            return &selectedClass;
            }
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const DataReadOptions::SelectedClass* DataReadOptions::GetSelectedClass(ECClassCR ecClass) const
    {
    return GetSelectedClass(ECDbHelper::ECClassKeyFromClass(ecClass));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions::SelectedClass::SelectedClass(Utf8StringCR classKey)
: m_classKey(classKey), m_allPropertiesSelected(true)
    {
    BeAssert(IsClassKeyValid(classKey));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DataReadOptions::SelectedClass::SelectProperty(Utf8StringCR propertyName)
    {
    m_allPropertiesSelected = false;

    if (propertyName.empty())
        {
        BeAssert(false);
        return;
        }

    auto found = std::find(m_orderedSelectedProperties.begin(), m_orderedSelectedProperties.end(), propertyName);
    if (m_orderedSelectedProperties.end() == found)
        {
        m_orderedSelectedProperties.push_back(propertyName);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DataReadOptions::SelectedClass::SelectAllProperties()
    {
    m_orderedSelectedProperties.clear();
    m_allPropertiesSelected = true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DataReadOptions::SelectedClass::SelectNoProperties()
    {
    m_orderedSelectedProperties.clear();
    m_allPropertiesSelected = false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataReadOptions::SelectedClass::AreAllPropertiesSelected() const
    {
    return m_allPropertiesSelected;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DataReadOptions::SelectedClass::GetClassKey() const
    {
    return m_classKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<Utf8String>& DataReadOptions::SelectedClass::GetSelectedProperties() const
    {
    return m_orderedSelectedProperties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions::OrderedClass::OrderedClass(Utf8StringCR classKey, const bvector<OrderedProperty>& properties)
: m_classKey(classKey), m_properties(properties)
    {
    BeAssert(IsClassKeyValid(classKey));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void DataReadOptions::OrderedClass::AddOrderedProperty(const OrderedProperty& property)
    {
    m_properties.push_back(property);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DataReadOptions::OrderedClass::GetClassKey() const
    {
    return m_classKey;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const bvector<DataReadOptions::OrderedProperty>& DataReadOptions::OrderedClass::GetProperties() const
    {
    return m_properties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DataReadOptions::OrderedProperty::OrderedProperty(Utf8StringCR propertyName, bool ascending)
: m_propertyName(propertyName), m_isAscending(ascending)
    {
    BeAssert(!propertyName.empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DataReadOptions::OrderedProperty::GetName() const
    {
    return m_propertyName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataReadOptions::OrderedProperty::IsOrderAscending() const
    {
    return m_isAscending;
    }
