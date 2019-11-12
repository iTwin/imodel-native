/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ClientInternal.h"
#include <WebServices/Ims/SolrQuery.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery::SolrQuery()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetQuery(Utf8StringCR qString)
    {
    m_qParameter = qString;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetQuery() const
    {
    return m_qParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetFilter(Utf8StringCR fqString)
    {
    m_fqParameter = fqString;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetFilter() const
    {
    return m_fqParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetSelect(Utf8StringCR flString)
    {
    m_flParameter = flString;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::AddSelect(Utf8StringCR flString)
    {
    if (!m_flParameter.empty() && !flString.empty())
        {
        m_flParameter += ',';
        }
    m_flParameter += flString;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetSelect() const
    {
    return m_flParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetResponseFormat(Utf8StringCR wtString)
    {
    m_wtParameter = wtString;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetResponseFormat() const
    {
    return m_wtParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetSort(Utf8StringCR sortString)
    {
    m_sortParameter = sortString;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::AddSort(Utf8StringCR sortString)
    {
    if (!m_sortParameter.empty() && !sortString.empty())
        {
        m_sortParameter += ',';
        }
    m_sortParameter += sortString;
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetSort() const
    {
    return m_sortParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetStart(const int start)
    {
    m_startParameter = Utf8PrintfString("%i", start >= 0 ? start : 0);
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetStart() const
    {
    return m_startParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetRows(const int rows)
    {
    m_rowsParameter = Utf8PrintfString("%i", rows > 0 ? rows : 1);
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetRows() const
    {
    return m_rowsParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrQuery& SolrQuery::SetIndent(const bool indent)
    {
    m_indentParameter = Utf8PrintfString("%s", Utf8String(indent ? "true" : "false").c_str());
    return *this;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrQuery::GetIndent() const
    {
    return m_indentParameter;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void SolrQuery::AppendOptionalParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value)
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
void SolrQuery::AppendParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value)
    {
    if (!query.empty())
        {
        query += "&";
        }

    query += name + "=" + value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SolrQuery::ToString() const
    {
    Utf8String query;

    AppendOptionalParameter(query, "q",      m_qParameter);
    AppendOptionalParameter(query, "fq",     m_fqParameter);
    AppendOptionalParameter(query, "fl",     m_flParameter);
    AppendOptionalParameter(query, "start",  m_startParameter);
    AppendOptionalParameter(query, "rows",   m_rowsParameter);
    AppendOptionalParameter(query, "sort",   m_sortParameter);
    AppendOptionalParameter(query, "wt",     m_wtParameter);
    AppendOptionalParameter(query, "indent", m_indentParameter);

    return query.empty() ? query : Utf8PrintfString("?%s", query.c_str());
    }
