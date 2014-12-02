/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ECDbInstances_Test_Old.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnCore/ColorUtil.h>
#if defined (DGNPLATFORM_HAVE_DGN_IMPORTER)
    #include <DgnPlatform/ForeignFormat/DgnProjectImporter.h>
#endif
#include <Logging/bentleylogging.h>
#include <DgnPlatform/DgnHandlers/DgnLinkTable.h>

#define LOG (*NativeLogging::LoggingManager::GetLogger (L"DgnECDb"))

#if defined (_MSC_VER)
#pragma warning (disable:4702)
#endif

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE 
USING_NAMESPACE_BENTLEY_SQLITE_EC

extern Int64 GetV9ElementId (Utf8CP v8Filename, Int64 v8ElementId, DgnProjectR project);

extern ElementId AddLineToModel (DgnModelR model);
extern ECInstanceId AddDgnLink (DgnProjectR project, ElementId const& elementId, int ordinal, Utf8CP displayLabel, Utf8CP url = nullptr);

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   04/2013
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInstances_Old, SelectDgnLinksForDgnElement)
    {
    ScopedDgnHost host;

    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    auto& project = *tdm.GetDgnProjectP();

    auto model = tdm.GetDgnModelP ();

    auto elementId = AddLineToModel (*model);
    auto linkId = AddDgnLink (project, elementId, 1, "Non-URL link");
    auto urlLinkId = AddDgnLink (project, elementId, 2, "URL link", "http://www.foo.com");

    ECSchemaP dgnschema = nullptr;
    auto schemaStat = project.GetEC().GetSchemaManager ().GetECSchema (dgnschema, DGNECSCHEMA_SchemaName);
    ASSERT_EQ (SUCCESS, schemaStat);
    auto elementClass = dgnschema->GetClassCP (DGNECSCHEMA_CLASSNAME_Element);
    ASSERT_TRUE (elementClass != nullptr);
    auto dgnLinkClass = dgnschema->GetClassCP (DGNECSCHEMA_CLASSNAME_DgnLink);
    ASSERT_TRUE (dgnLinkClass != nullptr);
    auto elementHasLinksRelClass = dgnschema->GetClassCP (DGNECSCHEMA_CLASSNAME_ElementHasLinks)->GetRelationshipClassCP ();
    ASSERT_TRUE (elementHasLinksRelClass != nullptr);

    //non-polymorphic query (only for DgnLink objects)
        {
        ECSqlSelectBuilder ecsql;
        ecsql.Select ("l.ECInstanceId").From (*dgnLinkClass, "l", false).Join (*elementClass, "e", false).Using (*elementHasLinksRelClass).Where ("e.ECInstanceId = ?");

        ECSqlStatement statement;
        auto stat = statement.Prepare (project, ecsql.ToString ().c_str ());
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        statement.BindInt64 (1, elementId.GetValue ());

        int count = 0;
        while (statement.Step () == ECSqlStepStatus::HasRow)
            {
            count++;
            ASSERT_EQ (linkId.GetValue (), statement.GetValueInt64 (0));
            }
        ASSERT_EQ (1, count);
        }

     //polymorphic query (for DgnLink and subclasses)
        {
        ECSqlSelectBuilder ecsql;
        ecsql.Select ("l.ECInstanceId").From (*dgnLinkClass, "l", true).Join (*elementClass, "e", false).Using (*elementHasLinksRelClass).Where ("e.ECInstanceId = ?").OrderBy ("e.ECInstanceId");

        ECSqlStatement statement;
        auto stat = statement.Prepare (project, ecsql.ToString ().c_str ());
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stat);

        statement.BindInt64 (1, elementId.GetValue ());

        int count = 0;
        while (statement.Step () == ECSqlStepStatus::HasRow)
            {
            count++;
            if (count == 1)
                {
                ASSERT_EQ (linkId.GetValue(), statement.GetValueInt64 (0));
                }
            else if (count == 2)
                {
                ASSERT_EQ (urlLinkId.GetValue(), statement.GetValueInt64 (0));
                }
            }

        ASSERT_EQ (2, count);
        }
    }



