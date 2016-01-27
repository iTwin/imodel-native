/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    BentleyStatus ImportECInstance(ECInstanceKey& instanceKey, IECInstanceR instance, DgnDbR dgndb);

    int GetChangeSummaryInstanceCount(BeSQLite::EC::ChangeSummaryCR changeSummary, Utf8CP qualifiedClassName) const;

public:
    ChangeSummaryTestFixture() : T_Super(L"ChangeSummaryTest.dgndb") {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/2015
//---------------------------------------------------------------------------------------
int ChangeSummaryTestFixture::GetChangeSummaryInstanceCount(ChangeSummaryCR changeSummary, Utf8CP qualifiedClassName) const
    {
    Utf8PrintfString ecSql("SELECT COUNT(*) FROM %s WHERE IsChangedInstance(?, GetECClassId(), ECInstanceId)", qualifiedClassName);

    ECSqlStatement stmt;
    ECSqlStatus ecSqlStatus = stmt.Prepare(*m_testDb, ecSql.c_str());
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
    RefCountedPtr<PhysicalElement> testElement = m_testDb->Elements().GetForEdit<PhysicalElement>(elementId);
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
    DgnDbStatus dbStatus = m_testDb->Elements().Delete(elementId);
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
    ECClassId classId = m_testDb->Schemas().GetECClassId(schemaName, className);

    Utf8PrintfString sql("SELECT NULL FROM %s WHERE ClassId=? AND InstanceId=? AND DbOpcode=?", tableName.c_str());
    CachedStatementPtr statement = m_testDb->GetCachedStatement(sql.c_str());
    BeAssert(statement.IsValid());

    statement->BindInt64(1, classId);
    statement->BindId(2, instanceId);
    statement->BindInt(3, (int) dbOpcode);

    DbResult result = statement->Step();
    return (result == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryTestFixture::ImportECInstance(ECInstanceKey& instanceKey, IECInstanceR instance, DgnDbR dgndb)
    {
    ECClassCR ecClass = instance.GetClass();
    ECInstanceInserter inserter(dgndb, ecClass);
    if (!inserter.IsValid())
        return ERROR;

    return inserter.Insert(instanceKey, instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::GetChangeSummaryFromCurrentTransaction(ChangeSummary& changeSummary)
    {
    AbortOnConflictChangeSet sqlChangeSet;
    DbResult result = sqlChangeSet.FromChangeTrack(m_testDb->Txns(), ChangeSet::SetType::Full);
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
    DgnDbStatus status = m_testDb->Txns().GetChangeSummary(changeSummary, m_testDb->Txns().GetSessionStartId());
    ASSERT_TRUE(status == DgnDbStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChangesFromCurrentTransaction)
    {
    CreateDgnDb();

    ChangeSummary changeSummary(*m_testDb);

    m_testDb->SaveChanges();
    
    DgnModelId csModelId = InsertSpatialModel("ChangeSummaryModel");
    SpatialModelPtr csModel = m_testDb->Models().Get<SpatialModel>(csModelId);
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    DgnElementId elementId = InsertPhysicalElement(*csModel, csCategoryId, 0, 0, 0);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserts");

    /*
	ChangeSummary after inserts:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
        ChangeSummary after inserts:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:1;dgn:SpatialModel:208;Insert;No
                Code.AuthorityId;NULL;6
                Code.Namespace;NULL;""
                Code.Value;NULL;"ChangeSetModel"
                DependencyIndex;NULL;-1
                Properties;NULL;"{"DisplayInfo":{"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}}"
                Visibility;NULL;1
        0:2;dgn:Category:133;Insert;No
                Code.AuthorityId;NULL;3
                Code.Namespace;NULL;""
                Code.Value;NULL;"ChangeSetTestCategory"
                Descr;NULL;""
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1
                Rank;NULL;2
                Scope;NULL;1
        0:3;dgn:SubCategory:215;Insert;No
                Code.AuthorityId;NULL;3
                Code.Namespace;NULL;"10000000002"
                Code.Value;NULL;"ChangeSetTestCategory"
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1
                ParentId;NULL;1099511627778
                Props;NULL;"{"color":16777215}"
        0:4;dgn:PhysicalElement:195;Insert;No
                BBoxHigh.X;NULL;0.5
                BBoxHigh.Y;NULL;0.5
                BBoxHigh.Z;NULL;0.5
                BBoxLow.X;NULL;-0.5
                BBoxLow.Y;NULL;-0.5
                BBoxLow.Z;NULL;-0.5
                CategoryId;NULL;1099511627778
                Code.AuthorityId;NULL;1
                Code.Namespace;NULL;""
                Geometry;NULL;...
                InPhysicalSpace;NULL;1
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1099511627777
                Origin.X;NULL;0
                Origin.Y;NULL;0
                Origin.Z;NULL;0
                Pitch;NULL;0
                Roll;NULL;0
                Yaw;NULL;0
        0:2;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:DictionaryModel:168
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:Category:133
                TargetECInstanceId;NULL;0:2
        0:3;dgn:ElementOwnsChildElements:179;Insert;No
                SourceECClassId;NULL;dgn:Category:133
                SourceECInstanceId;NULL;0:2
                TargetECClassId;NULL;dgn:SubCategory:215
                TargetECInstanceId;NULL;0:3
        0:3;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:DictionaryModel:168
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:SubCategory:215
                TargetECInstanceId;NULL;0:3
        0:4;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:SpatialModel:208
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:PhysicalElement:195
                TargetECInstanceId;NULL;0:4
        0:4;dgn:SpatialElementIsInCategory:205;Insert;No
                SourceECClassId;NULL;dgn:PhysicalElement:195
                SourceECInstanceId;NULL;0:4
                TargetECClassId;NULL;dgn:Category:133
                TargetECInstanceId;NULL;0:2
    */
    EXPECT_EQ(9, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(csModelId.GetValueUnchecked()), DGN_ECSCHEMA_NAME, DGN_CLASSNAME_SpatialModel, DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(m_testCategoryId.GetValueUnchecked()), DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Category, DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, DbOpcode::Insert));

    m_testDb->SaveChanges();
    ModifyElement(elementId);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after updates");

    /*
       ChangeSummary after updates:
       BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
               AccessString;OldValue;NewValue
       0:4;dgn:PhysicalElement:195;Update;No
               LastMod;2.4574e+006;2.4574e+006
               Origin.X;0;0
               Origin.Y;0;0
               Origin.Z;0;0
               Pitch;0;0
               Roll;0;0
               Yaw;0;0
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, DbOpcode::Update));

    m_testDb->SaveChanges();
    DeleteElement(elementId);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after deletes");

    /*
        ChangeSummary after deletes:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:4;dgn:PhysicalElement:195;Delete;No
                BBoxHigh.X;0.5;NULL
                BBoxHigh.Y;0.5;NULL
                BBoxHigh.Z;0.5;NULL
                BBoxLow.X;-0.5;NULL
                BBoxLow.Y;-0.5;NULL
                BBoxLow.Z;-0.5;NULL
                CategoryId;1099511627778;NULL
                Code.AuthorityId;1;NULL
                Code.Namespace;"";NULL
                Geometry;...;NULL
                InPhysicalSpace;1;NULL
                LastMod;2.4574e+006;NULL
                ModelId;1099511627777;NULL
                Origin.X;0;NULL
                Origin.Y;0;NULL
                Origin.Z;0;NULL
                Pitch;0;NULL
                Roll;0;NULL
                Yaw;0;NULL
        0:4;dgn:ModelContainsElements:193;Delete;No
                SourceECClassId;dgn:SpatialModel:208;NULL
                SourceECInstanceId;0:1;NULL
                TargetECClassId;dgn:PhysicalElement:195;NULL
                TargetECInstanceId;0:4;NULL
        0:4;dgn:SpatialElementIsInCategory:205;Delete;Yes
                SourceECClassId;dgn:PhysicalElement:195;NULL
                SourceECInstanceId;0:4;NULL
                TargetECClassId;dgn:Category:133;NULL
                TargetECInstanceId;0:2;NULL
    */
    EXPECT_EQ(3, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(elementId.GetValueUnchecked()), DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, DbOpcode::Delete));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChangesFromSavedTransactions)
    {
    CreateDgnDb();

    ChangeSummary changeSummary(*m_testDb);

    DgnModelId csModelId = InsertSpatialModel("ChangeSummaryModel");
    SpatialModelPtr csModel = m_testDb->Models().Get<SpatialModel>(csModelId);
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    DgnElementId elementId = InsertPhysicalElement(*csModel, csCategoryId, 0, 0, 0);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After inserts");

    /* 
        After inserts:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:1;dgn:SpatialModel:208;Insert;No
                Code.AuthorityId;NULL;6
                Code.Namespace;NULL;""
                Code.Value;NULL;"ChangeSetModel"
                DependencyIndex;NULL;-1
                Properties;NULL;"{"DisplayInfo":{"fmtDir":0.0,"fmtFlags":{"angMo
de":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0}
,"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"r
ndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}}"
                Visibility;NULL;1
        0:2;dgn:Category:133;Insert;No
                Code.AuthorityId;NULL;3
                Code.Namespace;NULL;""
                Code.Value;NULL;"ChangeSetTestCategory"
                Descr;NULL;""
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1
                Rank;NULL;2
                Scope;NULL;1
        0:3;dgn:SubCategory:215;Insert;No
                Code.AuthorityId;NULL;3
                Code.Namespace;NULL;"10000000002"
                Code.Value;NULL;"ChangeSetTestCategory"
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1
                ParentId;NULL;1099511627778
                Props;NULL;"{"color":16777215}"
        0:4;dgn:PhysicalElement:195;Insert;No
                BBoxHigh.X;NULL;0.5
                BBoxHigh.Y;NULL;0.5
                BBoxHigh.Z;NULL;0.5
                BBoxLow.X;NULL;-0.5
                BBoxLow.Y;NULL;-0.5
                BBoxLow.Z;NULL;-0.5
                CategoryId;NULL;1099511627778
                Code.AuthorityId;NULL;1
                Code.Namespace;NULL;""
                Geometry;NULL;...
                InPhysicalSpace;NULL;1
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1099511627777
                Origin.X;NULL;0
                Origin.Y;NULL;0
                Origin.Z;NULL;0
                Pitch;NULL;0
                Roll;NULL;0
                Yaw;NULL;0
        0:2;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:DictionaryModel:168
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:Category:133
                TargetECInstanceId;NULL;0:2
        0:3;dgn:ElementOwnsChildElements:179;Insert;No
                SourceECClassId;NULL;dgn:Category:133
                SourceECInstanceId;NULL;0:2
                TargetECClassId;NULL;dgn:SubCategory:215
                TargetECInstanceId;NULL;0:3
        0:3;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:DictionaryModel:168
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:SubCategory:215
                TargetECInstanceId;NULL;0:3
        0:4;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:SpatialModel:208
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:PhysicalElement:195
                TargetECInstanceId;NULL;0:4
        0:4;dgn:SpatialElementIsInCategory:205;Insert;No
                SourceECClassId;NULL;dgn:PhysicalElement:195
                SourceECInstanceId;NULL;0:4
                TargetECClassId;NULL;dgn:Category:133
                TargetECInstanceId;NULL;0:2
    */
    EXPECT_EQ(9, changeSummary.MakeInstanceIterator().QueryCount());

    ModifyElement(elementId);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After updates");

    /*
        After updates:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:1;dgn:SpatialModel:208;Insert;No
                Code.AuthorityId;NULL;6
                Code.Namespace;NULL;""
                Code.Value;NULL;"ChangeSetModel"
                DependencyIndex;NULL;-1
                Properties;NULL;"{"DisplayInfo":{"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}}"
                Visibility;NULL;1
        0:2;dgn:Category:133;Insert;No
                Code.AuthorityId;NULL;3
                Code.Namespace;NULL;""
                Code.Value;NULL;"ChangeSetTestCategory"
                Descr;NULL;""
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1
                Rank;NULL;2
                Scope;NULL;1
        0:3;dgn:SubCategory:215;Insert;No
                Code.AuthorityId;NULL;3
                Code.Namespace;NULL;"10000000002"
                Code.Value;NULL;"ChangeSetTestCategory"
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1
                ParentId;NULL;1099511627778
                Props;NULL;"{"color":16777215}"
        0:4;dgn:PhysicalElement:195;Insert;No
                BBoxHigh.X;NULL;0.5
                BBoxHigh.Y;NULL;0.5
                BBoxHigh.Z;NULL;0.5
                BBoxLow.X;NULL;-0.5
                BBoxLow.Y;NULL;-0.5
                BBoxLow.Z;NULL;-0.5
                CategoryId;NULL;1099511627778
                Code.AuthorityId;NULL;1
                Code.Namespace;NULL;""
                Geometry;NULL;...
                InPhysicalSpace;NULL;1
                LastMod;NULL;2.4574e+006
                ModelId;NULL;1099511627777
                Origin.X;NULL;0
                Origin.Y;NULL;0
                Origin.Z;NULL;0
                Pitch;NULL;0
                Roll;NULL;0
                Yaw;NULL;0
        0:2;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:DictionaryModel:168
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:Category:133
                TargetECInstanceId;NULL;0:2
        0:3;dgn:ElementOwnsChildElements:179;Insert;No
                SourceECClassId;NULL;dgn:Category:133
                SourceECInstanceId;NULL;0:2
                TargetECClassId;NULL;dgn:SubCategory:215
                TargetECInstanceId;NULL;0:3
        0:3;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:DictionaryModel:168
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:SubCategory:215
                TargetECInstanceId;NULL;0:3
        0:4;dgn:ModelContainsElements:193;Insert;No
                SourceECClassId;NULL;dgn:SpatialModel:208
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;dgn:PhysicalElement:195
                TargetECInstanceId;NULL;0:4
        0:4;dgn:SpatialElementIsInCategory:205;Insert;No
                SourceECClassId;NULL;dgn:PhysicalElement:195
                SourceECInstanceId;NULL;0:4
                TargetECClassId;NULL;dgn:Category:133
                TargetECInstanceId;NULL;0:2
    */
    EXPECT_EQ(9, changeSummary.MakeInstanceIterator().QueryCount());

    DeleteElement(elementId);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "After deletes");

    /*
	After deletes:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;dgn:SpatialModel:196;Insert;No
		Code.AuthorityId;NULL;6
		Code.Namespace;NULL;""
		Code.Value;NULL;"ChangeSetModel"
		DependencyIndex;NULL;-1
		Properties;NULL;"{"DisplayInfo":{"azimuth":-9.2559631349317831e+061,"fmtDir":0.0,"fmtFlags":{"angMode":0,"angPrec":0,"clockwise":0,"dirMode":0,"linMode":0,"linPrec":0,"linType":0},"mastUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2},"rndRatio":0.0,"rndUnit":0.0,"subUnit":{"base":1,"den":1.0,"label":"m","num":1.0,"sys":2}}}"
		Visibility;NULL;1
	0:2;dgn:Category:160;Insert;No
		Code.AuthorityId;NULL;3
		Code.Namespace;NULL;""
		Code.Value;NULL;"ChangeSetTestCategory"
		Descr;NULL;""
		LastMod;NULL;2.45737e+006
		ModelId;NULL;1
		Rank;NULL;2
		Scope;NULL;1
	0:3;dgn:SubCategory:211;Insert;No
		Code.AuthorityId;NULL;3
		Code.Namespace;NULL;"10000000002"
		Code.Value;NULL;"ChangeSetTestCategory"
		LastMod;NULL;2.45737e+006
		ModelId;NULL;1
		ParentId;NULL;1099511627778
		Props;NULL;"{"color":16777215}"
	0:2;dgn:ModelContainsElements:191;Insert;No
		SourceECClassId;NULL;dgn:DictionaryModel:167
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;dgn:Category:160
		TargetECInstanceId;NULL;0:2
	0:3;dgn:ElementOwnsChildElements:180;Insert;No
		SourceECClassId;NULL;dgn:Category:160
		SourceECInstanceId;NULL;0:2
		TargetECClassId;NULL;dgn:SubCategory:211
		TargetECInstanceId;NULL;0:3
	0:3;dgn:ModelContainsElements:191;Insert;No
		SourceECClassId;NULL;dgn:DictionaryModel:167
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;dgn:SubCategory:211
		TargetECInstanceId;NULL;0:3
    */
    EXPECT_EQ(6, changeSummary.MakeInstanceIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ValidateInstanceIterator)
    {
    CreateDgnDb();

    DgnModelId csModelId = InsertSpatialModel("ChangeSummaryModel");
    SpatialModelPtr csModel = m_testDb->Models().Get<SpatialModel>(csModelId);
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    DgnElementId elementId = InsertPhysicalElement(*csModel, csCategoryId, 0, 0, 0);

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    int countIter = 0;
    for (ChangeSummary::InstanceIterator::const_iterator const& entry : changeSummary.MakeInstanceIterator())
        {
        countIter++;
        UNUSED_VARIABLE(entry);
        }
    EXPECT_EQ(countIter, 9);

    int countQuery = changeSummary.MakeInstanceIterator().QueryCount();
    EXPECT_EQ(countQuery, 9);
    }

extern ECSchemaPtr ReadECSchemaFromDisk(WCharCP schemaPathname);
extern bool ImportECSchema(ECSchemaR ecSchema, DgnDbR project);
extern IECInstancePtr CreateStartupCompanyInstance(ECSchemaCR startupSchema);

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, StructArrayChangesFromCurrentTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    IECInstancePtr instance = CreateStartupCompanyInstance(*startupSchema);
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary changeSummary(*m_testDb);

    ECInstanceKey instanceKey;
    BentleyStatus status = ImportECInstance(instanceKey, *instance, *m_testDb);
    ASSERT_TRUE(SUCCESS == status);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instance with struct array");

    /*
	ChangeSummary after inserting instance with struct array:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Foo:1099511627830;Insert;No
		anglesFoo.Alpha;NULL;12.345
		anglesFoo.Beta;NULL;12.345
		arrayOfIntsFoo;NULL;...
		doubleFoo;NULL;12.345
		intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    m_testDb->SaveChanges();

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_AnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating one row in the struct array table");

    /*
	ChangeSummary after updating one row in the struct array table:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Foo:1099511627830;Update;No
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

    m_testDb->SaveChanges();

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_AnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after deleting one row in the struct array table");

    /*
	ChangeSummary after deleting one row in the struct array table:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Foo:1099511627830;Update;No
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Update));

    m_testDb->SaveChanges();
    
    ECClassCP ecClass = m_testDb->Schemas().GetECClass(instanceKey.GetECClassId());
    ASSERT_TRUE(ecClass != nullptr);
    Utf8PrintfString deleteECSql("DELETE FROM %s.%s WHERE ECInstanceId=?", ecClass->GetSchema().GetName().c_str(), ecClass->GetName().c_str());

    ECSqlStatement ecSqlStmt;
    ECSqlStatus ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, deleteECSql.c_str());
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ecSqlStmt.BindId(1, instanceKey.GetECInstanceId());
    DbResult stepStatus = ecSqlStmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == stepStatus);

    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after deleting instance that contains a struct array");

    /*
	ChangeSummary after deleting instance that contains a struct array:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Foo:1099511627830;Delete;No
		anglesFoo.Alpha;12.345;NULL
		anglesFoo.Beta;12.345;NULL
		arrayOfIntsFoo;...;NULL
		doubleFoo;12.345;NULL
		intFoo;67;NULL
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Delete));

    m_testDb->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, StructArrayChangesFromSavedTransactions)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    IECInstancePtr instance = CreateStartupCompanyInstance(*startupSchema);
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary changeSummary(*m_testDb);

    ECInstanceKey instanceKey;
    BentleyStatus status = ImportECInstance(instanceKey, *instance, *m_testDb);
    ASSERT_TRUE(SUCCESS == status);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instance with struct array");

    /*
	ChangeSummary after inserting instance with struct array:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Foo:1099511627830;Insert;No
		anglesFoo.Alpha;NULL;12.345
		anglesFoo.Beta;NULL;12.345
		arrayOfIntsFoo;NULL;...
		doubleFoo;NULL;12.345
		intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    Statement stmt;
    DbResult result = stmt.Prepare(*m_testDb, "UPDATE sc_AnglesStruct SET Alpha=1, Beta=2, Theta=3 WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating one row in the struct array table");

    /*
	ChangeSummary after updating one row in the struct array table:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Foo:1099511627830;Insert;No
		anglesFoo.Alpha;NULL;12.345
		anglesFoo.Beta;NULL;12.345
		arrayOfIntsFoo;NULL;...
		doubleFoo;NULL;12.345
		intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    stmt.Finalize();
    result = stmt.Prepare(*m_testDb, "DELETE FROM sc_AnglesStruct WHERE ECArrayIndex=2");
    ASSERT_TRUE(BE_SQLITE_OK == result);
    result = stmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == result);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after deleting one row in the struct array table");

    /*
	ChangeSummary after deleting one row in the struct array table:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Foo:1099511627830;Insert;No
		anglesFoo.Alpha;NULL;12.345
		anglesFoo.Beta;NULL;12.345
		arrayOfIntsFoo;NULL;...
		doubleFoo;NULL;12.345
		intFoo;NULL;67
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, instanceKey.GetECInstanceId(), "StartupCompany", "Foo", DbOpcode::Insert));

    ECClassCP ecClass = m_testDb->Schemas().GetECClass(instanceKey.GetECClassId());
    ASSERT_TRUE(ecClass != nullptr);
    Utf8PrintfString deleteECSql("DELETE FROM %s.%s WHERE ECInstanceId=?", ecClass->GetSchema().GetName().c_str(), ecClass->GetName().c_str());

    ECSqlStatement ecSqlStmt;
    ECSqlStatus ecSqlStatus = ecSqlStmt.Prepare(*m_testDb, deleteECSql.c_str());
    ASSERT_TRUE(ECSqlStatus::Success == ecSqlStatus);
    ecSqlStmt.BindId(1, instanceKey.GetECInstanceId());
    DbResult stepStatus = ecSqlStmt.Step();
    ASSERT_TRUE(BE_SQLITE_DONE == stepStatus);

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after deleting instance that contains a struct array");

    /*
	ChangeSummary after deleting instance that contains a struct array:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
    */
    EXPECT_EQ(0, changeSummary.MakeInstanceIterator().QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromCurrentTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    DbResult stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    /*
	ChangeSummary after inserting instances:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Employee:1099511627819;Insert;No
		FirstName;NULL;"John"
		LastName;NULL;"Doe"
	0:2;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"AcmeWorks"
	0:3;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"CmeaWorks"
	0:4;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Tesla"
		Model;NULL;"Model-S"
	0:5;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Toyota"
		Model;NULL;"Prius"
    */
    EXPECT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    m_testDb->SaveChanges();

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey1.GetECClassId());
    statement.BindId(4, companyKey1.GetECInstanceId());

    ECInstanceKey employeeCompanyKey;
    stepStatus = statement.Step(employeeCompanyKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey1.GetECClassId());
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
	0:6;StartupCompany:EmployeeHardware:1099511627824;Insert;No
		SourceECClassId;NULL;StartupCompany:Employee:1099511627819
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;StartupCompany:Hardware:1099511627814
		TargetECInstanceId;NULL;0:4
	0:1;StartupCompany:EmployeeCompany:1099511627821;Insert;No
		SourceECClassId;NULL;StartupCompany:Employee:1099511627819
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;StartupCompany:Company:1099511627810
		TargetECInstanceId;NULL;0:2
    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    m_testDb->SaveChanges();

    /* 
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeCompany WHERE EmployeeCompany.ECInstanceId=?");
    statement.BindId(1, employeeCompanyKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey2.GetECClassId());
    statement.BindId(4, hardwareKey2.GetECInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey2.GetECClassId());
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
	0:6;StartupCompany:EmployeeHardware:1099511627824;Delete;No
		SourceECClassId;StartupCompany:Employee:1099511627819;NULL
		SourceECInstanceId;0:1;NULL
		TargetECClassId;StartupCompany:Hardware:1099511627814;NULL
		TargetECInstanceId;0:4;NULL
	0:7;StartupCompany:EmployeeHardware:1099511627824;Insert;No
		SourceECClassId;NULL;StartupCompany:Employee:1099511627819
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;StartupCompany:Hardware:1099511627814
		TargetECInstanceId;NULL;0:5
	0:1;StartupCompany:EmployeeCompany:1099511627821;Update;No
		SourceECClassId;StartupCompany:Employee:1099511627819;StartupCompany:Employee:1099511627819
		SourceECInstanceId;0:1;0:1
		TargetECClassId;StartupCompany:Company:1099511627810;StartupCompany:Company:1099511627810
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
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromSavedTransaction)
    {
    CreateDgnDb();

    BeFileName schemaPathname;
    BeTest::GetHost().GetDocumentsRoot(schemaPathname);
    schemaPathname.AppendToPath(L"DgnDb\\ECDb\\Schemas\\StartupCompany.02.00.ecschema.xml");

    ECSchemaPtr startupSchema = ReadECSchemaFromDisk(schemaPathname);
    ASSERT_TRUE(startupSchema.IsValid());

    ImportECSchema(*startupSchema, *m_testDb);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    DbResult stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    m_testDb->SaveChanges();

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    /*
	ChangeSummary after inserting instances:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Employee:1099511627819;Insert;No
		FirstName;NULL;"John"
		LastName;NULL;"Doe"
	0:2;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"AcmeWorks"
	0:3;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"CmeaWorks"
	0:4;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Tesla"
		Model;NULL;"Model-S"
	0:5;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Toyota"
		Model;NULL;"Prius"
    */
    EXPECT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(companyKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey1.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(hardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey1.GetECClassId());
    statement.BindId(4, companyKey1.GetECInstanceId());

    ECInstanceKey employeeCompanyKey;
    stepStatus = statement.Step(employeeCompanyKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey1.GetECClassId());
    statement.BindId(4, hardwareKey1.GetECInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    m_testDb->SaveChanges();

    changeSummary.Free();
    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    /*
	ChangeSummary after inserting relationships:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Employee:1099511627819;Insert;No
		FirstName;NULL;"John"
		LastName;NULL;"Doe"
	0:2;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"AcmeWorks"
	0:3;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"CmeaWorks"
	0:4;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Tesla"
		Model;NULL;"Model-S"
	0:5;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Toyota"
		Model;NULL;"Prius"
	0:6;StartupCompany:EmployeeHardware:1099511627824;Insert;No
		SourceECClassId;NULL;StartupCompany:Employee:1099511627819
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;StartupCompany:Hardware:1099511627814
		TargetECInstanceId;NULL;0:4
	0:1;StartupCompany:EmployeeCompany:1099511627821;Insert;No
		SourceECClassId;NULL;StartupCompany:Employee:1099511627819
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;StartupCompany:Company:1099511627810
		TargetECInstanceId;NULL;0:2
    */
    EXPECT_EQ(7, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    /*
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "DELETE FROM StartupCompany.EmployeeCompany WHERE EmployeeCompany.ECInstanceId=?");
    statement.BindId(1, employeeCompanyKey.GetECInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) hardwareKey2.GetECClassId());
    statement.BindId(4, hardwareKey2.GetECInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(*m_testDb, "INSERT INTO StartupCompany.EmployeeCompany (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindInt64(1, (int64_t) employeeKey.GetECClassId());
    statement.BindId(2, employeeKey.GetECInstanceId());
    statement.BindInt64(3, (int64_t) companyKey2.GetECClassId());
    statement.BindId(4, companyKey2.GetECInstanceId());

    ECInstanceKey employeeCompanyKey2;
    stepStatus = statement.Step(employeeCompanyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    m_testDb->SaveChanges();

    changeSummary.Free();
    GetChangeSummaryFromSavedTransactions(changeSummary);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    /*
	ChangeSummary after updating (deleting and inserting different) relationships:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:1;StartupCompany:Employee:1099511627819;Insert;No
		FirstName;NULL;"John"
		LastName;NULL;"Doe"
	0:2;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"AcmeWorks"
	0:3;StartupCompany:Company:1099511627810;Insert;No
		Name;NULL;"CmeaWorks"
	0:4;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Tesla"
		Model;NULL;"Model-S"
	0:5;StartupCompany:Hardware:1099511627814;Insert;No
		Make;NULL;"Toyota"
		Model;NULL;"Prius"
	0:7;StartupCompany:EmployeeHardware:1099511627824;Insert;No
		SourceECClassId;NULL;StartupCompany:Employee:1099511627819
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;StartupCompany:Hardware:1099511627814
		TargetECInstanceId;NULL;0:5
	0:1;StartupCompany:EmployeeCompany:1099511627821;Insert;No
		SourceECClassId;NULL;StartupCompany:Employee:1099511627819
		SourceECInstanceId;NULL;0:1
		TargetECClassId;NULL;StartupCompany:Company:1099511627810
		TargetECInstanceId;NULL;0:3
    */
    EXPECT_EQ(7, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(employeeCompanyKey.GetECInstanceId() == employeeCompanyKey2.GetECInstanceId());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeCompanyKey.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeCompany", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(employeeHardwareKey2.GetECInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, ElementChildRelationshipChanges)
    {
    CreateDgnDb();

    DgnModelId csModelId = InsertSpatialModel("ChangeSummaryModel");
    SpatialModelPtr csModel = m_testDb->Models().Get<SpatialModel>(csModelId);
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    DgnElementId parentElementId = InsertPhysicalElement(*csModel, csCategoryId, 0, 0, 0);
    DgnElementId childElementId = InsertPhysicalElement(*csModel, csCategoryId, 1, 1, 1);

    m_testDb->SaveChanges();

    RefCountedPtr<DgnElement> childElementPtr = m_testDb->Elements().GetForEdit<DgnElement>(childElementId);
    
    DgnDbStatus dbStatus = childElementPtr->SetParentId(parentElementId);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);
    
    childElementPtr->Update(&dbStatus);
    ASSERT_TRUE(DgnDbStatus::Success == dbStatus);

    ChangeSummary changeSummary(*m_testDb);
    GetChangeSummaryFromCurrentTransaction(changeSummary);
    
    DumpChangeSummary(changeSummary, "ChangeSummary after setting ParentId");

    /*
	ChangeSummary after setting ParentId:
	BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
		AccessString;OldValue;NewValue
	0:5;dgn:PhysicalElement:194;Update;No
		LastMod;2.45737e+006;2.45737e+006
		ParentId;NULL;1099511627780
	0:5;dgn:ElementOwnsChildElements:180;Insert;No
		SourceECClassId;NULL;dgn:PhysicalElement:194
		SourceECInstanceId;NULL;0:4
		TargetECClassId;NULL;dgn:PhysicalElement:194
		TargetECInstanceId;NULL;0:5
    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), DGN_ECSCHEMA_NAME, DGN_RELNAME_ElementOwnsChildElements, DbOpcode::Insert)); // Captured due to change of FK relationship (ParentId column)
    EXPECT_TRUE(ChangeSummaryContainsInstance(changeSummary, ECInstanceId(childElementId.GetValueUnchecked()), DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement, DbOpcode::Update)); // Captured due to change of ParentId property

    ECClassId relClassId = m_testDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_RELNAME_ElementOwnsChildElements);
    ECClassId elClassId = m_testDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_PhysicalElement);

    ChangeSummary::Instance instance = changeSummary.GetInstance(elClassId, childElementId);
    ASSERT_TRUE(instance.IsValid());

    ChangeSummary::Instance relInstance = changeSummary.GetInstance(relClassId, childElementId);
    ASSERT_TRUE(relInstance.IsValid());

    DbDupValue value(nullptr);

    value = relInstance.GetNewValue("SourceECClassId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(elClassId, value.GetValueInt64());
    EXPECT_EQ(elClassId, relInstance.GetNewValue("SourceECClassId").GetValueInt64());

    value = relInstance.GetNewValue("SourceECInstanceId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(parentElementId.GetValueUnchecked(), value.GetValueInt64());
    EXPECT_EQ(parentElementId.GetValueUnchecked(), relInstance.GetNewValue("SourceECInstanceId").GetValueInt64());

    value = relInstance.GetNewValue("TargetECClassId");
    ASSERT_TRUE(value.IsValid());
    EXPECT_EQ(elClassId, value.GetValueInt64());
    EXPECT_EQ(elClassId, relInstance.GetNewValue("TargetECClassId").GetValueInt64());

    value = relInstance.GetNewValue("TargetECInstanceId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(childElementId.GetValueUnchecked(), value.GetValueInt64());
    ASSERT_EQ(childElementId.GetValueUnchecked(), relInstance.GetNewValue("TargetECInstanceId").GetValueInt64());

    value = instance.GetNewValue("ParentId");
    ASSERT_TRUE(value.IsValid());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), value.GetValueInt64());
    ASSERT_EQ(parentElementId.GetValueUnchecked(), instance.GetNewValue("ParentId").GetValueInt64());

    EXPECT_EQ(4, relInstance.MakeValueIterator(changeSummary).QueryCount());
    EXPECT_EQ(8, instance.MakeValueIterator(changeSummary).QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, QueryChangedElements)
    {
    CreateDgnDb();

    ChangeSummary changeSummary(*m_testDb);

    DgnModelId csModelId = InsertSpatialModel("ChangeSummaryModel");
    SpatialModelPtr csModel = m_testDb->Models().Get<SpatialModel>(csModelId);
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    bset<DgnElementId> insertedElements;
    for (int ii = 0; ii < 10; ii++)
        {
        DgnElementId elementId = InsertPhysicalElement(*csModel, csCategoryId, ii, 0, 0);
        insertedElements.insert(elementId);
        }

    m_testDb->SaveChanges();

    GetChangeSummaryFromSavedTransactions(changeSummary);

    // Query changed elements directly using ECSQL
    ECSqlStatement stmt;
    Utf8CP ecsql = "SELECT el.ECInstanceId FROM dgn.PhysicalElement el WHERE IsChangedInstance(?, el.GetECClassId(), el.ECInstanceId)";
    ECSqlStatus status = stmt.Prepare(*m_testDb, ecsql);
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
    ECClassId elClassId = m_testDb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "Element");
    changeSummary.QueryByClass(changes, elClassId, true, ChangeSummary::QueryDbOpcode::All);
    changedElements.empty();
    for (bmap<ECInstanceId, ChangeSummary::Instance>::const_iterator iter = changes.begin(); iter != changes.end(); iter++)
        {
        changedElements.insert(DgnElementId(iter->first.GetValueUnchecked()));
        }
    EXPECT_EQ(insertedElements.size()+2, changedElements.size()); // category and sub-category...
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, QueryMultipleSessions)
    {
    CreateDgnDb();

    DgnModelId csModelId = InsertSpatialModel("ChangeSummaryModel");
    SpatialModelPtr csModel = m_testDb->Models().Get<SpatialModel>(csModelId);
    DgnCategoryId csCategoryId = InsertCategory("ChangeSummaryCategory");

    m_testDb->SaveChanges();
    CloseDgnDb();

    int nSessions = 5;
    int nTransactionsPerSession = 5;
    for (int ii = 0; ii < nSessions; ii++)
        {
        OpenDgnDb();

        for (int jj = 0; jj < nTransactionsPerSession; jj++)
            {
            InsertPhysicalElement(*csModel, csCategoryId, ii, jj, 0);
            m_testDb->SaveChanges();
            }

        CloseDgnDb();
        }

    OpenDgnDb();

    TxnManager::TxnId startTxnId(TxnManager::SessionId(1), 0); // First session, First transaction
    ChangeSummary changeSummary(*m_testDb);
    DgnDbStatus status = m_testDb->Txns().GetChangeSummary(changeSummary, startTxnId);
    ASSERT_TRUE(status == DgnDbStatus::Success);

    DumpChangeSummary(changeSummary, "ChangeSummary after multiple sessions");

    int expectedChangeCount = nSessions * nTransactionsPerSession + 2 /*category and subcategory*/;
    EXPECT_EQ(expectedChangeCount, GetChangeSummaryInstanceCount(changeSummary, "dgn.Element"));
    }
