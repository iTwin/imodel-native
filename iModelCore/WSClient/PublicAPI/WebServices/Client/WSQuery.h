/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/WSQuery.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/ObjectId.h>
#include <ECObjects/ECSchema.h>
#include <Bentley/WString.h>
#include <deque>
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
        std::map<Utf8String, Utf8String> m_phrases;
        std::map<Utf8String, Utf8String> m_customParameters;

    private:
        Utf8String CreateNewAlias();
        static void AppendOptionalParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value);
        static void AppendOptionalParameter(Utf8StringR query, Utf8StringCR name, uint32_t value);
        static void AppendParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value);

    public:
        //! Construct query for given classes. If more than one schema is used, class name need to be prefixed with schema: "SchemaName.ClassName"
        //! @param[in] schemaName - main schema name to query against ("SchemaName")
        //! @param[in] classes - list of classes.
        //!     When class is from main schema, string should contain class name only ("ClassName").
        //!     If class is from other schema, it should be prefixed with schema name ("SchemaName.ClassName").
        WSCLIENT_EXPORT WSQuery(Utf8StringCR schemaName, std::set<Utf8String> classes);

        //! Construct query for given class.
        //! @param[in] schemaName - schema name ("SchemaName")
        //! @param[in] className - class name only ("ClassName")
        WSCLIENT_EXPORT WSQuery(Utf8StringCR schemaName, Utf8StringCR className);

        //! Construct query for given class.
        WSCLIENT_EXPORT WSQuery(ECN::ECClassCR ecClass, bool polymorphic = false);

        //! Construct query for given object
        WSCLIENT_EXPORT WSQuery(ObjectIdCR objectId);

        WSCLIENT_EXPORT Utf8StringCR GetSchemaName() const;
        WSCLIENT_EXPORT const std::set<Utf8String>& GetClasses() const;

        //! Set $select option. Comma seperated list of: '*', property names, related classes.
        //! Spaces are not allowed.
        WSCLIENT_EXPORT WSQuery& SetSelect(Utf8StringCR select);
        //! Add select to $select option. Seperating comma will be added if select was not empty
        WSCLIENT_EXPORT WSQuery& AddSelect(Utf8StringCR select);
        //! Get current select option
        WSCLIENT_EXPORT Utf8StringCR GetSelect() const;

        //! Set $filter option. OData expression.
        //! Spaces are not allowed - use '+' to seperate operators (example: "Property+eq+'Foo'").
        //! Unknown string values (for example ones that are entered by user) need to be escaped using EscapeValue().
        WSCLIENT_EXPORT WSQuery& SetFilter(Utf8StringCR filter);

        //! Add $filter $id+in+[...]
        //! @param[in][out] idsInOut - ids to create filter from. Will remove ids that were added.
        //! Will only use ids that are compatible with current query (by schema and classes).
        //! @param[out] idsAddedOut - [optional] return objects that were used for filter.
        //! @param[in] maxIdsInFilter - maximum count of ids in filter. Defaults to 100 - reasonable server/client load.
        //! @param[in] maxFilterLength - maximum lenght of filter. Servers usually limit maximum URL length to 2K.
        WSCLIENT_EXPORT WSQuery& AddFilterIdsIn
            (
            std::deque<ObjectId>& idsInOut,
            std::set<ObjectId>* idsAddedOut = nullptr,
            size_t maxIdsInFilter = 100,
            size_t maxFilterLength = 1024
            );

        WSCLIENT_EXPORT Utf8StringCR GetFilter() const;

        //! Escape parameter value in filter. Example: SetFilter ("Property+eq+'" + WSQuery::EscapeValue (userEnteredString) + "'");
        WSCLIENT_EXPORT static Utf8String EscapeValue(Utf8String value);

        //! Set $order by option.
        //! Spaces are not allowed, use '+' to seperate operators (example: "property1,property4+asc,property2+desc").
        WSCLIENT_EXPORT WSQuery& SetOrderBy(Utf8StringCR orderBy);
        WSCLIENT_EXPORT Utf8StringCR GetOrderBy() const;

        //! Set $skip option. Setting to 0 will clear option.
        WSCLIENT_EXPORT WSQuery& SetSkip(uint32_t skip);
        //! Get $skip option. Will return 0 if not set.
        WSCLIENT_EXPORT uint32_t GetSkip() const;

        //! Set $top option. Setting to 0 will clear option.
        WSCLIENT_EXPORT WSQuery& SetTop(uint32_t top);
        //! Get $top option. Will return 0 if not set.
        WSCLIENT_EXPORT uint32_t GetTop() const;

        //! Get auto-generated alias for repeating phrase in a query. Will return same alias if for same phrase.
        WSCLIENT_EXPORT Utf8StringCR GetAlias(Utf8StringCR phrase);

        //! Get phrase for generated alias. Returns null if alias not found
        WSCLIENT_EXPORT Utf8CP GetPhrase(Utf8StringCR alias);

        //! Get phrases-to-aliases map
        WSCLIENT_EXPORT const std::map<Utf8String, Utf8String>& GetAliasMapping() const;

        //! Set custom parameter for query. Will overwrite values with same name.
        WSCLIENT_EXPORT WSQuery& SetCustomParameter(Utf8StringCR name, Utf8StringCR value);

        //! Remove custom parameter from query if it exists.
        WSCLIENT_EXPORT WSQuery& RemoveCustomParameter(Utf8StringCR name);

        WSCLIENT_EXPORT const std::map<Utf8String, Utf8String>& GetCustomParameters() const;

        //! Construct OData style query string (URL part after '?'). Schema and class list will not be included.
        WSCLIENT_EXPORT Utf8String ToQueryString() const;

        //! Construct string representing whole query. Includes schema, class list and query string.
        WSCLIENT_EXPORT Utf8String ToFullString() const;

        WSCLIENT_EXPORT bool operator==(const WSQuery& other) const;
    };

typedef WSQuery& WSQueryR;
typedef const WSQuery& WSQueryCR;
typedef std::shared_ptr<WSQuery> WSQueryPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
