/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/WSQuery.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <WebServices/ObjectId.h>
#include <Bentley/WString.h>
#include <map>
#include <set>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSQuery
    {
    private:
        Utf8String m_mainSchemaName;
        std::set<Utf8String> m_classes;

        Utf8String m_select;
        Utf8String m_filter;
        Utf8String m_orderBy;
        uint32_t m_skip;
        uint32_t m_top;

        uint64_t m_aliasNumber;
        std::map<Utf8String, Utf8String> m_aliases;
        std::map<Utf8String, Utf8String> m_customParameters;

    private:
        Utf8String CreateNewAlias ();
        static void AppendOptionalParameter (Utf8StringR query, Utf8StringCR name, Utf8StringCR value);
        static void AppendOptionalParameter (Utf8StringR query, Utf8StringCR name, uint32_t value);

    public:
        //! Construct query for given classes. If more than one schema is used, prefix schema name to class. E.g.: "SchemaName.ClassName"
        WS_EXPORT WSQuery (Utf8StringCR schemaName, std::set<Utf8String> classes);
        //! Construct query for given class
        WS_EXPORT WSQuery (Utf8StringCR schemaName, Utf8StringCR className);
        //! Construct query for given object
        WS_EXPORT WSQuery (ObjectIdCR objectId);

        WS_EXPORT Utf8StringCR GetSchemaName () const;
        WS_EXPORT const std::set<Utf8String>& GetClasses () const;

        //! Set $select option. Comma seperated list of: '*', property names, related classes.
        WS_EXPORT WSQuery& SetSelect (Utf8StringCR select);
        WS_EXPORT Utf8StringCR GetSelect () const;

        //! Set $filter option. OData expression. E.g.: "property+eq+'Foo'". 
        //! Unknown string values need to be escaped using EscapeValue.
        WS_EXPORT WSQuery& SetFilter (Utf8StringCR filter);
        WS_EXPORT Utf8StringCR GetFilter () const;

        //! Escape parameter value in filter. E.g.: "property+eq+'" + WSQuery::EscapeValue (userEnteredString) + "'"
        WS_EXPORT static Utf8String EscapeValue (Utf8String value);

        // Set $order by option. E.g.: "property1,property4+asc,property2+desc"
        WS_EXPORT WSQuery& SetOrderBy (Utf8StringCR orderBy);
        WS_EXPORT Utf8StringCR GetOrderBy () const;

        // Set $skip option. Setting to 0 will clear
        WS_EXPORT WSQuery& SetSkip (uint32_t skip);
        // Get $skip option. Will return 0 if not set
        WS_EXPORT uint32_t GetSkip () const;

        // Set $top option. Setting to 0 will clear
        WS_EXPORT WSQuery& SetTop (uint32_t top);
        // Get $top option. Will return 0 if not set
        WS_EXPORT uint32_t GetTop () const;

        // Get generated alias for repeating phrase in a query. Will return same alias if generated before.
        WS_EXPORT Utf8StringCR GetAlias (Utf8StringCR phrase);

        // Get phrases to aliases map
        WS_EXPORT const std::map<Utf8String, Utf8String>& GetAliasMapping () const;

        // Set custom parameter for query. Will overwrite same name parameters
        WS_EXPORT WSQuery& SetCustomParameter (Utf8StringCR name, Utf8StringCR value);
        WS_EXPORT const std::map<Utf8String, Utf8String>& GetCustomParameters () const;

        //! Construct OData style query string
        WS_EXPORT Utf8String ToString () const;
    };

typedef WSQuery& WSQueryR;
typedef const WSQuery& WSQueryCR;
typedef std::shared_ptr<WSQuery> WSQueryPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
