/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnResourceURI_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/DgnResourceURI.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
#if defined (NEEDS_WORK_DGNITEM)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestParser)
    {
    ElementId eid;
    DgnResourceURI::UriToken key,value;
    Utf8Char logical = '\0';

      // Element id path
    {
    DgnResourceURI uri;
    ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString ("DgnElements/2342/") );
    ASSERT_TRUE( uri.ToEncodedString()                                      == "/DgnElements/2342" );
    ASSERT_TRUE( uri.GetPath()                                              == "/DgnElements/2342" );
    ASSERT_TRUE( uri.GetQuery().empty() );
    DgnResourceURI::PathParser pparser (uri.GetPath());
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("DgnElements") );
    ASSERT_TRUE( pparser.ParseNextToken().GetUInt64() == 2342 );
    DgnResourceURI::QueryParser qparser (uri.GetQuery());
    ASSERT_TRUE( qparser.ParseNextToken().GetType() == DgnResourceURI::UriToken::TYPE_EOS );
    }

      // Element by primary instance
    {
    DgnResourceURI uri;
    ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString ("/DgnElements?ECClass=op:Pump&BusinessKey=P-101") );
    ASSERT_TRUE( uri.ToEncodedString()                                       == "/DgnElements?ECClass=op:Pump&BusinessKey=P-101" );
    ASSERT_TRUE( uri.GetPath()                                               == "/DgnElements" );
    ASSERT_TRUE( uri.GetQuery()                                                   == "?ECClass=op:Pump&BusinessKey=P-101" );
    DgnResourceURI::PathParser pparser (uri.GetPath());
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("DgnElements") );
    DgnResourceURI::QueryParser qparser (uri.GetQuery());
    ASSERT_TRUE( qparser.ParseQueryParameter (key, value, logical) == SUCCESS );
    ASSERT_TRUE( logical == 0 );
    ASSERT_TRUE( key.GetString().Equals ("ECClass") && value.GetString().Equals ("op:Pump") );
    ASSERT_TRUE( qparser.ParseQueryParameter (key, value, logical) == SUCCESS );
    ASSERT_TRUE( logical == '&');
    ASSERT_TRUE( key.GetString().Equals ("BusinessKey") && value.GetString().Equals ("P-101") );
    }

      // instance by business key
    {
    DgnResourceURI uri;
    ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString ("/ECInstances/op:Pump/42") );
    ASSERT_TRUE( uri.GetPath()                                               == "/ECInstances/op:Pump/42" );
    ASSERT_TRUE( uri.GetQuery().empty() );
    DgnResourceURI::PathParser pparser (uri.GetPath());
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("ECInstances") );
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("op:Pump") );
    ASSERT_TRUE( pparser.ParseNextToken().GetUInt64() == 42 );
    DgnResourceURI::QueryParser qparser (uri.GetQuery());
    ASSERT_TRUE( qparser.ParseNextToken().GetType() == DgnResourceURI::UriToken::TYPE_EOS );
    }

      // some custom path
    {
    DgnResourceURI uri;
    ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString ("/MyResourceType/myid") );
    ASSERT_TRUE( uri.GetPath()                                               == "/MyResourceType/myid" );
    ASSERT_TRUE( uri.GetQuery().empty() );
    DgnResourceURI::PathParser pparser (uri.GetPath());
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("MyResourceType") );
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("myid") );
    }

      //  some custom path with embedded blanks
    {
    DgnResourceURI uri;
    ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString ("/MyResourceType/myid%20with%20blanks") );
    ASSERT_TRUE( uri.ToEncodedString()                                       == "/MyResourceType/myid%20with%20blanks" );
    ASSERT_TRUE( uri.GetPath()                                               == "/MyResourceType/myid%20with%20blanks" );
    ASSERT_TRUE( uri.GetQuery().empty() );
    DgnResourceURI::PathParser pparser (uri.GetPath());
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("MyResourceType") );
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("myid with blanks") );
    }

      //  some custom path with embedded blanks
    {
    DgnResourceURI uri;
    ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString ("/MyResourceType/myid+with+blanks") );
    ASSERT_TRUE( uri.ToEncodedString()                                       == "/MyResourceType/myid+with+blanks" );
    ASSERT_TRUE( uri.GetPath()                                               == "/MyResourceType/myid+with+blanks" );
    ASSERT_TRUE( uri.GetQuery().empty() );
    DgnResourceURI::PathParser pparser (uri.GetPath());
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("MyResourceType") );
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("myid with blanks") );
    }

     //  Element by provenance
    {
    DgnResourceURI uri;
    ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString ("/DgnElements?SourceId=abc.i.dgn&ElementId=123") );
    ASSERT_TRUE( uri.GetPath()                                               == "/DgnElements" );
    ASSERT_TRUE( uri.GetQuery()                                                          == "?SourceId=abc.i.dgn&ElementId=123" );
    DgnResourceURI::PathParser pparser (uri.GetPath());
    ASSERT_TRUE( pparser.ParseNextToken().GetString().Equals ("DgnElements") );
    DgnResourceURI::QueryParser qparser (uri.GetQuery());
    ASSERT_TRUE( qparser.ParseQueryParameter (key, value, logical) == SUCCESS );
    ASSERT_TRUE( logical == 0 );
    ASSERT_TRUE( key.GetString().Equals ("SourceId") && value.GetString().Equals ("abc.i.dgn") );
    ASSERT_TRUE( qparser.ParseQueryParameter (key, value, logical) == SUCCESS );
    ASSERT_TRUE( logical == '&');
    ASSERT_TRUE( key.GetString().Equals ("ElementId") && value.GetUInt64() == 123 );
    }
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestDgnElements)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm (L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    DgnModelP defaultModel = project->Models().GetModel (project->Models().GetFirstModelId());
    ASSERT_TRUE( defaultModel != nullptr );
    defaultModel->FillModel();

    bmap<Utf8String, PersistentDgnElementP> uris;

    FOR_EACH (PersistentDgnElementP ref, defaultModel->GetElementsCollection ())
        {
        DgnResourceURI uri;
        ASSERT_TRUE( SUCCESS == ref->CreateDgnResourceURI (uri) );
        uris[uri.ToEncodedString()] = ref;
        }

    for (auto pair : uris)
        {
        DgnResourceURI uri;
        ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString (pair.first.c_str()) );
        PersistentDgnElementPtr ref = project->Models().GetElementById (project->Models().GetElementByURI (uri));
        ASSERT_EQ( ref.get(), pair.second );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestECInstances)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm (L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );

    DgnModelP defaultModel = project->Models().GetModel (project->Models().GetFirstModelId());
    ASSERT_TRUE( defaultModel != nullptr );
    project->Models().FillSectionsInModel (*defaultModel);

    bmap<Utf8String, BeSQLite::EC::ECInstanceKey> uris;

    FOR_EACH (PersistentDgnElementP ref, defaultModel->GetElementsCollection ())
        {
        ElementHandle eh (ref);
        BeSQLite::EC::ECInstanceKey instanceKey;
        if (DgnECPersistence::GetPrimaryInstanceOnElement (instanceKey, eh))
            {
            DgnResourceURI uri;
            ASSERT_TRUE( SUCCESS == project->Models().CreateDgnResourceURI (uri, instanceKey) );
            uris[uri.ToEncodedString()] = instanceKey;
            }
        }

    for (auto pair : uris)
        {
        DgnResourceURI uri;
        ASSERT_TRUE( DgnResourceURI::PARSE_STATUS_SUCCESS == uri.FromEncodedString (pair.first.c_str()) );
        BeSQLite::EC::ECInstanceKey instanceKey;
        ASSERT_TRUE( SUCCESS == project->Models().FindECInstanceByURI (instanceKey, uri) );
        ASSERT_EQ( instanceKey.GetECClassId(), pair.second.GetECClassId() );
        ASSERT_EQ( instanceKey.GetECInstanceId().GetValue (), pair.second.GetECInstanceId().GetValue());
        }

    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hassan.Arshad                      01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestBuildUriPath)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm(L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);
           
    DgnResourceURI::UriToken key, value;
       
    DgnResourceURI::Builder uri;
    uri.SetTargetFile(*project);
    Utf8CP targetPath = "DgnElements / 2342 / ";
    uri.AppendPath(targetPath);
            
    DgnResourceURI newUri = uri.ToDgnResourceURI();
    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("DgnElements/2342/"));
    ASSERT_TRUE(newUri.GetPath() == "/DgnElements/2342")<<newUri.GetPath();
    ASSERT_TRUE(newUri.GetQuery().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hassan.Arshad                      01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestBuildUriQueryParams)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm(L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);
                  
    DgnResourceURI::UriToken key, value;
    DgnResourceURI::Builder uri;
    uri.SetTargetFile(*project);
    Utf8CP targetPath = "DgnElements / 2342 / ";
    uri.AppendPath(targetPath);
    Utf8CP newKey;
    Utf8CP newValue;
        
    newKey = "DgnElements";
    newValue = "2342";
    uri.AppendQueryParameter(newKey, newValue);         

    DgnResourceURI newUri = uri.ToDgnResourceURI();
    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("?DgnElements=2342"));
    ASSERT_TRUE(newUri.GetQuery() == "?DgnElements=2342"); 

    DgnResourceURI::QueryParser qparser(newUri.GetQuery());
    ASSERT_TRUE(qparser.ParseNextToken().GetType() == DgnResourceURI::UriToken::TYPE_String) ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hassan.Arshad                  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestBuildUriSetEncodedPath)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm(L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);
        
    DgnResourceURI::UriToken key, value;
    // Element id path

    DgnResourceURI::Builder uri;
    uri.SetTargetFile(*project);
    Utf8CP targetPath = "DgnElements / 2342 / ";

    uri.SetEncodedPath(targetPath);
    uri.AppendEncodedPath(targetPath);
        
    DgnResourceURI newUri = uri.ToDgnResourceURI();
    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("DgnElements/2342/"));
    ASSERT_TRUE(newUri.GetPath() == "/DgnElements/2342") << newUri.GetPath();
    ASSERT_TRUE(newUri.GetQuery().empty()); 

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hassan.Arshad                  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestBuildUriSetEncodedQuery)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm(L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);
        
    DgnResourceURI::UriToken key, value;
        
    DgnResourceURI::Builder uri;
    uri.SetTargetFile(*project);

    Utf8CP newQuery = "DgnElement=2342";
    uri.SetEncodedQuery(newQuery);
    DgnResourceURI newUri = uri.ToDgnResourceURI();
    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("?DgnElements=2342"));
    DgnResourceURI::QueryParser qparser(newUri.GetQuery());
    ASSERT_TRUE(qparser.ParseNextToken().GetType() == DgnResourceURI::UriToken::TYPE_String);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hassan.Arshad                  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestBuildUriUtf8StringCRQuery)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm(L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);
        
    DgnResourceURI::UriToken key, value;

    DgnResourceURI::Builder uri;
        uri.SetTargetFile(*project);

    Utf8CP newQuery = "DgnElement=2342";

    uri.SetEncodedQuery(newQuery);
        
    DgnResourceURI newUri = uri.ToDgnResourceURI();
    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("?DgnElements=2342"));
    DgnResourceURI::QueryParser qparser(newUri.GetQuery());
    ASSERT_TRUE(qparser.ParseNextToken().GetType() == DgnResourceURI::UriToken::TYPE_String);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hassan.Arshad                      01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestBuildUriAppendPath)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm(L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    DgnResourceURI::UriToken key, value;
    DgnResourceURI::Builder uri;
    uri.SetTargetFile(*project);
    Utf8String targetPath = "DgnElements / 2342 / ";
    uri.AppendPath(targetPath);
    Utf8CP newFragment = "/12345678H";
    uri.SetEncodedFragment(newFragment);
    DgnResourceURI newUri = uri.ToDgnResourceURI();
    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("#/12345678H"));
    ASSERT_TRUE(newUri.GetFragment() == "#/12345678H") << newUri.GetFragment();

    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("DgnElements/2342/"));
    ASSERT_TRUE(newUri.GetPath() == "/DgnElements/2342") << newUri.GetPath();
    ASSERT_TRUE(newUri.GetQuery().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Hassan.Arshad                      01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnResourceURI_Test, TestBuildUriParseCompleteURI)
    {
    ScopedDgnHost  autoDgnHost;

    DgnDbTestDgnManager tdm(L"Mobile_file.i.idgndb", __FILE__, OPENMODE_READONLY);
    DgnDbP project = tdm.GetDgnProjectP();
    ASSERT_TRUE(project != NULL);

    DgnResourceURI::UriToken key, value;
    DgnResourceURI::Builder uri;
    uri.SetTargetFile(*project);
    Utf8String targetPath = "DgnElements / 2342 / ";
    uri.AppendPath(targetPath);
    Utf8CP newFragment = "/12345678H";
    uri.SetEncodedFragment(newFragment);
    Utf8CP newQuery = "DgnElement=2342";
    uri.SetEncodedQuery(newQuery);

    DgnResourceURI newUri = uri.ToDgnResourceURI();
    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("#/12345678H"));
    ASSERT_TRUE(newUri.GetFragment() == "#/12345678H") << newUri.GetFragment();

    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("DgnElements/2342/"));
    ASSERT_TRUE(newUri.GetPath() == "/DgnElements/2342") << newUri.GetPath();
    ASSERT_TRUE(newUri.GetQuery().empty());

    ASSERT_TRUE(DgnResourceURI::PARSE_STATUS_SUCCESS == newUri.FromEncodedString("?DgnElements=2342"));
    ASSERT_TRUE(newUri.GetQuery() == "?DgnElements=2342") << newUri.GetQuery();
    }
#endif
