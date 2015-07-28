/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Persistence/DataReadOptions.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/ISelectProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
* Helper class for defining select/read options for class instances.
* Does not support polymorphism. Use ISelectProvider for polymorphic select.
+---------------+---------------+---------------+---------------+---------------+------*/
typedef struct DataReadOptions& DataReadOptionsR;
typedef const struct DataReadOptions& DataReadOptionsCR;
typedef std::shared_ptr<DataReadOptions> DataReadOptionsPtr;
typedef std::shared_ptr<const DataReadOptions> DataReadOptionsCPtr;

struct EXPORT_VTABLE_ATTRIBUTE DataReadOptions : public ISelectProvider
    {
    public:
        struct SelectedClass;
        struct OrderedClass;
        struct OrderedProperty;

    private:
        bool                    m_selectAll;
        bvector<SelectedClass>  m_selected;
        bvector<OrderedClass>   m_orderBy;

    private:
        bvector<SelectedClass>::iterator EnsureClassIsSelected (Utf8StringCR classKey);

    public:
        //! Create empty options to read all data
        WSCACHE_EXPORT DataReadOptions ();
        //! Select all classes and clear any previous selects
        WSCACHE_EXPORT DataReadOptions& SelectAllClasses ();
        //! Explicitly add class to select and disable selecting all classes
        WSCACHE_EXPORT DataReadOptions& SelectClass (Utf8StringCR classKey);
        //! Helper overload for ecClass
        WSCACHE_EXPORT DataReadOptions& SelectClass (ECClassCR ecClass);
        //! Explicitly add class and all properties for it to select
        WSCACHE_EXPORT DataReadOptions& SelectClassWithAllProperties (Utf8StringCR classKey);
        //! Helper overload for ecClass
        WSCACHE_EXPORT DataReadOptions& SelectClassWithAllProperties (ECClassCR ecClass);
        //! Explicitly add class and no properties to select
        WSCACHE_EXPORT DataReadOptions& SelectClassWithNoProperties (Utf8StringCR classKey);
        //! Helper overload for ecClass
        WSCACHE_EXPORT DataReadOptions& SelectClassWithNoProperties (ECClassCR ecClass);
        //! Explicitly add class and its property to select
        WSCACHE_EXPORT DataReadOptions& SelectClassAndProperty (Utf8StringCR classKey, Utf8StringCR propertyName);
        //! Helper overload for ecProperty. Class will be taken from property
        WSCACHE_EXPORT DataReadOptions& SelectClassAndProperty (ECPropertyCR ecProperty);
        //! Add ordering information for queries
        WSCACHE_EXPORT DataReadOptions& AddOrderByClassAndProperties (Utf8StringCR classKey, const bvector<OrderedProperty>& properties);
        //! Helper overload for ecClass
        WSCACHE_EXPORT DataReadOptions& AddOrderByClassAndProperties (ECClassCR ecClass, const bvector<OrderedProperty>& properties);

        WSCACHE_EXPORT bool AreAllClassesSelected () const;
        WSCACHE_EXPORT const SelectedClass* GetSelectedClass (Utf8StringCR classKey) const;
        WSCACHE_EXPORT const SelectedClass* GetSelectedClass (ECClassCR ecClass) const;
        WSCACHE_EXPORT const bvector<SelectedClass>& GetSelected () const;

        //! Returns ordered classes and properties
        WSCACHE_EXPORT const bvector<OrderedClass>& GetOrderBy () const;

        WSCACHE_EXPORT std::shared_ptr<SelectProperties> GetSelectProperties (ECClassCR ecClass) const override;
        WSCACHE_EXPORT int GetSortPriority (ECClassCR ecClass) const override;
        WSCACHE_EXPORT bvector<SortProperty> GetSortProperties (ECClassCR ecClass) const override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataReadOptions::SelectedClass
    {
    private:
        Utf8String          m_classKey;
        bvector<Utf8String> m_orderedSelectedProperties;
        bool                m_allPropertiesSelected;

    public:
        WSCACHE_EXPORT SelectedClass (Utf8StringCR classKey);

        WSCACHE_EXPORT void SelectProperty (Utf8StringCR propertyName);
        WSCACHE_EXPORT void SelectAllProperties ();
        WSCACHE_EXPORT void SelectNoProperties ();

        WSCACHE_EXPORT bool AreAllPropertiesSelected () const;
        WSCACHE_EXPORT Utf8StringCR GetClassKey () const;
        WSCACHE_EXPORT const bvector<Utf8String>& GetSelectedProperties () const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataReadOptions::OrderedClass
    {
    private:
        Utf8String                  m_classKey;
        bvector<OrderedProperty>    m_properties;

    public:
        WSCACHE_EXPORT OrderedClass (Utf8StringCR classKey, const bvector<OrderedProperty>& properties = bvector<OrderedProperty> ());

        WSCACHE_EXPORT void AddOrderedProperty (const OrderedProperty& property);

        WSCACHE_EXPORT Utf8StringCR GetClassKey () const;
        WSCACHE_EXPORT const bvector<OrderedProperty>& GetProperties () const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataReadOptions::OrderedProperty
    {
    private:
        Utf8String  m_propertyName;
        bool        m_isAscending;

    public:
        WSCACHE_EXPORT OrderedProperty (Utf8StringCR propertyName, bool ascending = true);

        WSCACHE_EXPORT Utf8StringCR GetName () const;
        WSCACHE_EXPORT bool IsOrderAscending () const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
