/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/RefImportTest.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
USING_NAMESPACE_BENTLEY_SQLITE

# ifdef WIP_NoMoreTextElements

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void countModelsInFirstView (size_t& count, DgnDbR project)
    {
    count = 0;
    for (auto& viewEntry : project.Views().MakeIterator())
        {
        if (viewEntry.GetDgnViewType() == DgnViewType::Physical)
            {
            auto view = project.Views().QueryViewById (viewEntry.GetDgnViewId());
            
            PhysicalViewController phys(project, viewEntry.GetDgnViewId());
            phys.Load();

            for (auto selectedModelId : phys.GetViewedModels())
                {
                ++count;
                }

            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void countTextElementsInProject (size_t& modelCount, size_t& textElementCount, DgnDbR project)
    {
    //  There should be 4 models containing a total of 4 text elements
    modelCount=0;
    textElementCount=0;
    for (auto& modelEntry : project.Models().MakeIterator())
        {
        ++modelCount;

        auto dgnModel = project.Models().GetAndFillModelById (NULL, modelEntry.GetModelId(), /*fillCache*/true);
        for (auto ref : *dgnModel)
            {
            ElementHandle eh (ref);
            if (NULL != dynamic_cast<ITextQuery*>(&eh.GetHandler()))
                ++textElementCount;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void countModelsAndTextElements (DgnDbR project, size_t expectedModelCount, size_t expectedTextElementCount)
    {
    //  There should be at least one physical view, which selects all 4 models.
    size_t modelCount = 0;
    countModelsInFirstView (modelCount, project);
    ASSERT_EQ( modelCount, expectedModelCount );

    //  There should be 4 models containing a total of 4 text elements
    modelCount=0;
    size_t textElementCount=0;
    countTextElementsInProject (modelCount, textElementCount, project);

    ASSERT_EQ( modelCount, expectedModelCount );
    ASSERT_EQ( textElementCount, expectedTextElementCount );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RefImport, DupDirectAttachments)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"master3d_dup_direct_attachments.i.idgndb", __FILE__, Db::OPEN_Readonly);
    auto project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );
    countModelsAndTextElements (*project, 4, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RefImport, LiveNesting)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"master3d_live_nesting.i.idgndb", __FILE__, Db::OPEN_Readonly);
    auto project = tdm.GetDgnProjectP();
    countModelsAndTextElements (*project, 4, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RefImport, NoNesting)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"master3d_no_nesting.i.idgndb", __FILE__, Db::OPEN_Readonly);
    auto project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );
    countModelsAndTextElements (*project, 3, 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RefImport, BFS)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"tfs96614.i.idgndb", __FILE__, Db::OPEN_Readonly);
    auto project = tdm.GetDgnProjectP();
    ASSERT_TRUE( project != NULL );
    countModelsAndTextElements (*project, 4, 4);
    }

#endif
