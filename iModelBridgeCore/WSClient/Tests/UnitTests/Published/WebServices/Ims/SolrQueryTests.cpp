/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include <WebServices/Ims/SolrQuery.h>

using namespace ::testing;
using namespace ::std;

struct SolrQueryTests : WSClientBaseTest {};

/*Query Parameters:
q       - Defines a query using standard query syntax. This query is mandatory.
        - Constructor (Category, Values)
fq      - Applies a filter query to the search results.
        - SetResponseFilter(Category, Values)
fl      - Limits the information included in a query response to a specified list of fields.
        - SetRequestFilter(Category)
wt      - The fields Secpfics the Response Writer to be used to format the query response.
        - SetResponseFormat(ResponseFormat::Type)
start   - Specifies an offset (by default, 0) into the responses at which API should begin displaying content.
        - SetStart(int)
rows    - Controls how many rows of responses are displayed at a time (default value: 10)
        - SetRows(int)
indent  - Indent document
        - SetIndent(bool)
sort    - Sorts response
        - SetSort(Category, ResponseSort::Type)
*/
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, Ctor_EmptyQuery)
    {
    SolrQuery queryBuilder;
    EXPECT_STREQ(queryBuilder.GetQuery().c_str(), "");
    EXPECT_STREQ(queryBuilder.GetFilter().c_str(), "");
    EXPECT_STREQ(queryBuilder.GetSelect().c_str(), "");
    EXPECT_STREQ(queryBuilder.GetResponseFormat().c_str(), "");
    EXPECT_STREQ(queryBuilder.GetSort().c_str(), "");
    EXPECT_STREQ(queryBuilder.GetStart().c_str(), "");
    EXPECT_STREQ(queryBuilder.GetRows().c_str(), "");
    EXPECT_STREQ(queryBuilder.GetIndent().c_str(), "");
    EXPECT_STREQ(queryBuilder.ToString().c_str(), "");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetQuery)
    {
    SolrQuery query;
    query.SetQuery("Zip:35801");
    EXPECT_STREQ(query.GetQuery().c_str(), "Zip:35801");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetFilter)
    {
    SolrQuery query;
    query.SetFilter("Zip:35801");
    EXPECT_STREQ(query.GetFilter().c_str(), "Zip:35801");

    query.SetFilter("Zip:(35801 OR 35802)");
    EXPECT_STREQ(query.GetFilter().c_str(), "Zip:(35801 OR 35802)");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetSelect)
    {
    SolrQuery query;
    query.SetSelect("Zip");
    EXPECT_STREQ(query.GetSelect().c_str(), "Zip");
    query.SetSelect("UserId");
    EXPECT_STREQ(query.GetSelect().c_str(), "UserId");
    query.AddSelect("Zip");
    EXPECT_STREQ(query.GetSelect().c_str(), "UserId,Zip");
    query.AddSelect("");
    EXPECT_STREQ(query.GetSelect().c_str(), "UserId,Zip");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetResponseFormat)
    {
    SolrQuery query;
    query.SetResponseFormat("json");
    EXPECT_STREQ(query.GetResponseFormat().c_str(), "json");
    query.SetResponseFormat("standard");
    EXPECT_STREQ(query.GetResponseFormat().c_str(), "standard");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetSort)
    {
    SolrQuery query;
    query.SetSort("UserId asc");
    EXPECT_STREQ(query.GetSort().c_str(), "UserId asc");
    query.SetSort("Zip asc");
    EXPECT_STREQ(query.GetSort().c_str(), "Zip asc");
    query.AddSort("UserId desc");
    EXPECT_STREQ(query.GetSort().c_str(), "Zip asc,UserId desc");
    query.AddSort("");
    EXPECT_STREQ(query.GetSort().c_str(), "Zip asc,UserId desc");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetStart)
    {
    SolrQuery query;
    query.SetStart(0);
    EXPECT_STREQ(query.GetStart().c_str(), "0");
    query.SetStart(10);
    EXPECT_STREQ(query.GetStart().c_str(), "10");
    query.SetStart(-1);
    EXPECT_STREQ(query.GetStart().c_str(), "0");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetRows)
    {
    SolrQuery query;
    query.SetRows(1);
    EXPECT_STREQ(query.GetRows().c_str(), "1");
    query.SetRows(10);
    EXPECT_STREQ(query.GetRows().c_str(), "10");
    query.SetRows(0);
    EXPECT_STREQ(query.GetRows().c_str(), "1");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, SetIndent)
    {
    SolrQuery query;
    query.SetIndent(true);
    EXPECT_STREQ(query.GetIndent().c_str(), "true");
    query.SetIndent(false);
    EXPECT_STREQ(query.GetIndent().c_str(), "false");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetQuery)
    {
    SolrQuery query;
    query.SetQuery("Zip:35801");
    EXPECT_STREQ(query.ToString().c_str(), "?q=Zip:35801");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetFilter)
    {
    SolrQuery query;
    query.SetFilter("Zip:35801");
    EXPECT_STREQ(query.ToString().c_str(), "?fq=Zip:35801");

    query.SetFilter("Zip:(35801 OR 35802)");
    EXPECT_STREQ(query.ToString().c_str(), "?fq=Zip:(35801 OR 35802)");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetSelect)
    {
    SolrQuery query;
    query.SetSelect("Zip");
    EXPECT_STREQ(query.ToString().c_str(), "?fl=Zip");
    query.SetSelect("UserId");
    EXPECT_STREQ(query.ToString().c_str(), "?fl=UserId");
    query.AddSelect("Zip");
    EXPECT_STREQ(query.ToString().c_str(), "?fl=UserId,Zip");
    query.AddSelect("");
    EXPECT_STREQ(query.ToString().c_str(), "?fl=UserId,Zip");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetResponseFormat)
    {
    SolrQuery query;
    query.SetResponseFormat("json");
    EXPECT_STREQ(query.ToString().c_str(), "?wt=json");
    query.SetResponseFormat("standard");
    EXPECT_STREQ(query.ToString().c_str(), "?wt=standard");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetSort)
    {
    SolrQuery query;
    query.SetSort("UserId asc");
    EXPECT_STREQ(query.ToString().c_str(), "?sort=UserId asc");
    query.SetSort("Zip asc");
    EXPECT_STREQ(query.ToString().c_str(), "?sort=Zip asc");
    query.AddSort("UserId desc");
    EXPECT_STREQ(query.ToString().c_str(), "?sort=Zip asc,UserId desc");
    query.AddSort("");
    EXPECT_STREQ(query.ToString().c_str(), "?sort=Zip asc,UserId desc");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetStart)
    {
    SolrQuery query;
    query.SetStart(0);
    EXPECT_STREQ(query.ToString().c_str(), "?start=0");
    query.SetStart(10);
    EXPECT_STREQ(query.ToString().c_str(), "?start=10");
    query.SetStart(-1);
    EXPECT_STREQ(query.ToString().c_str(), "?start=0");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetRows)
    {
    SolrQuery query;
    query.SetRows(1);
    EXPECT_STREQ(query.ToString().c_str(), "?rows=1");
    query.SetRows(10);
    EXPECT_STREQ(query.ToString().c_str(), "?rows=10");
    query.SetRows(0);
    EXPECT_STREQ(query.ToString().c_str(), "?rows=1");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_SetIndent)
    {
    SolrQuery query;
    query.SetIndent(true);
    EXPECT_STREQ(query.ToString().c_str(), "?indent=true");
    query.SetIndent(false);
    EXPECT_STREQ(query.ToString().c_str(), "?indent=false");
    }

//--------------------Ims API Doc Examples Tests--------------------//
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_Conglomerate_q_fl_wt_indent)
    {
    SolrQuery query;
    query.SetQuery("Email:\"jibin.samuel@bentley.com\"")
        .SetSelect("UserId")
        .SetResponseFormat("json")
        .SetIndent(true);
    EXPECT_STREQ(query.ToString().c_str(), "?q=Email:\"jibin.samuel@bentley.com\"&fl=UserId&wt=json&indent=true");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_Conglomerate_q_fl_wt_indent2)
    {
    SolrQuery query;
    query.SetQuery("UserId:\"20ADABDC-14B2-4AE7-A09B-66DE100227E6\"")
        .SetSelect("Email")
        .SetResponseFormat("json")
        .SetIndent(true);
    EXPECT_STREQ(query.ToString().c_str(), "?q=UserId:\"20ADABDC-14B2-4AE7-A09B-66DE100227E6\"&fl=Email&wt=json&indent=true");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_Conglomerate_q_fl_fq_wt_indent)
    {
    SolrQuery query;
    query.SetQuery("RoleId:\"88710CD6-4AA3-45B1-92B3-AB834A7A017E\"")
        .SetFilter("OrganizationId:\"065CFE00-3141-48C1-A3C6-EF4854C019D8\"")
        .SetSelect("UserId")
        .SetResponseFormat("json")
        .SetIndent(true);
    EXPECT_STREQ(query.ToString().c_str(), "?q=RoleId:\"88710CD6-4AA3-45B1-92B3-AB834A7A017E\"&fq=OrganizationId:\"065CFE00-3141-48C1-A3C6-EF4854C019D8\"&fl=UserId&wt=json&indent=true");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_Conglomerate_fq_start_rows_wt_indent)
    {
    SolrQuery query;
    query.SetFilter("UserId:(\"20ADABDC-14B2-4AE7-A09B-66DE100227E6\" OR \"6B54BD1D-DEBD-4954-A26D-A6017D6FF8B0\")")
        .SetStart(0)
        .SetRows(100)
        .SetResponseFormat("json")
        .SetIndent(true);
    EXPECT_STREQ(query.ToString().c_str(), "?fq=UserId:(\"20ADABDC-14B2-4AE7-A09B-66DE100227E6\" OR \"6B54BD1D-DEBD-4954-A26D-A6017D6FF8B0\")&start=0&rows=100&wt=json&indent=true");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_Conglomerate_sort_wt_indent)
    {
    SolrQuery query;
    query.SetSort("FirstName desc")
        .SetResponseFormat("json")
        .SetIndent(true);
    EXPECT_STREQ(query.ToString().c_str(), "?sort=FirstName desc&wt=json&indent=true");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SolrQueryTests, ToString_Conglomerate_q_wt_indent)
    {
    SolrQuery query;
    query.SetQuery("GroupId:\"GroupId\"")
        .SetResponseFormat("json")
        .SetIndent(true);
    EXPECT_STREQ(query.ToString().c_str(), "?q=GroupId:\"GroupId\"&wt=json&indent=true");
    }
