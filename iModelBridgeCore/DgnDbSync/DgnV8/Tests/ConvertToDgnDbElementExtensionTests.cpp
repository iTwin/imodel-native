/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"
#include <iModelBridge/iModelBridgeSacAdapter.h>


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConvertToDgnDbElementExtensionTests : public ConverterTestBaseFixture
    {
    virtual void SetUp() override;
    virtual void TearDown() override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestHandlerExtension : public ConvertToDgnDbElementExtension
    {
    static TestHandlerExtension* s_instance;
    bool                         m_isPreConvertCalled;

    TestHandlerExtension()
        :m_isPreConvertCalled(false)
        {

        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Abeesh.Basheer                  01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void Register()
        {
        if (nullptr == s_instance)
            s_instance = new TestHandlerExtension();
        RegisterExtension(DgnV8Api::LineHandler::GetInstance(), *s_instance);
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Abeesh.Basheer                  01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void UnRegister()
        {
        DropExtension(DgnV8Api::LineHandler::GetInstance());
        delete s_instance;
        s_instance = nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Abeesh.Basheer                  01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual Result _PreConvertElement(DgnV8EhCR, Converter&, ResolvedModelMapping const&) 
        {
        m_isPreConvertCalled = true;
        return Result::SkipElement;
        }
    };

TestHandlerExtension* TestHandlerExtension::s_instance = NULL;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConvertToDgnDbElementExtensionTests::SetUp()
    {
    ConverterTestBaseFixture::SetUp();
    TestHandlerExtension::Register();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void            ConvertToDgnDbElementExtensionTests::TearDown()
    {
    TestHandlerExtension::UnRegister();
    ConverterTestBaseFixture::TearDown();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConvertToDgnDbElementExtensionTests, IsPreConvertCalled)
    {
    LineUpFiles(L"AddResourceFileToFwkDb.bim", L"Test3d.dgn", false); // defines m_dgnDbFileName, and m_v8FileName
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1);
    v8editor.Save();

    //RootModelConverterApp v8Bridge;
    //iModelBridge::Params params;
    //iModelBridgeSacAdapter::InitForBeTest(params);
    //iModelBridgeSacAdapter::Execute(*v8Bridge, params);
    DoConvert(m_dgnDbFileName, m_v8FileName);

    EXPECT_TRUE(TestHandlerExtension::s_instance->m_isPreConvertCalled);
    }
