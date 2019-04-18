/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__


/*SolrQuery Parameters:
Q       - Defines a query using standard query syntax. This query is mandatory.
- SetQuery (Utf8String)
Fq      - Applies a filter query to the search results.
- SetFilter(Utf8String)
Wt      - The fields specifies the Response Writer to be used to format the query response. (i.e. json, standard)
- SetResponseFormat(Utf8String)
Sort    - Sorts response; desc for descending, asc for ascending
- SetSort(Utf8String)
- AddSort(Utf8String)
Fl      - Limits the information included in a query response to a specified list of fields.
- SetSelect(Utf8String)
- AddSelect(Utf8String)
Start   - Specifies an offset (by default, 0) into the responses at which API should begin displaying content.
- SetStart(int)
Rows    - Controls how many rows of responses are displayed at a time (default value: 10)
- SetRows(int)
Indent  - Indent document
- SetIndentation(bool)
*/

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
class SolrQuery
    {
    private:
        Utf8String  m_qParameter;
        Utf8String  m_fqParameter;
        Utf8String  m_wtParameter;
        Utf8String  m_sortParameter;
        Utf8String  m_flParameter;
        Utf8String  m_startParameter;
        Utf8String  m_rowsParameter;
        Utf8String  m_indentParameter;

    private:
        static void AppendOptionalParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value);
        static void AppendParameter(Utf8StringR query, Utf8StringCR name, Utf8StringCR value);

    public:
        //! Construct SolrQuery
        //! This is based on the Solr REST Api, meaning it allows the same syntax for all parameters. 
        //! The IMS Search API builds on top of the Solr REST API by allowing for token-based REST calls, but the parameter options are the same.
        //! Note: If the SolrQuery is token-based, the q parameter is prohibited, and excluded in the Url generation.
        WSCLIENT_EXPORT SolrQuery();
        //! Accessor to see if this SolrQuery isTokenBased, or not.
        WSCLIENT_EXPORT const bool IsTokenBased() const;

        //! Set the q parameter of the Solr REST Url.
        //! The q parameter is only allowed (and required) if the SolrQuery is not token-based. When the SolrQuery is token-based, the q parameter is gathered from the authenticated token
        //! @param[in] qString - The value of the q parameter. 
        WSCLIENT_EXPORT SolrQuery& SetQuery(Utf8StringCR qString);
        //! Get the q parameter
        WSCLIENT_EXPORT Utf8StringCR GetQuery() const;

        //! Sets the fq parameter of the Solr REST Url
        //! @param[in] fqString - the value of the fq parameter
        //! @return SolrQuery reference to chain query building
        WSCLIENT_EXPORT SolrQuery& SetFilter(Utf8StringCR fqString);
        //! Get the fq parameter
        WSCLIENT_EXPORT Utf8StringCR GetFilter() const;

        //! Sets the fl parameter of the Solr REST Url
        //! @param[in] flString - the value of the fl parameter
        //! @return SolrQuery reference to chain query building
        WSCLIENT_EXPORT SolrQuery& SetSelect(Utf8StringCR flString);
        //! Append string to the fl parameter. Seperating comma will be added if parameter was not empty
        WSCLIENT_EXPORT SolrQuery& AddSelect(Utf8StringCR flString);
        //! Get the fl parameter
        WSCLIENT_EXPORT Utf8StringCR GetSelect() const;

        //! Sets the wt parameter for the REST Url. This field specifies the Response Writer to be used to format the query response.
        //! @param[in] wtString - The requested format of the http response
        //! @return SolrQuery reference to chain query building
        WSCLIENT_EXPORT SolrQuery& SetResponseFormat(Utf8StringCR wtString);
        //! Get the wt parameter
        WSCLIENT_EXPORT Utf8StringCR GetResponseFormat() const;

        //! Sets the sort parameter for the REST Url. Specifies how the response should be sorted
        //! @param[in] sortString - the value of the sort parameter; Formatted like "Category SortType", category being one of the search fields, and sortType being asc (ascending) or desc (descending)
        //! @return SolrQuery reference to chain query building
        WSCLIENT_EXPORT SolrQuery& SetSort(Utf8StringCR sortString);
        //! Append string to the sort parameter. Seperating comma will be added if parameter was not empty
        WSCLIENT_EXPORT SolrQuery& AddSort(Utf8StringCR sortString);
        //! Get the sort parameter
        WSCLIENT_EXPORT Utf8StringCR GetSort() const;

        //! Sets the start parameter for the REST Url. Specifies an offset (by default, 0) into the responses at which API should begin displaying content.
        //! @param[in] start - The start number
        //! @return SolrQuery reference to chain query building
        WSCLIENT_EXPORT SolrQuery& SetStart(const int start);
        //! Get the start parameter
        WSCLIENT_EXPORT Utf8StringCR GetStart() const;

        //! Sets the rows parameter for the REST Url. Controls how many rows of responses are displayed at a time (default value: 10).
        //! @param[in] rows - The rows number
        //! @return - SolrQuery reference to chain query building
        WSCLIENT_EXPORT SolrQuery& SetRows(const int rows);
        //! Get the rows parameter
        WSCLIENT_EXPORT Utf8StringCR GetRows() const;

        //! Sets the indent parameter for the REST Url. Whether or not the response is indented.
        //! @param[in] shouldIndent - The indent specifier
        //! @return - SolrQuery reference to chain query building
        WSCLIENT_EXPORT SolrQuery& SetIndent(const bool shouldIndent);
        //! Get the indent parameter
        WSCLIENT_EXPORT Utf8StringCR GetIndent() const;

        //! Constructs the query string, which is formatted for the Solr REST API.
        //! @return - The constructed query string included all correctly set parameters
        WSCLIENT_EXPORT Utf8String ToString() const;
    };

typedef SolrQuery& SolrQueryR;
typedef const SolrQuery& SolrQueryCR;
typedef std::shared_ptr<SolrQuery> SolrQueryPtr;

END_BENTLEY_WEBSERVICES_NAMESPACE
