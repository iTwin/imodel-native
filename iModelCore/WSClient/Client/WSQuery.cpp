/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WSQuery.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <BeHttp/HttpClient.h>

#define WSQUERY_OPTION_Select   "$select"
#define WSQUERY_OPTION_Filter   "$filter"
#define WSQUERY_OPTION_OrderBy  "$orderby"
#define WSQUERY_OPTION_Skip     "$skip"
#define WSQUERY_OPTION_Top      "$top"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_EC

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery::WSQuery(Utf8StringCR schemaName, std::set<Utf8String> classes) :
m_mainSchemaName(schemaName),
m_classes(classes),
m_skip(0),
m_top(0),
m_aliasNumber(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery::WSQuery(Utf8StringCR schemaName, Utf8StringCR className) :
WSQuery(schemaName, std::set<Utf8String> {className})
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery::WSQuery(ECClassCR ecClass, bool polymorphic) :
WSQuery
(
Utf8String(ecClass.GetSchema().GetName()),
Utf8PrintfString("%s%s", Utf8String(ecClass.GetName()).c_str(), polymorphic ? "!poly" : "")
)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery::WSQuery(ObjectIdCR objectId) :
WSQuery(objectId.schemaName, objectId.className)
    {
    SetFilter("$id+in+['" + EscapeValue(objectId.remoteId) + "']");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSQuery::EscapeValue(Utf8String value)
    {
    value.ReplaceAll("'", "''");
    return HttpClient::EscapeString(value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSQuery::GetSchemaName() const
    {
    return m_mainSchemaName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const std::set<Utf8String>& WSQuery::GetClasses() const
    {
    return m_classes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::SetSelect(Utf8StringCR select)
    {
    m_select = select;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::AddSelect(Utf8StringCR select)
    {
    if (!m_select.empty() && !select.empty())
        {
        m_select += ',';
        }
    m_select += select;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSQuery::GetSelect() const
    {
    return m_select;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::SetFilter(Utf8StringCR filter)
    {
    m_filter = filter;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSQuery::GetFilter() const
    {
    return m_filter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::SetOrderBy(Utf8StringCR orderBy)
    {
    m_orderBy = orderBy;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSQuery::GetOrderBy() const
    {
    return m_orderBy;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::SetSkip(uint32_t skip)
    {
    m_skip = skip;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t WSQuery::GetSkip() const
    {
    return m_skip;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::SetTop(uint32_t top)
    {
    m_top = top;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t WSQuery::GetTop() const
    {
    return m_top;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSQuery::GetAlias(Utf8StringCR phrase)
    {
    auto it = m_aliases.find(phrase);
    if (it != m_aliases.end())
        {
        return it->second;
        }

    Utf8String alias = CreateNewAlias();
    it = m_aliases.insert({phrase, alias}).first;
    m_phrases.insert({alias, phrase});

    return it->second;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSQuery::CreateNewAlias()
    {
    Utf8Char alphabetSize = 'Z' - 'A' + 1;
    Utf8Char aliasCharacter = 'A' + m_aliasNumber % alphabetSize;
    uint64_t aliasPostfixNumber = m_aliasNumber / alphabetSize;

    m_aliasNumber++;

    if (0 == aliasPostfixNumber)
        {
        return Utf8PrintfString("@%c", aliasCharacter);
        }

    return Utf8PrintfString("@%c%llu", aliasCharacter, aliasPostfixNumber);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP WSQuery::GetPhrase(Utf8StringCR phrase)
    {
    auto it = m_phrases.find(phrase);
    if (it != m_phrases.end())
        {
        return it->second.c_str();
        }
    return nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const std::map<Utf8String, Utf8String>& WSQuery::GetAliasMapping() const
    {
    return m_aliases;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::SetCustomParameter(Utf8StringCR name, Utf8StringCR value)
    {
    m_customParameters[name] = value;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
WSQuery& WSQuery::RemoveCustomParameter(Utf8StringCR name)
    {
    m_customParameters.erase(name);
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const std::map<Utf8String, Utf8String>& WSQuery::GetCustomParameters() const
    {
    return m_customParameters;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSQuery::AppendOptionalParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value)
    {
    if (name.empty() || value.empty())
        {
        return;
        }

    AppendParameter(query, name, value);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void WSQuery::AppendParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value)
    {
    if (!query.empty())
        {
        query += "&";
        }

    query += name + "=" + value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void WSQuery::AppendOptionalParameter(Utf8StringR query, Utf8StringCR name, uint32_t value)
    {
    if (0 == value)
        {
        return;
        }

    AppendOptionalParameter(query, name, Utf8PrintfString("%lu", value));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSQuery::ToQueryString() const
    {
    Utf8String query;

    AppendOptionalParameter(query, WSQUERY_OPTION_Select, m_select);
    AppendOptionalParameter(query, WSQUERY_OPTION_Filter, m_filter);
    AppendOptionalParameter(query, WSQUERY_OPTION_OrderBy, m_orderBy);
    AppendOptionalParameter(query, WSQUERY_OPTION_Skip, m_skip);
    AppendOptionalParameter(query, WSQUERY_OPTION_Top, m_top);

    for (auto pair : m_aliases)
        {
        AppendOptionalParameter(query, pair.second, pair.first);
        }

    for (auto pair : m_customParameters)
        {
        AppendParameter(query, pair.first, pair.second);
        }

    return query;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String WSQuery::ToFullString() const
    {
    return m_mainSchemaName + '/' + StringUtils::Join(m_classes.begin(), m_classes.end(), ',') + '?' + ToQueryString();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSQuery::operator== (const WSQuery& other) const
    {
    return
        m_mainSchemaName == other.m_mainSchemaName &&
        m_classes == other.m_classes &&
        ToQueryString() == other.ToQueryString();
    }
