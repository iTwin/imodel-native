/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/BackwardsCompatibility_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
#if defined (WIP_COMPATIBILITY)
/*---------------------------------------------------------------------------------**//**
* @bsiclass                                Adeel.Shoukat
+---------------+---------------+---------------+---------------+---------------+------*/
struct BackwardsCompatibilityTests : public testing::Test
{
    BackwardsCompatibilityTests()
        {
        
        }
        void verifyElements(DgnFileP dgnFile,DgnDbR project,BeFileName name )
            {
             bset<ElementId> elements;
            // FileOpenMode f = 2;

        dgnFile->FillDictionaryModel();
        FOR_EACH(PersistentDgnElementP ref, dgnFile->GetDictionaryModel()->GetElementsCollection())
            elements.insert(ref->GetElementId());

        FOR_EACH(DgnModelTable::Iterator::Entry const& entry, dgnFile->MakeModelIterator(MODEL_ITERATE_All))
        {
            DgnModelP model = dgnFile->LoadModelById(entry.GetModelId());

            FOR_EACH(PersistentDgnElementP ref, model->GetElementsCollection())
                elements.insert(ref->GetElementId());
        }

        ECN::ECSchemaP dgnschema = NULL;
        SchemaManagerStatus schemaStat = project.Schemas().GetECSchema(dgnschema, "dgn");
        ASSERT_EQ(SCHEMAMANAGER_Success, schemaStat);
        ECN::ECClassP elementClass = dgnschema->GetClassP(L"Element");
        ASSERT_TRUE(elementClass != NULL);

        ECSqlSelectBuilder builder;
        builder.SelectAll().From(*elementClass);
        ECDbStatement ecDbStatement;
        DbResult result = ecDbStatement.Prepare(project, builder);
        ASSERT_EQ(BE_SQLITE_OK, result);

        bset<ElementId> elementInstances;
        try{
            ECDbInstanceAdapter adapter(ecDbStatement, *elementClass);
            while (BE_SQLITE_ROW == ecDbStatement.Step())
            {
                ECId id;
                bool status = adapter.GetInstanceId(id);
                ASSERT_TRUE(status);
               //  dgnFile->isGraphicElement(ElementId(id));
                BeTest::SetThrowOnAssert(false);// just to ignore few element with no 
                DgnElementP ref = dgnFile->GetElementById(ElementId(id)).get();
               BeTest::SetThrowOnAssert(true);
                if(ref!=NULL)
                elementInstances.insert(ElementId(id));
            }
            WString elementsStr;
           // int n=0;
            FOR_EACH(ElementId id, elements)         { elementsStr.append(WPrintfString(L"%lld ", id));}
            WString elementInstancesStr;
            FOR_EACH(ElementId id, elementInstances) { elementInstancesStr.append(WPrintfString(L"%lld ", id)); }
            ASSERT_STREQ(elementsStr.c_str(), elementInstancesStr.c_str());
        }
        catch(...)
        {
            EXPECT_FALSE(true)<<name.GetName();
        }

      }
};

/*---------------------------------------------------------------------------------------
* Open Graphite04 files in current tree
* @bsimethod                                                    Adeel.Shoukat   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BackwardsCompatibilityTests, openGraphite05In0501)
    {
     ScopedDgnHost host;
    BeFileName fullFileName (getenv("CompatibilityRoot"));
    fullFileName.AppendToPath(L"src");
    fullFileName.AppendToPath(L"05");
    fullFileName.AppendToPath(L"*.idgndb");
    BeFileListIterator fileIterator(fullFileName,false);
    BeFileName name;
    while (fileIterator.GetNextFileName(name) != ERROR)
        {
        BeFileName outFullFileName(getenv ("CompatibilityRoot"));
        outFullFileName.AppendToPath(L"out");
        outFullFileName.AppendToPath(L"03");
        outFullFileName.AppendToPath(L"05");
        outFullFileName.AppendToPath(BeFileName::GetFileNameAndExtension(name).c_str());
        BeFileName::CreateNewDirectory(BeFileName::GetDirectoryName(outFullFileName).c_str());
        printf("file - > %S \n",name.GetName());
        if (BeFileName::BeCopyFile(name, outFullFileName) != BE_FILENAME_STATUS_Success)
            {
            ASSERT_FALSE(true)<<"Cannot Open "<<name;
            }
        TestDataManager tdm(outFullFileName, OPENMODE_READWRITE, true);
        DgnFileP dgnFile = tdm.GetLoadedDgnPtr();
        DgnDbR project = dgnFile->GetDgnDb();
        verifyElements(dgnFile,project,name);
        }
    } 
/*---------------------------------------------------------------------------------------
* Open Graphite04 files in current tree
* @bsimethod                                                    Adeel.Shoukat   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/    
    TEST_F(BackwardsCompatibilityTests, openGraphite04In0501)
    {
     ScopedDgnHost host;
    BeFileName fullFileName (getenv("CompatibilityRoot"));
    fullFileName.AppendToPath(L"src");
    fullFileName.AppendToPath(L"04");
    fullFileName.AppendToPath(L"*.idgndb");
    BeFileListIterator fileIterator(fullFileName,false);
    BeFileName name;
    while (fileIterator.GetNextFileName(name) != ERROR)
        {
        BeFileName outFullFileName(getenv ("CompatibilityRoot"));
        outFullFileName.AppendToPath(L"out");
        outFullFileName.AppendToPath(L"03");
        outFullFileName.AppendToPath(L"04");
        outFullFileName.AppendToPath(BeFileName::GetFileNameAndExtension(name).c_str());
        BeFileName::CreateNewDirectory(BeFileName::GetDirectoryName(outFullFileName).c_str());
        printf("file - > %S \n",name.GetName());
        if (BeFileName::BeCopyFile(name, outFullFileName) != BE_FILENAME_STATUS_Success)
            {
            ASSERT_FALSE(true)<<"Cannot Open "<<name;
            }
        TestDataManager tdm(outFullFileName, OPENMODE_READWRITE, true);
        DgnFileP dgnFile = tdm.GetLoadedDgnPtr();
        DgnDbR project = dgnFile->GetDgnDb();
        verifyElements(dgnFile,project,name);
        }
    }     
#endif