/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/NamedGroupTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "GeomTestHelper.h"
#include "ConverterTestsBaseFixture.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                          03/16
//----------------------------------------------------------------------------------------
struct NamedGroupTests : public GeomTestFixture
{
    DEFINE_T_SUPER(GeomTestFixture);
    ECObjectsV8::ECSchemaPtr m_testSchema;
    BentleyApi::ECN::ECSchemaPtr m_supplementalSchema;

private:
    ECObjectsV8::ECSchemaP AddSchema(V8FileEditor& v8editor);
    void AddInstance(V8FileEditor& v8editor, ECObjectsV8::ECSchemaCP schema, DgnV8Api::ElementHandle& eh);
    DgnElementId GetGroupId(DgnElementCR member, bool groupOwnsMembers);
    DgnElementIdSet GetMembers(DgnElementCR group, bool groupOwnsMembers);

protected:
    void RemoveMember(bool groupOwnsMembers);
    void DeleteMember(bool groupOwnsMembers);
    void DeleteGroup(bool groupOwnsMembers);
    void CreateGroupInDictionary(bool groupOwnsMembers);
    void Update(bool groupOwnsMembers);

    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsV8::ECSchemaP NamedGroupTests::AddSchema(V8FileEditor& v8editor)
    {
    if (!m_testSchema.IsValid())
        {
        Utf8CP schemaXml = "<ECSchema schemaName=\"NamedGroupTestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECClass typeName='Element1' >"
            "    <ECProperty propertyName='StringProp' typeName='string' />"
            "    <ECProperty propertyName='LongProp' typeName='long' />"
            "    <ECProperty propertyName='DoubleProp' typeName='double' />"
            "  </ECClass>"
            " </ECSchema>";

        ECObjectsV8::ECSchemaReadContextPtr  schemaContext = ECObjectsV8::ECSchemaReadContext::CreateContext();
        EXPECT_EQ(SUCCESS, ECObjectsV8::ECSchema::ReadFromXmlString(m_testSchema, schemaXml, *schemaContext));

        EXPECT_EQ(DgnV8Api::SCHEMAIMPORT_Success, DgnV8Api::DgnECManager::GetManager().ImportSchema(*m_testSchema, *(v8editor.m_file)));
        v8editor.Save();
        }
    return m_testSchema.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
void NamedGroupTests::AddInstance(V8FileEditor& v8editor, ECObjectsV8::ECSchemaCP schema, DgnV8Api::ElementHandle& eh)
    {
    ECObjectsV8::ECValue doubleValue;
    doubleValue.SetDouble(12.345);
    ECObjectsV8::ECValue longValue;
    longValue.SetLong(67);

    ECObjectsV8::ECClassCP elementClass = schema->GetClassCP(L"Element1");
    DgnV8Api::DgnECInstanceEnabler* enabler = DgnV8Api::DgnECManager::GetManager().ObtainInstanceEnabler(*elementClass, *(v8editor.m_file));
    ECObjectsV8::StandaloneECInstanceR wip = enabler->GetSharedWipInstance();
    wip.SetValue(L"LongProp", longValue);
    wip.SetValue(L"DoubleProp", doubleValue);

    DgnV8Api::DgnElementECInstancePtr dgnEc;
    enabler->CreateInstanceOnElement(&dgnEc, wip, eh);

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId NamedGroupTests::GetGroupId(DgnElementCR member, bool groupOwnsMembers)
    {
    if (groupOwnsMembers)
        return member.GetParentId();

    DgnElementIdSet groups = ElementGroupsMembers::QueryGroups(member);
    EXPECT_EQ(1, groups.size());
    return *groups.begin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementIdSet NamedGroupTests::GetMembers(DgnElementCR group, bool groupOwnsMembers)
    {
    if (groupOwnsMembers)
        return group.QueryChildren();

    return ElementGroupsMembers::QueryMembers(group);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
void NamedGroupTests::RemoveMember(bool groupOwnsMembers)
    {
    LineUpFiles(L"Update.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    ECObjectsV8::ECSchemaP schema = nullptr;
    if (groupOwnsMembers)
        schema = AddSchema(v8editor);
    DgnV8Api::ElementId eid1;
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid1);
    v8editor.AddLine(&eid2);

    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    DgnV8Api::NamedGroupPtr nGroup1;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    if (groupOwnsMembers)
        {
        DgnV8Api::ElementHandle eh(nGroup1->GetElementRef());
        AddInstance(v8editor, schema, eh);
        }
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    DgnElementId groupId;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);

        DgnElementCPtr elem2 = FindV8ElementInDgnDb(*db, eid1);
        EXPECT_TRUE(elem2.IsValid());
        groupId = GetGroupId(*elem2, groupOwnsMembers);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        EXPECT_TRUE(elemGroup.IsValid());

        EXPECT_EQ(2, GetMembers(*elemGroup, groupOwnsMembers).size()) << "There should be 2 member in NamedGroup";
        }

    nGroup1->SetDescription(L"Test Description 1 Updated");
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->RemoveMember(0));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    // Verify Updated
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        ASSERT_TRUE(elemGroup.IsValid());

        ASSERT_EQ(1, GetMembers(*elemGroup, groupOwnsMembers).size()) << "After removing member element, there should be only 1 member in NamedGroup";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2016
//---------------+---------------+---------------+---------------+---------------+-------
void NamedGroupTests::DeleteMember(bool groupOwnsMembers)
    {
    LineUpFiles(L"Update.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    ECObjectsV8::ECSchemaP schema = nullptr;
    if (groupOwnsMembers)
        schema = AddSchema(v8editor);
    DgnV8Api::ElementId eid1;
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid1);
    v8editor.AddLine(&eid2);

    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    DgnV8Api::NamedGroupPtr nGroup1;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    if (groupOwnsMembers)
        {
        DgnV8Api::ElementHandle eh(nGroup1->GetElementRef());
        AddInstance(v8editor, schema, eh);
        }
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    DgnElementId groupId;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);

        DgnElementCPtr elem2 = FindV8ElementInDgnDb(*db, eid1);
        EXPECT_TRUE(elem2.IsValid());
        groupId = GetGroupId(*elem2, groupOwnsMembers);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        EXPECT_TRUE(elemGroup.IsValid());

        EXPECT_EQ(2, GetMembers(*elemGroup, groupOwnsMembers).size()) << "There should be 2 member in NamedGroup";
        }

    DgnV8Api::EditElementHandle v8Eh(eid1, v8editor.m_defaultModel);
    ASSERT_TRUE(v8Eh.IsValid());
    ASSERT_EQ(BentleyApi::SUCCESS, v8Eh.DeleteFromModel());
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    // Verify Updated
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        ASSERT_TRUE(elemGroup.IsValid());

        ASSERT_EQ(1, GetMembers(*elemGroup, groupOwnsMembers).size()) << "After delete a member element, there should be only 1 member in NamedGroup";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2016
//---------------+---------------+---------------+---------------+---------------+-------
void NamedGroupTests::DeleteGroup(bool groupOwnsMembers)
    {
    LineUpFiles(L"Update.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    ECObjectsV8::ECSchemaP schema = nullptr;
    if (groupOwnsMembers)
        schema = AddSchema(v8editor);
    DgnV8Api::ElementId eid1;
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid1);
    v8editor.AddLine(&eid2);

    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    DgnV8Api::NamedGroupPtr nGroup1;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    if (groupOwnsMembers)
        {
        DgnV8Api::ElementHandle eh(nGroup1->GetElementRef());
        AddInstance(v8editor, schema, eh);
        }
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    DgnElementId groupId;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);

        DgnElementCPtr elem2 = FindV8ElementInDgnDb(*db, eid1);
        EXPECT_TRUE(elem2.IsValid());
        groupId = GetGroupId(*elem2, groupOwnsMembers);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        EXPECT_TRUE(elemGroup.IsValid());

        EXPECT_EQ(2, GetMembers(*elemGroup, groupOwnsMembers).size()) << "There should be 2 member in NamedGroup";
        }

    nGroup1->SetDescription(L"Test Description 1 Updated");
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->DeleteFromFile());
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    // Verify Updated
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        ASSERT_FALSE(elemGroup.IsValid()) << "Element group should have been deleted";
        DgnElementCPtr elem1 = FindV8ElementInDgnDb(*db, eid1);
        DgnElementCPtr elem2 = FindV8ElementInDgnDb(*db, eid2);
        if (groupOwnsMembers)
            {
            ASSERT_FALSE(elem1.IsValid()) << "Element member should have been deleted when its group was deleted and is owned by group";
            ASSERT_FALSE(elem2.IsValid()) << "Element member should have been deleted when its group was deleted and is owned by group";
            }
        else
            {
            ASSERT_TRUE(elem1.IsValid()) << "Element member should not have been deleted when its group was deleted and it isn't owned by group";
            ASSERT_TRUE(elem2.IsValid()) << "Element member should not have been deleted when its group was deleted and it isn't owned by group";
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2016
//---------------+---------------+---------------+---------------+---------------+-------
void NamedGroupTests::Update(bool groupOwnsMembers)
    {
    LineUpFiles(L"Update.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    ECObjectsV8::ECSchemaP schema = nullptr;
    if (groupOwnsMembers)
        schema = AddSchema(v8editor);
    DgnV8Api::ElementId eid1;
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid1);
    v8editor.AddLine(&eid2);

    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    DgnV8Api::NamedGroupPtr nGroup1;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    if (groupOwnsMembers)
        {
        DgnV8Api::ElementHandle eh(nGroup1->GetElementRef());
        AddInstance(v8editor, schema, eh);
        }
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    DgnElementId groupId;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);

        DgnElementCPtr elem2 = FindV8ElementInDgnDb(*db, eid1);
        EXPECT_TRUE(elem2.IsValid());
        groupId = GetGroupId(*elem2, groupOwnsMembers);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        EXPECT_TRUE(elemGroup.IsValid());

        EXPECT_EQ(2, GetMembers(*elemGroup, groupOwnsMembers).size()) << "There should be 2 member in NamedGroup";
        }

    //Create a named group.
    DgnV8Api::ElementId eid3;
    DgnV8Api::ElementId eid4;
    DgnV8Api::ElementId eid5;
    v8editor.AddLine(&eid3);
    v8editor.AddLine(&eid4);
    v8editor.AddLine(&eid5);

    DgnV8Api::NamedGroupPtr nGroup2;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup2, L"Test Named Group 2", L"Test Description 2", ngFlags, v8editor.m_defaultModel));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->AddMember(eid3, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->AddMember(eid4, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->WriteToFile(true));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid5, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    if (groupOwnsMembers)
        {
        DgnV8Api::ElementHandle eh(nGroup2->GetElementRef());
        AddInstance(v8editor, schema, eh);
        }
    v8editor.Save();

    DoUpdate(m_dgnDbFileName, m_v8FileName);
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);

        DgnElementCPtr elem2 = FindV8ElementInDgnDb(*db, eid1);
        EXPECT_TRUE(elem2.IsValid());
        groupId = GetGroupId(*elem2, groupOwnsMembers);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        EXPECT_TRUE(elemGroup.IsValid());

        EXPECT_EQ(3, GetMembers(*elemGroup, groupOwnsMembers).size()) << "There should now be 3 members in NamedGroup";

        DgnElementCPtr elem3 = FindV8ElementInDgnDb(*db, eid3);
        EXPECT_TRUE(elem3.IsValid());
        groupId = GetGroupId(*elem3, groupOwnsMembers);
        DgnElementCPtr elemGroup2 = db->Elements().GetElement(groupId);
        EXPECT_TRUE(elemGroup2.IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2016
//---------------+---------------+---------------+---------------+---------------+-------
void NamedGroupTests::CreateGroupInDictionary(bool groupOwnsMembers)
    {
    LineUpFiles(L"Dictionary.bim", L"Test3d.dgn", false); 
    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    ECObjectsV8::ECSchemaP schema = nullptr;
    if (groupOwnsMembers)
        schema = AddSchema(v8editor);

    DgnV8Api::ElementId eid1;
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid1);
    v8editor.AddLine(&eid2);

    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    ngFlags.m_allowFarReferences = 1;
    DgnV8Api::NamedGroupPtr nGroup1;

    //EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, &v8editor.m_file->GetDictionaryModel()));
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    if (groupOwnsMembers)
        {
        DgnV8Api::ElementHandle eh(nGroup1->GetElementRef());
        AddInstance(v8editor, schema, eh);
        }
    v8editor.Save();
    uint32_t memberCount;
    nGroup1->GetMemberCount(&memberCount, nullptr);
    ASSERT_EQ(2, memberCount);

    DoConvert(m_dgnDbFileName, m_v8FileName);
    DgnElementId groupId;
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);

        DgnElementCPtr elem1 = FindV8ElementInDgnDb(*db, eid1);
        EXPECT_TRUE(elem1.IsValid());
        groupId = GetGroupId(*elem1, groupOwnsMembers);
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        EXPECT_TRUE(elemGroup.IsValid());
        if (groupOwnsMembers)
            {
            //EXPECT_EQ(0, strcmp(GENERIC_CLASS_PhysicalObject, elemGroup->MyHandlerECClassName())) << "Expected " << GENERIC_CLASS_PhysicalObject << " but was " << elemGroup->MyHandlerECClassName();
            EXPECT_EQ(elemGroup->GetModelId(), elem1->GetModelId());
            }
        else
            {
            //EXPECT_EQ(0, strcmp(GENERIC_CLASS_Group, elemGroup->MyHandlerECClassName())) << "Expected " << GENERIC_CLASS_Group << " but was " << elemGroup->MyHandlerECClassName();
            EXPECT_TRUE(elemGroup->GetModel()->IsInformationModel());
            }
        }

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NamedGroupTests, Basic)
    {
    LineUpFiles(L"NamedGroup.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1);
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid2);
    DgnV8Api::ElementId eid3;
    v8editor.AddLine(&eid3);
    
    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    DgnV8Api::NamedGroupPtr nGroup1, emtpyGroup;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(emtpyGroup, L"Empty Named Group", L"Test Description Empty", ngFlags, v8editor.m_defaultModel));

    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    EXPECT_EQ(DgnV8Api::NG_Success, emtpyGroup->WriteToFile(true));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);
        DgnElementCPtr elem1 = FindV8ElementInDgnDb(*db, eid1);
        ASSERT_TRUE(elem1.IsValid());
        DgnElementIdSet groups = ElementGroupsMembers::QueryGroups(*elem1);
        EXPECT_EQ(1, groups.size()) << "Basic element should be part of one group";
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NamedGroupTests, NamedGroupOf2dElement)
    {
    LineUpFiles(L"NamedGroup.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::DgnModelStatus modelStatus;
    DgnV8Api::DgnModel* drawingModel = v8editor.m_file->CreateNewModel(&modelStatus, L"TestModelNew", DgnV8Api::DgnModelType::Drawing, /*is3D*/ false);
    EXPECT_TRUE(DgnV8Api::DGNMODEL_STATUS_Success == modelStatus);
    v8editor.Save();
    v8editor.m_file->SetDefaultModelID(drawingModel->GetModelId());
    v8editor.Save();
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1, drawingModel);
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid2, drawingModel);
    
    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    DgnV8Api::NamedGroupPtr nGroup1, emtpyGroup;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, drawingModel));

    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, drawingModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, drawingModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);
        DgnElementCPtr elem1 = FindV8ElementInDgnDb(*db, eid1);
        ASSERT_TRUE(elem1.IsValid());
        DgnElementIdSet groups = ElementGroupsMembers::QueryGroups(*elem1);
        // CGM: Currently, the converter empties 2d models if it isn't the root model after doing the initial conversion.  This means the control list is empty when
        // it tries to convert the named group, and no named groups are created.
//        EXPECT_EQ(1, groups.size()) << "2d element should be part of one group"; 
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NamedGroupTests, NestedNamedGroup)
    {
    LineUpFiles(L"NamedGroup.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    V8FileEditor v8editor;
    v8editor.Open(m_v8FileName);
    DgnV8Api::ElementId eid1;
    v8editor.AddLine(&eid1);
    DgnV8Api::ElementId eid2;
    v8editor.AddLine(&eid2);
    DgnV8Api::ElementId eid3;
    v8editor.AddLine(&eid3);
    DgnV8Api::ElementId eid4;
    v8editor.AddLine(&eid4);
    
    //Create a named group.
    DgnV8Api::NamedGroupFlags ngFlags;
    DgnV8Api::NamedGroupPtr nGroup1, nGroup2;
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
    EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup2, L"Test Named Group 2", L"Test Description 2", ngFlags, v8editor.m_defaultModel));

    DgnV8Api::NamedGroupMemberFlags memberFlags;
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));

    EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->AddMember(eid3, v8editor.m_defaultModel, memberFlags));
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->AddMember(nGroup1->GetElementRef()->GetElementId(), v8editor.m_defaultModel, memberFlags)); // Nest NameGroup
    EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->WriteToFile(true));
    v8editor.Save();

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    if (true)
        {
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        VerifyElement(*db, eid1, GeometricPrimitive::GeometryType::CurvePrimitive);
        DgnElementCPtr elem3 = FindV8ElementInDgnDb(*db, eid3);
        ASSERT_TRUE(elem3.IsValid());
        DgnElementIdSet groups = ElementGroupsMembers::QueryGroups(*elem3);
        EXPECT_EQ(1, groups.size()) << "Line element should be part of one group";

        DgnElementId groupId;
        groupId = *groups.begin();
        DgnElementCPtr elemGroup = db->Elements().GetElement(groupId);
        ASSERT_TRUE(elemGroup.IsValid());

#if 0 // WIP_GROUPS
        ASSERT_EQ(2, ElementGroupsMembers::QueryMembers(*elemGroup).size()) << "There should be 2 member in NamedGroup";
#endif
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NamedGroupTests, NamedGroupInReferenceModel)
    {
    LineUpFiles(L"NamedGroupInReferenceModel.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    RepositoryLinkId repo2; // TODO look up RepositoryLink for second file

    DgnV8Api::ElementId ng1id=0, ng2id=0;
        {
        //--------------------------------------------------------------------------------------------------------
        // Add attachement from different file and verify update operation
        BentleyApi::BeFileName refV8File;
        CreateAndAddV8Attachment(refV8File, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        V8FileEditor v8editor_ref;
        v8editor_ref.Open(refV8File);
    
        DgnV8Api::ElementId eid1;
        v8editor.AddLine(&eid1);
        DgnV8Api::ElementId eid2;
        v8editor.AddLine(&eid2);
        DgnV8Api::ElementId eid3;
        v8editor_ref.AddLine(&eid3);
        DgnV8Api::ElementId eid4;
        v8editor_ref.AddLine(&eid4);
    
        //Create a named group.
        DgnV8Api::NamedGroupFlags ngFlags;
        DgnV8Api::NamedGroupPtr nGroup1, nGroup2;
        EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));
        EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup2, L"Test Named Group 2", L"Test Description 2", ngFlags, v8editor_ref.m_defaultModel));

        DgnV8Api::NamedGroupMemberFlags memberFlags;
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
        v8editor.Save();

        EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->AddMember(eid3, v8editor_ref.m_defaultModel, memberFlags));
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->AddMember(eid4, v8editor_ref.m_defaultModel, memberFlags));
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup2->WriteToFile(true));
        v8editor_ref.Save();

        ng1id = nGroup1->GetElementRef()->GetElementId();
        ng2id = nGroup2->GetElementRef()->GetElementId();
        }

    ASSERT_NE(0, ng1id);
    ASSERT_NE(0, ng2id);

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    if (true)
        {
        // Named Group from Master Model
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        DgnElementCPtr groupElem1 = FindV8ElementInDgnDb(*db, ng1id);
        ASSERT_TRUE(groupElem1.IsValid());
        ASSERT_EQ(2, ElementGroupsMembers::QueryMembers(*groupElem1).size()) << "There should be 2 member in NamedGroup";
        // Named Group from referenced Model
        DgnElementCPtr groupElem2 = FindV8ElementInDgnDb(*db, ng2id, repo2);
        ASSERT_TRUE(groupElem2.IsValid());
        ASSERT_EQ(2, ElementGroupsMembers::QueryMembers(*groupElem2).size()) << "There should be 2 member in NamedGroup";
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          04/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NamedGroupTests, NamedGroupAcrossReference)
    {
    LineUpFiles(L"NamedGroupAcrossReference.bim", L"Test3d.dgn", false); // creates TestAddRef.bim from Test3d.dgn and defines m_dgnDbFileName, and m_v8FileName

    DgnV8Api::ElementId ng1id = 0;
        {
        //--------------------------------------------------------------------------------------------------------
        // Add attachement from different file and verify update operation
        BentleyApi::BeFileName refV8File;
        CreateAndAddV8Attachment(refV8File, 1);  // Create a copy of Test3d.dgn and attach it. Note that it will have the same level table.

        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        V8FileEditor v8editor_ref;
        v8editor_ref.Open(refV8File);


        DgnV8Api::ElementId eid1;
        v8editor.AddLine(&eid1);
        DgnV8Api::ElementId eid2;
        v8editor_ref.AddLine(&eid2);
        DgnV8Api::ElementId eid3;
        v8editor_ref.AddLine(&eid3);
        v8editor_ref.Save();
    
        //Create a named group.
        DgnV8Api::NamedGroupFlags ngFlags;
        ngFlags.m_allowFarReferences = true;
        DgnV8Api::NamedGroupPtr nGroup1;
        EXPECT_EQ(DgnV8Api::NG_Success, DgnV8Api::NamedGroup::Create(nGroup1, L"Test Named Group 1", L"Test Description 1", ngFlags, v8editor.m_defaultModel));

        DgnV8Api::NamedGroupMemberFlags memberFlags;
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid1, v8editor.m_defaultModel, memberFlags));
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->AddMember(eid2, v8editor.m_defaultModel, memberFlags));
        EXPECT_EQ(DgnV8Api::NG_Success, nGroup1->WriteToFile(true));
        v8editor.Save();

        ng1id = nGroup1->GetElementRef()->GetElementId();
        }

    DoConvert(m_dgnDbFileName, m_v8FileName);
    // Verify
    if (true)
        {
        // Named Group having member in far reference
        DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
        DgnElementCPtr groupElem1 = FindV8ElementInDgnDb(*db, ng1id);
        ASSERT_TRUE(groupElem1.IsValid());
        ASSERT_EQ(2, ElementGroupsMembers::QueryMembers(*groupElem1).size()) << "There should be 2 member in NamedGroup";
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NamedGroupTests, UpdateWithRemovedMember)
    {
    RemoveMember(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NamedGroupTests, UpdateWithRemovedMemberInOwnedGroup)
    {
    RemoveMember(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, UpdateWithDeletedMember)
    {
    DeleteMember(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, UpdateWithDeletedMemberInOwnedGroup)
    {
    DeleteMember(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, UpdateWithDeletedGroup)
    {
    DeleteGroup(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, UpdateWithDeletedOwnedGroup)
    {
    DeleteGroup(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, ConvertNamedGroupFromDictionary)
    {
    CreateGroupInDictionary(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, ConvertNamedGroupFromDictionaryWithOwnership)
    {
    CreateGroupInDictionary(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, UpdateWithAddedMember)
    {
    Update(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NamedGroupTests, UpdateWithAddedMemberWithOwnership)
    {
    Update(true);
    }
