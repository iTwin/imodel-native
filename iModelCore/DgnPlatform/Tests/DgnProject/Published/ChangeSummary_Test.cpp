/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

// The counts recorded by change summary are quite sensitive to changes in the schema and implementation...
// Turn this on for debugging.
// #define DUMP_CHANGE_SUMMARY 1

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
struct ChangeSummaryTestFixture : ChangeTestFixture
{
DEFINE_T_SUPER(ChangeTestFixture)
protected:
    void ModifyElement(DgnElementId elementId);
    void DeleteElement(DgnElementId elementId);

    void DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label);
    void DumpSqlChanges(DgnDbCR dgnDb, Changes const& sqlChanges, Utf8CP label);

    void GetChangeSummaryFromCurrentTransaction(ChangeSummary& changeSummary);
    void GetChangeSummaryFromSavedTransactions(ChangeSummary& changeSummary);

    bool ChangeSummaryContainsInstance(ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode);

    int GetChangeSummaryInstanceCount(BeSQLite::EC::ChangeSummaryCR changeSummary, Utf8CP qualifiedClassName) const;

public:
    ChangeSummaryTestFixture() {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2015
//---------------------------------------------------------------------------------------
int ChangeSummaryTestFixture::GetChangeSummaryInstanceCount(ChangeSummaryCR changeSummary, Utf8CP qualifiedClassName) const
    {
    Utf8PrintfString ecSql("SELECT COUNT(*) FROM %s WHERE IsChangedInstance(?, ECClassId, ECInstanceId)", qualifiedClassName);

    ECSqlStatement stmt;
    ECSqlStatus ecSqlStatus = stmt.Prepare(*m_db, ecSql.c_str());
    BeAssert(ecSqlStatus.IsSuccess());

    stmt.BindInt64(1, (int64_t) &changeSummary);

    DbResult ecSqlStepStatus = stmt.Step();
    BeAssert(ecSqlStepStatus == BE_SQLITE_ROW);

    return stmt.GetValueInt(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::ModifyElement(DgnElementId elementId)
    {
    RefCountedPtr<PhysicalElement> testElement = m_db->Elements().GetForEdit<PhysicalElement>(elementId);
    BeAssert(testElement.IsValid());

    DgnDbStatus dbStatus;
    testElement->Update(&dbStatus);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DeleteElement(DgnElementId elementId)
    {
    DgnDbStatus dbStatus = m_db->Elements().Delete(elementId);
    BeAssert(dbStatus == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label)
    {
#ifdef DUMP_CHANGE_SUMMARY
    printf("\t%s:\n", label);
    changeSummary.Dump();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    06/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DumpSqlChanges(DgnDbCR dgnDb, Changes const& changes, Utf8CP label)
    {
    printf("%s:\n", label);
    for (Changes::Change change : changes)
        {
        Utf8CP tableName;
        int nCols, indirect;
        DbOpcode opcode;
        DbResult rc = change.GetOperation(&tableName, &nCols, &opcode, &indirect);
        BeAssert(rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);

        bvector<Utf8String> columnNames;
        dgnDb.GetColumns(columnNames, tableName);
        BeAssert((int) columnNames.size() == nCols);

        if (opcode == DbOpcode::Delete || opcode == DbOpcode::Update)
            {
            printf("Old:\n");
            for (int ii = 0; ii < nCols; ii++)
                {
                DbValue v = change.GetValue(ii, Changes::Change::Stage::Old);

                printf("%s=", columnNames[ii].c_str());

                if (!v.IsValid())
                    printf("<invalid>");
                else if (v.IsNull())
                    printf("<null>");
                else
                    printf("%s", v.Format(1000).c_str());

                printf(" ");
                }
            printf("\n");
            }

        if (opcode == DbOpcode::Update || opcode == DbOpcode::Insert)
            {
            printf("New:\n");
            for (int ii = 0; ii < nCols; ii++)
                {
                DbValue v = change.GetValue(ii, Changes::Change::Stage::New);

                printf("%s=", columnNames[ii].c_str());

                if (!v.IsValid())
                    printf("<invalid>");
                else if (v.IsNull())
                    printf("<null>");
                else
                    printf("%s", v.Format(1000).c_str());

                printf(" ");
                }
            printf("\n");
            }
        }

    printf("\n");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    072015
//---------------------------------------------------------------------------------------
bool ChangeSummaryTestFixture::ChangeSummaryContainsInstance(ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode)
    {
    Utf8String tableName = changeSummary.GetInstancesTableName();
    ECClassId classId = m_db->Schemas().GetECClassId(schemaName, className);

    Utf8PrintfString sql("SELECT NULL FROM %s WHERE ClassId=? AND InstanceId=? AND DbOpcode=?", tableName.c_str());
    CachedStatementPtr statement = m_db->GetCachedStatement(sql.c_str());
    BeAssert(statement.IsValid());

    statement->BindId(1, classId);
    statement->BindId(2, instanceId);
    statement->BindInt(3, (int) dbOpcode);

    DbResult result = statement->Step();
    return (result == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::GetChangeSummaryFromCurrentTransaction(ChangeSummary& changeSummary)
    {
    AbortOnConflictChangeSet sqlChangeSet;
    DbResult result = sqlChangeSet.FromChangeTrack(m_db->Txns(), ChangeSet::SetType::Full);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    changeSummary.Free();
    BentleyStatus status = changeSummary.FromChangeSet(sqlChangeSet);
    ASSERT_TRUE(SUCCESS == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::GetChangeSummaryFromSavedTransactions(ChangeSummary& changeSummary)
    {
    BentleyStatus status = m_db->Txns().GetChangeSummary(changeSummary, m_db->Txns().GetSessionStartId());
    ASSERT_TRUE(status == SUCCESS);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChangesFromCurrentTransaction)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ElementChangesFromCurrentTransaction.bim");

    ChangeSummary changeSummary(*m_db);

    m_db->SaveChanges();

    PhysicalModelPtr csModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "ChangeSummaryModel");

    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    DgnElementId elementId = InsertPhysicalElement(*m_db, *csModel, csCategoryId, 0, 0, 0);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserts");

    /*
        ChangeSummary after inserts:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:9;Generic:PhysicalObject:328;Insert;No
                BBoxHigh.X;NULL;0.5
                BBoxHigh.Y;NULL;0.5
                BBoxHigh.Z;NULL;0.5
                BBoxLow.X;NULL;-0.5
                BBoxLow.Y;NULL;-0.5
                BBoxLow.Z;NULL;-0.5
                Category.Id;NULL;1099511627783
                CodeAuthority.Id;NULL;1
                CodeNamespace;NULL;""
                GeometryStream;NULL;...
                InSpatialIndex;NULL;1
                LastMod;NULL;2.45777e+06
                Model.Id;NULL;1099511627782
                Origin.X;NULL;0
                Origin.Y;NULL;0
                Origin.Z;NULL;0
                Pitch;NULL;0
                Roll;NULL;0
                Yaw;NULL;0
        0:8;BisCore:SubCategory:311;Insert;No
                CodeAuthority.Id;NULL;23
                CodeNamespace;NULL;"10000000007"
                CodeValue;NULL;"ChangeSummaryCategory"
                LastMod;NULL;2.45777e+06
                Model.Id;NULL;16
                Parent.Id;NULL;1099511627783
                Properties;NULL;"{"color":16777215}"
        0:6;BisCore:PhysicalPartition:284;Insert;No
                CodeAuthority.Id;NULL;22
                CodeNamespace;NULL;"1"
                CodeValue;NULL;"ChangeSummaryModel"
                LastMod;NULL;2.45777e+06
                Model.Id;NULL;1
                Parent.Id;NULL;1
        0:7;BisCore:SpatialCategory:259;Insert;No
                CodeAuthority.Id;NULL;13
                CodeNamespace;NULL;""
                CodeValue;NULL;"ChangeSummaryCategory"
                Descr;NULL;""
                LastMod;NULL;2.45777e+06
                Model.Id;NULL;16
                Rank;NULL;2
        0:6;BisCore:PhysicalModel:281;Insert;No
                IsTemplate;NULL;0
                ModeledElement.Id;NULL;1099511627782
                ModeledElement.RelECClassId;NULL;BisCore:ModelModelsElement:174
                Properties;NULL;"{"DisplayInfo":{"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"mm","num":1000.0,"sys":2}}}"
                Visibility;NULL;1
        0:9;BisCore:AuthorityIssuesElementCode:169;Insert;No
                SourceECClassId;NULL;BisCore:NullAuthority:274
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;Generic:PhysicalObject:328
                TargetECInstanceId;NULL;0:9
        0:9;BisCore:ModelContainsElements:172;Insert;No
                SourceECClassId;NULL;BisCore:PhysicalModel:281
                SourceECInstanceId;NULL;0:6
                TargetECClassId;NULL;Generic:PhysicalObject:328
                TargetECInstanceId;NULL;0:9
        0:8;BisCore:AuthorityIssuesElementCode:169;Insert;No
                SourceECClassId;NULL;BisCore:ElementScopeAuthority:253
                SourceECInstanceId;NULL;0:23
                TargetECClassId;NULL;BisCore:SubCategory:311
                TargetECInstanceId;NULL;0:8
        0:8;BisCore:ModelContainsElements:172;Insert;No
                SourceECClassId;NULL;BisCore:DictionaryModel:232
                SourceECInstanceId;NULL;0:16
                TargetECClassId;NULL;BisCore:SubCategory:311
                TargetECInstanceId;NULL;0:8
        0:6;BisCore:AuthorityIssuesElementCode:169;Insert;No
                SourceECClassId;NULL;BisCore:ElementScopeAuthority:253
                SourceECInstanceId;NULL;0:22
                TargetECClassId;NULL;BisCore:PhysicalPartition:284
                TargetECInstanceId;NULL;0:6
        0:6;BisCore:ModelContainsElements:172;Insert;No
                SourceECClassId;NULL;BisCore:RepositoryModel:288
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;BisCore:PhysicalPartition:284
                TargetECInstanceId;NULL;0:6
        0:7;BisCore:AuthorityIssuesElementCode:169;Insert;No
                SourceECClassId;NULL;BisCore:DatabaseScopeAuthority:226
                SourceECInstanceId;NULL;0:13
                TargetECClassId;NULL;BisCore:SpatialCategory:259
                TargetECInstanceId;NULL;0:7
        0:7;BisCore:ModelContainsElements:172;Insert;No
                SourceECClassId;NULL;BisCore:DictionaryModel:232
                SourceECInstanceId;NULL;0:16
                TargetECClassId;NULL;BisCore:SpatialCategory:259
                TargetECInstanceId;NULL;0:7
        0:6;BisCore:ModelModelsElement:174;Insert;No
                SourceECClassId;NULL;BisCore:PhysicalModel:281
                SourceECInstanceId;NULL;0:6
                TargetECClassId;NULL;BisCore:PhysicalPartition:284
                TargetECInstanceId;NULL;0:6
        0:9;BisCore:GeometricElement3dIsInCategory:258;Insert;No
                SourceECClassId;NULL;Generic:PhysicalObject:328
                SourceECInstanceId;NULL;0:9
                TargetECClassId;NULL;BisCore:SpatialCategory:259
                TargetECInstanceId;NULL;0:7
    */
    
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(csModel->GetModelId().GetValueUnchecked()), BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalModel, DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(csCategoryId.GetValueUnchecked()), BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialCategory, DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject, DbOpcode::Insert));
    EXPECT_EQ(1, GetChangeSummaryInstanceCount(changeSummary, GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject)));
    EXPECT_EQ(1, GetChangeSummaryInstanceCount(changeSummary, BIS_SCHEMA(BIS_CLASS_SpatialCategory)));
    EXPECT_EQ(4, GetChangeSummaryInstanceCount(changeSummary, BIS_SCHEMA(BIS_REL_ModelContainsElements)));

    m_db->SaveChanges();
    ModifyElement(elementId);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after updates");

    /*
        ChangeSummary after updates:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:9;Generic:PhysicalObject:328;Update;No
                LastMod;2.45777e+06;2.45777e+06
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject, DbOpcode::Update));

    m_db->SaveChanges();
    DeleteElement(elementId);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after deletes");

    /*
        ChangeSummary after deletes:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:9;Generic:PhysicalObject:328;Delete;No
                BBoxHigh.X;0.5;NULL
                BBoxHigh.Y;0.5;NULL
                BBoxHigh.Z;0.5;NULL
                BBoxLow.X;-0.5;NULL
                BBoxLow.Y;-0.5;NULL
                BBoxLow.Z;-0.5;NULL
                Category.Id;1099511627783;NULL
                CodeAuthority.Id;1;NULL
                CodeNamespace;"";NULL
                GeometryStream;...;NULL
                InSpatialIndex;1;NULL
                LastMod;2.45777e+06;NULL
                Model.Id;1099511627782;NULL
                Origin.X;0;NULL
                Origin.Y;0;NULL
                Origin.Z;0;NULL
                Pitch;0;NULL
                Roll;0;NULL
                Yaw;0;NULL
        0:9;BisCore:AuthorityIssuesElementCode:169;Delete;No
                SourceECClassId;BisCore:NullAuthority:274;NULL
                SourceECInstanceId;0:1;NULL
                TargetECClassId;Generic:PhysicalObject:328;NULL
                TargetECInstanceId;0:9;NULL
        0:9;BisCore:ModelContainsElements:172;Delete;No
                SourceECClassId;BisCore:PhysicalModel:281;NULL
                SourceECInstanceId;0:6;NULL
                TargetECClassId;Generic:PhysicalObject:328;NULL
                TargetECInstanceId;0:9;NULL
        0:9;BisCore:GeometricElement3dIsInCategory:258;Delete;Yes
                SourceECClassId;Generic:PhysicalObject:328;NULL
                SourceECInstanceId;0:9;NULL
                TargetECClassId;BisCore:SpatialCategory:259;NULL
                TargetECInstanceId;0:7;NULL
    */
    EXPECT_EQ(4, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject, DbOpcode::Delete));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChangesFromSavedTransactions)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ElementChangesFromSavedTransactions.bim");

    DgnElementId elementId = InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, 0, 0, 0);

    m_db->SaveChanges();

    ChangeSummary changeSummary(*m_db);
    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After inserts");

    /* 
        After inserts:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:6;Generic:PhysicalObject:328;Insert;No
                BBoxHigh.X;NULL;0.5
                BBoxHigh.Y;NULL;0.5
                BBoxHigh.Z;NULL;0.5
                BBoxLow.X;NULL;-0.5
                BBoxLow.Y;NULL;-0.5
                BBoxLow.Z;NULL;-0.5
                Category.Id;NULL;1099511627779
                CodeSpec.Id;NULL;1
                CodeScope;NULL;""
                GeometryStream;NULL;...
                InSpatialIndex;NULL;1
                LastMod;NULL;2.45777e+06
                Model.Id;NULL;1099511627778
                Origin.X;NULL;0
                Origin.Y;NULL;0
                Origin.Z;NULL;0
                Pitch;NULL;0
                Roll;NULL;0
                Yaw;NULL;0
        0:6;BisCore:AuthorityIssuesElementCode:169;Insert;No
                SourceECClassId;NULL;BisCore:NullAuthority:274
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;Generic:PhysicalObject:328
                TargetECInstanceId;NULL;0:6
        0:6;BisCore:ModelContainsElements:172;Insert;No
                SourceECClassId;NULL;BisCore:PhysicalModel:281
                SourceECInstanceId;NULL;0:2
                TargetECClassId;NULL;Generic:PhysicalObject:328
                TargetECInstanceId;NULL;0:6
        0:6;BisCore:GeometricElement3dIsInCategory:258;Insert;No
                SourceECClassId;NULL;Generic:PhysicalObject:328
                SourceECInstanceId;NULL;0:6
                TargetECClassId;NULL;BisCore:SpatialCategory:259
                TargetECInstanceId;NULL;0:3
    */
    EXPECT_EQ(4, changeSummary.MakeInstanceIterator().QueryCount());

    ModifyElement(elementId);

    m_db->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After updates");

    /*
        After updates:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:6;Generic:PhysicalObject:328;Insert;No
                BBoxHigh.X;NULL;0.5
                BBoxHigh.Y;NULL;0.5
                BBoxHigh.Z;NULL;0.5
                BBoxLow.X;NULL;-0.5
                BBoxLow.Y;NULL;-0.5
                BBoxLow.Z;NULL;-0.5
                Category.Id;NULL;1099511627779
                CodeSpec.Id;NULL;1
                CodeScope;NULL;""
                GeometryStream;NULL;...
                InSpatialIndex;NULL;1
                LastMod;NULL;2.45777e+06
                Model.Id;NULL;1099511627778
                Origin.X;NULL;0
                Origin.Y;NULL;0
                Origin.Z;NULL;0
                Pitch;NULL;0
                Roll;NULL;0
                Yaw;NULL;0
        0:6;BisCore:AuthorityIssuesElementCode:169;Insert;No
                SourceECClassId;NULL;BisCore:NullAuthority:274
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;Generic:PhysicalObject:328
                TargetECInstanceId;NULL;0:6
        0:6;BisCore:ModelContainsElements:172;Insert;No
                SourceECClassId;NULL;BisCore:PhysicalModel:281
                SourceECInstanceId;NULL;0:2
                TargetECClassId;NULL;Generic:PhysicalObject:328
                TargetECInstanceId;NULL;0:6
        0:6;BisCore:GeometricElement3dIsInCategory:258;Insert;No
                SourceECClassId;NULL;Generic:PhysicalObject:328
                SourceECInstanceId;NULL;0:6
                TargetECClassId;NULL;BisCore:SpatialCategory:259
                TargetECInstanceId;NULL;0:3
    */
    EXPECT_EQ(4, changeSummary.MakeInstanceIterator().QueryCount());

    DeleteElement(elementId);

    m_db->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After deletes");
    EXPECT_EQ(0, changeSummary.MakeInstanceIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ValidateInstanceIterator)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ValidateInstanceIterator.bim");

    PhysicalModelPtr csModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "ChangeSummaryModel");
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");
    DgnElementId elementId = InsertPhysicalElement(*m_db, *csModel, csCategoryId, 0, 0, 0);

    ChangeSummary changeSummary(*m_db);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    int countIter = 0;
    for (ChangeSummary::InstanceIterator::const_iterator const& entry : changeSummary.MakeInstanceIterator())
        {
        countIter++;
        UNUSED_VARIABLE(entry);
        }

    int countQuery = changeSummary.MakeInstanceIterator().QueryCount();
    EXPECT_EQ(countQuery, countIter);
    EXPECT_NE(countQuery, 0);
    }

extern ECSchemaPtr ReadECSchemaFromDisk(WCharCP schemaPathname);
extern bool ImportECSchema(ECSchemaR ecSchema, DgnDbR project);
extern IECInstancePtr CreateStartupCompanyInstance(ECSchemaCR startupSchema);

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromCurrentTransaction)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"RelationshipChangesFromCurrentTransaction.bim");

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_db);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    DbResult stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    ChangeSummary changeSummary(*m_db);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    /*
        ChangeSummary after inserting instances:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:1;StartupCompany:Employee:1099511627843;Insert;No
                FirstName;NULL;"John"
                LastName;NULL;"Doe"
        0:2;StartupCompany:Company:1099511627836;Insert;No
                Name;NULL;"AcmeWorks"
        0:3;StartupCompany:Company:1099511627836;Insert;No
                Name;NULL;"CmeaWorks"
        0:4;StartupCompany:Hardware:1099511627840;Insert;No
                Make;NULL;"Tesla"
                Model;NULL;"Model-S"
        0:5;StartupCompany:Hardware:1099511627840;Insert;No
                Make;NULL;"Toyota"
                Model;NULL;"Prius"
    */
    EXPECT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    m_db->SaveChanges();

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindId(3, companyKey1.GetECClassId());
    statement.BindId(4, companyKey1.GetECInstanceId());

    ECInstanceKey employeeCompanyKey;
    stepStatus = statement.Step(employeeCompanyKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindId(3, hardwareKey1.GetECClassId());
    statement.BindId(4, hardwareKey1.GetECInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    /*
        ChangeSummary after inserting relationships:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:1;StartupCompany:EmployeeCompany:1099511627845;Insert;No
                SourceECClassId;NULL;StartupCompany:Employee:1099511627843
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;StartupCompany:Company:1099511627836
                TargetECInstanceId;NULL;0:2
        0:6;StartupCompany:EmployeeHardware:1099511627847;Insert;No
                SourceECClassId;NULL;StartupCompany:Employee:1099511627843
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;StartupCompany:Hardware:1099511627840
                TargetECInstanceId;NULL;0:4
    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    m_db->SaveChanges();

    /* 
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(*m_db, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "DELETE FROM StartupCompany.EmployeeCompany WHERE EmployeeCompany.ECInstanceId=?");
    statement.BindId(1, employeeCompanyKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindId(3, hardwareKey2.GetECClassId());
    statement.BindId(4, hardwareKey2.GetECInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_db, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindId(3, companyKey2.GetECClassId());
    statement.BindId(4, companyKey2.GetECInstanceId());

    ECInstanceKey employeeCompanyKey2;
    stepStatus = statement.Step(employeeCompanyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    /*
        ChangeSummary after updating (deleting and inserting different) relationships:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:6;StartupCompany:EmployeeHardware:1099511627847;Delete;No
                SourceECClassId;StartupCompany:Employee:1099511627843;NULL
                SourceECInstanceId;0:1;NULL
                TargetECClassId;StartupCompany:Hardware:1099511627840;NULL
                TargetECInstanceId;0:4;NULL
        0:7;StartupCompany:EmployeeHardware:1099511627847;Insert;No
                SourceECClassId;NULL;StartupCompany:Employee:1099511627843
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;StartupCompany:Hardware:1099511627840
                TargetECInstanceId;NULL;0:5
        0:1;StartupCompany:EmployeeCompany:1099511627845;Update;No
                SourceECClassId;StartupCompany:Employee:1099511627843;StartupCompany:Employee:1099511627843
                SourceECInstanceId;0:1;0:1
                TargetECClassId;StartupCompany:Company:1099511627836;StartupCompany:Company:1099511627836
                TargetECInstanceId;0:2;0:3
    */
    EXPECT_EQ(3, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Update));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Delete));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChildRelationshipChanges)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ElementChildRelationshipChanges.bim");

    PhysicalModelPtr csModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "ChangeSummaryModel");
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    DgnElementId parentElementId = InsertPhysicalElement(*m_db, *csModel, csCategoryId, 0, 0, 0);
    DgnElementId childElementId = InsertPhysicalElement(*m_db, *csModel, csCategoryId, 1, 1, 1);
    DgnClassId parentRelClassId = m_db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);

    m_db->SaveChanges();

    RefCountedPtr<DgnElement> childElementPtr = m_db->Elements().GetForEdit<DgnElement>(childElementId);
    
    DgnDbStatus dbStatus = childElementPtr->SetParentId(parentElementId, parentRelClassId);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);
    
    childElementPtr->Update(&dbStatus);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);

    ChangeSummary changeSummary(*m_db);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after setting ParentId");

    /*
        ChangeSummary after setting ParentId:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:10;Generic:PhysicalObject:328;Update;No
                LastMod;2.45777e+06;2.45777e+06
                Parent.Id;NULL;1099511627785
                Parent.RelECClassId;NULL;BisCore:ElementOwnsChildElements:175
        0:10;BisCore:ElementOwnsChildElements:175;Insert;No
                SourceECClassId;NULL;Generic:PhysicalObject:328
                SourceECInstanceId;NULL;0:9
                TargetECClassId;NULL;Generic:PhysicalObject:328
                TargetECInstanceId;NULL;0:10

    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_EQ(1, GetChangeSummaryInstanceCount(changeSummary, GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject)));
    EXPECT_EQ(1, GetChangeSummaryInstanceCount(changeSummary, BIS_SCHEMA(BIS_REL_ElementOwnsChildElements))); // NEEDS_WORK: (Shaun?) This should really be 0 - likely issue with setting parent rel class ids (Shaun)
    EXPECT_EQ(0, GetChangeSummaryInstanceCount(changeSummary, BIS_SCHEMA(BIS_REL_PhysicalElementAssemblesElements))); // NEEDS_WORK: (Shaun?) This should really be 1 - likely issue with setting parent rel class ids (Shaun)
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements, DbOpcode::Insert)); // Captured due to change of FK relationship (ParentId column)
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject, DbOpcode::Update)); // Captured due to change of ParentId property

    ECClassId relClassId = m_db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
    ECClassId elClassId = m_db->Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);

    ChangeSummary::Instance instance = changeSummary.GetInstance(elClassId, ECInstanceId(childElementId.GetValue()));
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary::Instance relInstance = changeSummary.GetInstance(relClassId, ECInstanceId(childElementId.GetValue()));
    ASSERT_TRUE(relInstance.IsValid());

    DbDupValue value(nullptr);

    value = relInstance.GetNewValue("SourceECClassId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(elClassId.GetValue(), value.GetValueId<ECClassId>().GetValue());
    EXPECT_EQ(elClassId.GetValue(), relInstance.GetNewValue("SourceECClassId").GetValueId<ECClassId>().GetValue());

    value = relInstance.GetNewValue("SourceECInstanceId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(parentElementId.GetValueUnchecked(), value.GetValueUInt64());
    EXPECT_EQ(parentElementId.GetValueUnchecked(), relInstance.GetNewValue("SourceECInstanceId").GetValueUInt64());

    value = relInstance.GetNewValue("TargetECClassId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(elClassId.GetValue(), value.GetValueId<ECClassId>().GetValue());
    EXPECT_EQ(elClassId.GetValue(), relInstance.GetNewValue("TargetECClassId").GetValueId<ECClassId>().GetValue());

    value = relInstance.GetNewValue("TargetECInstanceId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(childElementId.GetValueUnchecked(), value.GetValueUInt64());
    ASSERT_EQ(childElementId.GetValueUnchecked(), relInstance.GetNewValue("TargetECInstanceId").GetValueUInt64());

    value = instance.GetNewValue("Parent.Id");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), value.GetValueUInt64());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), instance.GetNewValue("Parent.Id").GetValueUInt64());

    EXPECT_EQ(4, relInstance.MakeValueIterator().QueryCount());
    EXPECT_EQ(3, instance.MakeValueIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, QueryChangedElements)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"QueryChangedElements.bim");
    m_db->SaveChanges();

    ChangeSummary changeSummary(*m_db);

    bset<DgnElementId> insertedElements;
    for (int ii = 0; ii < 10; ii++)
        {
        DgnElementId elementId = InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, ii, 0, 0);
        insertedElements.insert(elementId);
        }

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    // Query changed elements directly using ECSQL
    ECSqlStatement stmt;
    Utf8CP ecsql = "SELECT el.ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_PhysicalElement) " el WHERE IsChangedInstance(?, el.ECClassId, el.ECInstanceId)";
    ECSqlStatus status = stmt.Prepare(*m_db, ecsql);
    ASSERT_TRUE(status.IsSuccess());
    
    stmt.BindInt64(1, (int64_t) &changeSummary);

    bset<DgnElementId> changedElements;
    DbResult stepStatus;
    while ((stepStatus = stmt.Step()) == BE_SQLITE_ROW)
        {
        changedElements.insert(stmt.GetValueId<DgnElementId>(0));
        }
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);
    EXPECT_EQ(insertedElements, changedElements);

    stmt.Finalize();
    
    // Query changed elements directly using the API
    bmap<ECInstanceId, ChangeSummary::Instance> changes;
    ECClassId elClassId = m_db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, "Element");
    changeSummary.QueryByClass(changes, elClassId, true, ChangeSummary::QueryDbOpcode::All);
    changedElements.empty();
    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        changedElements.insert(DgnElementId(iter->first.GetValueUnchecked()));
        }
    EXPECT_EQ(insertedElements.size(), changedElements.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, QueryMultipleSessions)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"QueryMultipleSessions.bim");
    
    int nSessions = 5;
    int nTransactionsPerSession = 5;
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    for (int ii = 0; ii < nSessions; ii++)
        {
        OpenDgnDb(fileName);

        for (int jj = 0; jj < nTransactionsPerSession; jj++)
            {
            InsertPhysicalElement(*m_db, *m_defaultModel, m_defaultCategoryId, ii, jj, 0);
            m_db->SaveChanges();
            }

        CloseDgnDb();
        }

    OpenDgnDb(fileName);

    TxnManager::TxnId startTxnId(TxnManager::SessionId(1), 0); // First session, First transaction
    ChangeSummary changeSummary(*m_db);
    BentleyStatus status = m_db->Txns().GetChangeSummary(changeSummary, startTxnId);
    ASSERT_TRUE(status == SUCCESS);

    //printf("\t%s:\n", "ChangeSummary after multiple sessions");
    //changeSummary.Dump();

    int expectedPhysicalObjects = nSessions * nTransactionsPerSession;
    EXPECT_EQ(expectedPhysicalObjects, GetChangeSummaryInstanceCount(changeSummary, GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject)));
    }

