/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ISelectProvider.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
* Class for providing select and sorting for queries.
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ISelectProvider
    {
    public:
        struct SelectProperties;
        struct SortProperty;
        typedef bvector<SortProperty> SortProperties;

    public:
        virtual ~ISelectProvider()
            {};

        //! Return properties that should be selected for class instances. Return null to not select instances of this class.
        //! Default returns select all.
        WSCACHE_EXPORT virtual std::shared_ptr<SelectProperties> GetSelectProperties(ECClassCR ecClass) const;

        //! Return integer priority to sort instances by class.
        //! Default returns 0.
        WSCACHE_EXPORT virtual int GetSortPriority(ECClassCR ecClass) const;

        //! Return ordered properties that are used to sort class instances.
        //! Default returns no sorting properties.
        WSCACHE_EXPORT virtual SortProperties GetSortProperties(ECClassCR ecClass) const;
    };

typedef const ISelectProvider& ISelectProviderCR;
typedef std::shared_ptr<const ISelectProvider> ISelectProviderPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ISelectProvider::SelectProperties
    {
    private:
        bvector<ECPropertyCP> m_ecProperties;
        bvector<Utf8String> m_extendedProperties;
        bool m_selectAll;
        bool m_selectInstanceId;

    public:
        // Default constructor to select all properties and instance id
        WSCACHE_EXPORT SelectProperties();

        //! Add property to select. Will add required properties if this is ECCalculatedProperty
        WSCACHE_EXPORT void AddProperty(ECPropertyCP ecProperty);
        WSCACHE_EXPORT const bvector<ECPropertyCP>& GetProperties() const;

        //! Add property that is not part of class but is specific to data source
        WSCACHE_EXPORT void AddExtendedProperty(Utf8StringCR extendedProperty);
        WSCACHE_EXPORT const bvector<Utf8String>& GetExtendedProperties() const;

        WSCACHE_EXPORT void SetSelectInstanceId(bool selectInstanceId);
        WSCACHE_EXPORT bool GetSelectInstanceId() const;

        WSCACHE_EXPORT void SetSelectAll(bool selectAll);
        WSCACHE_EXPORT bool GetSelectAll() const;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ISelectProvider::SortProperty
    {
    private:
        ECPropertyCP m_ecProperty;
        bool m_ascending;

    public:
        WSCACHE_EXPORT SortProperty(ECPropertyCR ecProperty, bool ascending = true);
        WSCACHE_EXPORT ECPropertyCR GetProperty() const;
        WSCACHE_EXPORT bool GetSortAscending() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
