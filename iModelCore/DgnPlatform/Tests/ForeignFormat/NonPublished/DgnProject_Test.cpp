/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ForeignFormat/NonPublished/DgnProject_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ForeignFormatTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>

#include <DgnPlatform/ForeignFormat/DgnProjectImporter.h>
#include <DgnPlatform/ForeignFormat/DgnV8ProjectImporter.h>
#include <libxml/xpathInternals.h>

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_SQLITE 

static void openProject (DgnProjectPtr& project, BeFileName const& projectFileName, BeSQLite::Db::OpenMode mode = BeSQLite::Db::OPEN_Readonly)
    {
    DgnFileStatus result;
    project = DgnProject::OpenProject (&result, projectFileName, DgnProject::OpenParams(mode));

    ASSERT_TRUE( project != NULL);
    ASSERT_TRUE( result == DGNFILE_STATUS_Success);

    Utf8String projectFileNameUtf8 (projectFileName.GetName());
    ASSERT_TRUE( projectFileNameUtf8 == Utf8String(project->GetDbFileName  ()));
    }

static void inspectDgnFile (DgnProjectP project, bool isReadOnly, bool expectFilledModels)
    {
    ASSERT_TRUE( project->IsReadonly() == isReadOnly );
    ASSERT_TRUE( project->IsDbOpen() );

    bool foundDefault = false;
    size_t modelCount=0;
    for (auto const& entry : project->Models().MakeIterator ())
        {
        foundDefault |= project->Models().GetFirstModelId() == entry.GetModelId();

        DgnModelP model = project->Models().GetModelById (entry.GetModelId());
        ASSERT_TRUE( model != NULL );
        ASSERT_TRUE( expectFilledModels == (model->IsFilled (DgnModelSections::Model) == DgnModelSections::Model) );
        modelCount++;
        }
    ASSERT_TRUE( modelCount == 1 ) << "The model count should have been 1 but it is: " << modelCount;
    ASSERT_TRUE( foundDefault );

    StatusInt mresult;
    DgnModelP defaultModel = project->Models().GetAndFillModelById (&mresult, project->Models().GetFirstModelId());
    ASSERT_TRUE( defaultModel != NULL );
    ASSERT_TRUE( mresult == SUCCESS );
    ASSERT_TRUE( defaultModel == project->Models().FindModelById (defaultModel->GetModelId()) );
    ASSERT_TRUE( expectFilledModels == (defaultModel->IsFilled (DgnModelSections::Model) == DgnModelSections::Model ) );
    if (!expectFilledModels)
        ASSERT_TRUE( defaultModel->FillSections (DgnModelSections::Model) == SUCCESS );

    size_t ecount=0;
    FOR_EACH (PersistentElementRefP ref, defaultModel->GetElementsCollection ())
        {
        ElementHandle eh (ref);

        // *** NEEDS WORK

        ++ecount;

        // *** NEEDS WORK
        }
    }

/*-----------------------------------------------------------------------------------**//**
* The same test as CreateFromDgn except that it uses the 3d seed file
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject,CreateFrom3dDgn)
    {
    ScopedDgnHost autoDgnHost;

    DgnProjectPtr project;
    DgnDbTestDgnManager::CreateProjectFromDgn (project, DgnDbTestDgnManager::GetOutputFilePath (L"Test3d.idgndb"), DgnDbTestDgnManager::GetSeedFilePath (L"3dMetricGeneral.dgn.v8"));
    ASSERT_TRUE( project != NULL);
    inspectDgnFile (project.get(), /*isReadOnly*/false, false);

    // Make sure we can re-open the project
    project = NULL;
    openProject (project, DgnDbTestDgnManager::GetOutputFilePath (L"Test3d.idgndb"));
    ASSERT_TRUE( project != NULL);
    inspectDgnFile (project.get(),/*isReadOnly*/true, false);
    }

/*---------------------------------------------------------------------------------**//**
* Test to see any name with space can be used as project name
* @bsimethod                                    Majd.Uddin                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DgnProject, CreateProjectNameWithSpace)
    {
    ScopedDgnHost autoDgnHost;

    DgnProjectPtr project;
    DgnDbTestDgnManager::CreateProjectFromDgn (project, DgnDbTestDgnManager::GetOutputFilePath (L"Test name with space.idgndb"), DgnDbTestDgnManager::GetSeedFilePath (L"3dMetricGeneral.dgn.v8"));
    ASSERT_TRUE( project != NULL);
    inspectDgnFile (project.get(), /*isReadOnly*/false, false);

    // Make sure we can re-open the project
    project = NULL;
    openProject (project, DgnDbTestDgnManager::GetOutputFilePath  (L"Test name with space.idgndb"));
    ASSERT_TRUE( project != NULL);
    inspectDgnFile (project.get(),/*isReadOnly*/true, false);
    }
    
// This test checks that creating a bound project from a DGN file seed captures the content of the seed and presents
// a DgnFile interface that can be used according to the DgnFile API to access the content, e.g., models and elements.
TEST(DgnProject,CreateFromDgn)
    {
    ScopedDgnHost autoDgnHost;

    DgnProjectPtr project;
    DgnDbTestDgnManager::CreateProjectFromDgn (project, DgnDbTestDgnManager::GetOutputFilePath (L"2dMetricGeneral.idgndb"), DgnDbTestDgnManager::GetSeedFilePath (L"2dMetricGeneral.dgn.v8"));
    ASSERT_TRUE( project != NULL);
    inspectDgnFile (project.get(), /*isReadOnly*/false, false);

    // Make sure we can re-open the project
    project = NULL;
    openProject (project, DgnDbTestDgnManager::GetOutputFilePath (L"2dMetricGeneral.idgndb"));
    ASSERT_TRUE( project != NULL);
    inspectDgnFile (project.get(),/*isReadOnly*/true, false);
    }
