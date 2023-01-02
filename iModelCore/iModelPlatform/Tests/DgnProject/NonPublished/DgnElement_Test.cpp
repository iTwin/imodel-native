/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <ECObjects/ECJsonUtilities.h>
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// @bsiclass
//----------------------------------------------------------------------------------------
struct DgnElementTests : public DgnDbTestFixture
    {
    TestElementCPtr AddChild(DgnElementCR parent);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnElementTests, InsertElement)
    {
    SetupSeedProject();

    //Inserts a model
    PhysicalModelPtr m1 = DgnDbTestUtils::InsertPhysicalModel(*m_db, "Model1");
    EXPECT_TRUE(m1.IsValid());
    m_db->SaveChanges("changeSet1");

    auto keyE1 = InsertElement(m1->GetModelId());
    DgnElementId e1id = keyE1->GetElementId();
    DgnElementCPtr e1 = m_db->Elements().GetElement(e1id);
    EXPECT_TRUE(e1 != nullptr);

    // Now update without making any change => LASTMOD should change anyway.
    uint64_t insertTime;
    ASSERT_EQ(BSISUCCESS, e1->QueryLastModifyTime().ToJulianDay(insertTime));

    BeThreadUtilities::BeSleep(1); // NB! LASTMOD resolution is 1 millisecond!

    auto ed1 = e1->CopyForEdit();
    ed1->Update();
    e1 = m_db->Elements().GetElement(e1id);
    uint64_t updateTime;
    ASSERT_EQ(BSISUCCESS, e1->QueryLastModifyTime().ToJulianDay(updateTime));
    EXPECT_NE(insertTime, updateTime);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TestElementCPtr DgnElementTests::AddChild(DgnElementCR parent)
    {
    TestElementPtr child = TestElement::Create(*m_db, parent.GetModelId(), m_defaultCategoryId);
    child->SetParentId(parent.GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
    auto el = child->Insert();
    if (!el.IsValid())
        return nullptr;
    return dynamic_cast<TestElement const*>(el.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ElementCopierTests)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    DgnElementCPtr parent = TestElement::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
    TestElementCPtr c1 = AddChild(*parent);
    TestElementCPtr c2 = AddChild(*parent);

    DgnModelPtr destModel = parent->GetModel();

    // Verify that children are copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyChildren(true);
        DgnElementCPtr parent_cc = copier.MakeCopy(nullptr, *destModel, *parent, DgnCode());
        ASSERT_TRUE(parent_cc.IsValid());
        auto c1ccId = copier.GetCloneContext().FindElementId(c1->GetElementId());
        ASSERT_TRUE(c1ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c1ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c1ccId)->GetModel() == destModel);
        auto c2ccId = copier.GetCloneContext().FindElementId(c2->GetElementId());
        ASSERT_TRUE(c2ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c2ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(c2ccId)->GetModel() == destModel);
        size_t cccount = 0;
        for (auto childid : parent_cc->QueryChildren())
            {
            ASSERT_TRUE(childid == c1ccId || childid == c2ccId);
            ++cccount;
            }
        ASSERT_EQ(2, cccount);

        // Verify that a second attempt to copy an already copied element does nothing
        ASSERT_TRUE(parent_cc.get() == copier.MakeCopy(nullptr, *destModel, *parent, DgnCode()).get());
        ASSERT_TRUE(c1ccId == copier.MakeCopy(nullptr, *destModel, *c1, DgnCode())->GetElementId());
        ASSERT_TRUE(c2ccId == copier.MakeCopy(nullptr, *destModel, *c2, DgnCode())->GetElementId());
        }

    // Verify that children are NOT copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyChildren(false);
        DgnElementCPtr parent_cc = copier.MakeCopy(nullptr, *destModel, *parent, DgnCode());
        ASSERT_TRUE(parent_cc.IsValid());
        auto c1ccId = copier.GetCloneContext().FindElementId(c1->GetElementId());
        ASSERT_FALSE(c1ccId.IsValid());
        auto c2ccId = copier.GetCloneContext().FindElementId(c2->GetElementId());
        ASSERT_FALSE(c2ccId.IsValid());
        ASSERT_EQ(0, parent_cc->QueryChildren().size());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ElementCopierTests_Group)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();

    DgnElementCPtr group = TestGroup::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
    DgnElementCPtr m1 = TestElement::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
    DgnElementCPtr m2 = TestElement::Create(*m_db, model->GetModelId(), m_defaultCategoryId)->Insert();
    ElementGroupsMembers::Insert(*group, *m1, 0);
    ElementGroupsMembers::Insert(*group, *m2, 0);
    ASSERT_TRUE(group->ToIElementGroup()->QueryMembers().size() == 2);

    DgnModelPtr destModel = group->GetModel();

    // Verify that members are copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyGroups(true);
        DgnElementCPtr group_cc = copier.MakeCopy(nullptr, *destModel, *group, DgnCode());
        ASSERT_TRUE(group_cc.IsValid());
        auto m1ccId = copier.GetCloneContext().FindElementId(m1->GetElementId());
        ASSERT_TRUE(m1ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m1ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m1ccId)->GetModel() == destModel);
        auto m2ccId = copier.GetCloneContext().FindElementId(m2->GetElementId());
        ASSERT_TRUE(m2ccId.IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m2ccId).IsValid());
        ASSERT_TRUE(destModel->GetDgnDb().Elements().GetElement(m2ccId)->GetModel() == destModel);
        size_t cccount = 0;
        for (auto memberid : group_cc->ToIElementGroup()->QueryMembers())
            {
            ASSERT_TRUE(memberid == m1ccId || memberid == m2ccId);
            ++cccount;
            }
        ASSERT_EQ(2, cccount);

        // Verify that a second attempt to copy an already copied element does nothing
        ASSERT_TRUE(group_cc.get() == copier.MakeCopy(nullptr, *destModel, *group, DgnCode()).get());
        ASSERT_TRUE(m1ccId == copier.MakeCopy(nullptr, *destModel, *m1, DgnCode())->GetElementId());
        ASSERT_TRUE(m2ccId == copier.MakeCopy(nullptr, *destModel, *m2, DgnCode())->GetElementId());
        }

    // Verify that members are NOT copied
        {
        DgnCloneContext ccontext;
        ElementCopier copier(ccontext);
        copier.SetCopyGroups(false);
        DgnElementCPtr group_cc = copier.MakeCopy(nullptr, *destModel, *group, DgnCode());
        ASSERT_TRUE(group_cc.IsValid());
        ASSERT_EQ(0, group_cc->ToIElementGroup()->QueryMembers().size());
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, CopyIdentityFrom)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnModelId modelId = model->GetModelId();
    DgnCategoryId categoryId = GetDefaultCategoryId();
    DgnClassId classId = m_db->Domains().GetClassId(generic_ElementHandler::PhysicalObject::GetHandler());
    DgnElementId elementId;

    // Test creating an element the "normal" way (by letting the DgnElementId be assigned by the framework)
        {
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(GenericPhysicalObject::CreateParams(*m_db, modelId, classId, categoryId));
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        ASSERT_FALSE(element->Insert().IsValid()) << "Second insert of the same element should fail";
        elementId = element->GetElementId();
        }

    DgnElementId forcedElementId(elementId.GetValue() + 100);
    BeGuid forcedFederationGuid(true);

    // Confirm that supplying a DgnElementId in CreateParams for Insert does not work (not intended to work)
        {
        GenericPhysicalObject::CreateParams createParams(*m_db, modelId, classId, categoryId);
        createParams.SetElementId(DgnElementId(elementId.GetValue() + 100));

        GenericPhysicalObjectPtr element = new GenericPhysicalObject(createParams);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->Insert().IsValid()) << "It is not valid to supply a DgnElementId for Insert via CreateParams";
        }

    // Test PKPM's synchronization workflow where they must force a DgnElementId on Insert.
        {
        GenericPhysicalObjectPtr element = new GenericPhysicalObject(GenericPhysicalObject::CreateParams(*m_db, modelId, classId, categoryId));
        ASSERT_TRUE(element.IsValid());
        element->CopyIdentityFrom(forcedElementId, forcedFederationGuid);
        ASSERT_EQ(element->GetElementId(), forcedElementId);
        ASSERT_EQ(element->GetFederationGuid(), forcedFederationGuid);
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_EQ(element->GetElementId(), forcedElementId);
        ASSERT_EQ(element->GetFederationGuid(), forcedFederationGuid);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, GenericDomainElements)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnCategoryId categoryId = GetDefaultCategoryId();

    // GenericSpatialLocation
        {
        GenericSpatialLocationPtr element = GenericSpatialLocation::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        }

    // GenericPhysicalObject
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        ASSERT_TRUE(element->GetElementId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementGeomAndPlacementTests : DgnElementTests
{
    GeometryBuilderPtr CreateGeom();
    RefCountedPtr<PhysicalElement> CreateElement(bool wantPlacement, bool wantGeom);
    static bool AreEqualPlacements(Placement3dCR a, Placement3dCR b);
    void TestLoadElem(DgnElementId id, Placement3d const* placement, bool hasGeometry);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryBuilderPtr ElementGeomAndPlacementTests::CreateGeom()
    {
    DgnModelPtr model = m_db->Models().GetModel(m_defaultModelId);
    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, m_defaultCategoryId, DPoint3d::From(0.0, 0.0, 0.0));
    CurveVectorPtr curveVector = GeomHelper::computeShape();
    builder->Append(*curveVector);
    double dz = 3.0, radius = 1.5;
    DgnConeDetail cylinderDetail(DPoint3d::From(0, 0, 0), DPoint3d::From(0, 0, dz), radius, radius, true);
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone(cylinderDetail);
    builder->Append(*cylinder);

    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,
        0, 0, 2,
        0, 3, 0,
        0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    builder->Append(*ellipse);

    Render::GeometryParams elemDisplayParams;
    elemDisplayParams.SetCategoryId(m_defaultCategoryId);
    elemDisplayParams.SetWeight(2);
    builder->Append(elemDisplayParams);

    return builder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementGeomAndPlacementTests::AreEqualPlacements(Placement3dCR a, Placement3dCR b)
    {
    return DoubleOps::AlmostEqual(a.GetOrigin().x, b.GetOrigin().x)
        && DoubleOps::AlmostEqual(a.GetOrigin().y, b.GetOrigin().y)
        && DoubleOps::AlmostEqual(a.GetOrigin().z, b.GetOrigin().z)
        && DoubleOps::AlmostEqual(a.GetAngles().GetYaw().Degrees(), b.GetAngles().GetYaw().Degrees())
        && DoubleOps::AlmostEqual(a.GetAngles().GetPitch().Degrees(), b.GetAngles().GetPitch().Degrees())
        && DoubleOps::AlmostEqual(a.GetAngles().GetRoll().Degrees(), b.GetAngles().GetRoll().Degrees());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGeomAndPlacementTests::TestLoadElem(DgnElementId id, Placement3d const* placement, bool hasGeometry)
    {
    auto el = m_db->Elements().Get<PhysicalElement>(id);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(nullptr != placement, el->GetPlacement().IsValid());
    if (nullptr != placement)
        {
        EXPECT_TRUE(placement->IsValid());
        EXPECT_TRUE(el->GetPlacement().IsValid());
        EXPECT_TRUE(AreEqualPlacements(*placement, el->GetPlacement()));
        }

    EXPECT_EQ(hasGeometry, el->HasGeometry());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ElementGeomAndPlacementTests, ValidateOnInsert)
    {
    SetupSeedProject();

    DgnElementId noPlacementNoGeomId, placementAndGeomId, placementAndNoGeomId;
    Placement3d placement;

        {
        // Null placement + null geom
        TestElementPtr el = TestElement::CreateWithoutGeometry(*m_db, m_defaultModelId, m_defaultCategoryId);
        EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
        TestLoadElem(el->GetElementId(), nullptr, false);
        noPlacementNoGeomId = el->GetElementId();

        // Non-null placement + non-null geom
        el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnCode());
        auto geom = CreateGeom();
        EXPECT_EQ(SUCCESS, geom->Finish(*el));
        placement = el->GetPlacement();
        EXPECT_TRUE(placement.IsValid());
        EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
        placementAndGeomId = el->GetElementId();

        // Non-null placement + null geom
        el = TestElement::CreateWithoutGeometry(*m_db, m_defaultModelId, m_defaultCategoryId);
        el->SetPlacement(placement);
        EXPECT_TRUE(m_db->Elements().Insert(*el).IsValid());
        placementAndNoGeomId = el->GetElementId();

        // Null placement + non-null geom
        el = TestElement::Create(*m_db, m_defaultModelId, m_defaultCategoryId, DgnCode());
        EXPECT_EQ(SUCCESS, geom->Finish(*el));
        el->SetPlacement(Placement3d());
        DgnDbStatus status;
        EXPECT_FALSE(m_db->Elements().Insert(*el, &status).IsValid());
        EXPECT_EQ(DgnDbStatus::BadElement, status);
        }

    m_db->Elements().ClearCache();

    TestLoadElem(noPlacementNoGeomId, nullptr, false);
    TestLoadElem(placementAndGeomId, &placement, true);
    TestLoadElem(placementAndNoGeomId, &placement, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int32_t countElementsOfClass(DgnClassId classId, DgnDbR db)
    {
    CachedStatementPtr stmt = db.Elements().GetStatement("SELECT count(*) FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ECClassId=?");
    stmt->BindUInt64(1, classId.GetValue());
    stmt->Step();
    return stmt->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* INSERT statements on element table were using the ClassId of the base class if
* the derived class did not define its own Handler subclass.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, HandlerlessClass)
    {
    SetupSeedProject();

    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);
    EXPECT_TRUE(el.Insert().IsValid());

    EXPECT_EQ(0, countElementsOfClass(TestElement::QueryClassId(*m_db), *m_db));
    EXPECT_EQ(1, countElementsOfClass(classId, *m_db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, TestAutoHandledPropertiesCA)
    {
    SetupSeedProject();
    // *** test Custom Attributes when we get them
    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    static Utf8CP s_autoHandledPropNames[] = {
        "IntegerProperty1"
        ,"IntegerProperty2"
        ,"IntegerProperty3"
        ,"IntegerProperty4"
        ,"DoubleProperty1"
        ,"DoubleProperty2"
        ,"DoubleProperty3"
        ,"DoubleProperty4"
        ,"PointProperty1"
        ,"PointProperty2"
        ,"PointProperty3"
        ,"PointProperty4"
        ,"StringProperty"
        ,"IntProperty"
    };

    for (int i = 0; i<_countof(s_autoHandledPropNames); ++i)
    {
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, s_autoHandledPropNames[i]));
        EXPECT_TRUE(checkValue.IsNull());
    }

    // Check a few non-auto-handled props
    ECN::ECValue checkValue;
    EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "TestIntegerProperty1"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledProperties)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t datetime;
    uint32_t datetimeutc;
    uint32_t boolean;
    uint32_t p2d;
    uint32_t p3d;
    DateTime dateTime = DateTime(2016, 9, 24);
    ECN::ECValue checkValue;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(boolean, "b"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(datetime, "dt"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(datetimeutc, "dtUtc"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(p2d, "p2d"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(p3d, "p3d"));

        // get properties
        ASSERT_EQ(false, el.GetPropertyValueBoolean("b"));
        ASSERT_TRUE(!(el.GetPropertyValueDateTime("dt").IsValid()));
        ASSERT_TRUE(!(el.GetPropertyValueDateTime("dtUtc").IsValid()));
        ASSERT_TRUE(el.GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 0))));
        ASSERT_TRUE(el.GetPropertyValueDPoint3d("p3d").IsEqual((DPoint3d::From(0, 0, 0))));

        // No unhandled properties yet
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_TRUE(checkValue.IsNull());

        // Set unhandled property (in memory)
        ASSERT_EQ(DgnDbStatus::Success, el.SetPropertyValue("StringProperty", ECN::ECValue("initial value")));

        // check that we see the pending value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());

        // Set properties
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("b", false));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dt", dateTime));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dtUtc", dateTime));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p2d", DPoint2d::From(0, 9)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p3d", DPoint3d::From(0, 9, 9)));
        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());

        // Check that we see the stored value in memory
        EXPECT_EQ(false, persistentEl->GetPropertyValueBoolean("b"));

        EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dt").Equals(dateTime, true));

        EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dtUtc").Equals(dateTime, true));

        EXPECT_EQ(DPoint2d::From(0, 9), persistentEl->GetPropertyValueDPoint2d("p2d"));

        EXPECT_EQ(DPoint3d::From(0, 9, 9), persistentEl->GetPropertyValueDPoint3d("p3d"));

        elementId = persistentEl->GetElementId();
        m_db->SaveChanges();
        }

    // Before updating the element check what is stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    EXPECT_EQ(false, element->GetPropertyValueBoolean("b"));

    EXPECT_TRUE(element->GetPropertyValueDateTime("dt").Equals(dateTime, true));

    EXPECT_TRUE(element->GetPropertyValueDateTime("dtUtc").Equals(dateTime, true));

    EXPECT_EQ(DPoint2d::From(0, 9), element->GetPropertyValueDPoint2d("p2d"));

    EXPECT_EQ(DPoint3d::From(0, 9, 9), element->GetPropertyValueDPoint3d("p3d"));
    // Get some non-auto-handled properties using the same dynamic property API
    EXPECT_EQ(element->GetModelId(), element->GetPropertyValueId<DgnModelId>("Model"));
    EXPECT_EQ(element->GetCategoryId(), element->GetPropertyValueId<DgnCategoryId>("Category"));
    EXPECT_STREQ(element->GetUserLabel(), element->GetPropertyValueString("UserLabel").c_str());
    EXPECT_EQ(element->GetCode().GetCodeSpecId(), element->GetPropertyValueId<CodeSpecId>("CodeSpec"));
    EXPECT_EQ(element->GetCode().GetScopeElementId(*m_db), element->GetPropertyValueId<DgnElementId>("CodeScope"));
    EXPECT_STREQ(element->GetCode().GetValueUtf8().c_str(), element->GetPropertyValueString("CodeValue").c_str());
    }

    if (true)
        {
        // Get ready to modify the element
         TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);

        // initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("initial value", checkValue.ToString().c_str());
        EXPECT_EQ(false, editEl->GetPropertyValueBoolean("b"));
        // Set a new value (in memory)
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("StringProperty", ECN::ECValue("changed value")));
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("b", true));
        // check that we now see the pending value on the edited copy ...
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "StringProperty"));
        EXPECT_STREQ("changed value", checkValue.ToString().c_str());
        EXPECT_EQ(true, editEl->GetPropertyValueBoolean("b"));
        // Update the element
        DgnDbStatus stat = editEl->Update();
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(nullptr == m_db->Elements().FindLoadedElement(editEl->GetElementId())); // after update, element should not be in MRU cache.
        m_db->SaveChanges();
        }

    // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "StringProperty"));
    EXPECT_STREQ("changed value", checkValue.ToString().c_str());
    EXPECT_EQ(true, Element->GetPropertyValueBoolean("b"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, FederationGuid_Update)
    {
    SetupSeedProject();
    DgnElementId elementId;
    DgnDbStatus stat;
    //Insert element
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());
        elementId = persistentEl->GetElementId();
        }

    BeGuid fedId(true);
        {
        // Get ready to modify the element
        TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
        ASSERT_TRUE(editEl.IsValid());

        editEl->SetFederationGuid(fedId);
        DgnDbStatus stat;
        DgnElementCPtr updatedElement = editEl->UpdateAndGet(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(updatedElement.IsValid());
        EXPECT_EQ(fedId, updatedElement->GetFederationGuid());
        }

    TestElementCPtr el = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(el.IsValid());
    EXPECT_EQ(fedId, el->GetFederationGuid());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledStructProperties)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t LocStruct;
    ECN::ECValue checkValue;
    if (true)
        {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(LocStruct, "Location"));

        // get Struct properties before setting
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.Street"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.Name"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.State"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.Country"));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_TRUE(checkValue.IsNull());

        // check that we see the pending value
        checkValue.Clear();
        // Set a struct valued property in memory
        BeTest::SetFailOnAssert(false);
        EXPECT_NE(DgnDbStatus::Success, el.SetPropertyValue("Location", ECN::ECValue("<<you cannot set a struct directly>>")));
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.Street", ECN::ECValue("690 Pennsylvania Drive")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Name", ECN::ECValue("Exton")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.State", ECN::ECValue("PA")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Country", ECN::ECValue("US")));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("Location.City.Zip", ECN::ECValue(19341)));
        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());
        // Check that we see the stored value in memory

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.Street"));
        EXPECT_STREQ("690 Pennsylvania Drive", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Name"));
        EXPECT_STREQ("Exton", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.State"));
        EXPECT_STREQ("PA", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Country"));
        EXPECT_STREQ("US", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_EQ(19341, checkValue.GetInteger());
        elementId = persistentEl->GetElementId();
        m_db->SaveChanges();
        }
    // Before updatation of element check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.Street"));
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.Name"));
    EXPECT_STREQ("Exton", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.State"));
    EXPECT_STREQ("PA", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.Country"));
    EXPECT_STREQ("US", checkValue.ToString().c_str());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "Location.City.Zip"));
    EXPECT_EQ(19341, checkValue.GetInteger());
    }
    if (true)
        {
        // Get ready to modify the element
        TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);

        // initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_EQ(19341, checkValue.GetInteger());

        // Set a new value (in memory)
        EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("Location.City.Zip", ECN::ECValue(19342)));

        // check that we now see the pending value on the edited copy ...
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "Location.City.Zip"));
        EXPECT_EQ(19342, checkValue.GetInteger());
        // Update the element
        DgnDbStatus stat =  editEl->Update();
        ASSERT_EQ(DgnDbStatus::Success, stat);
        m_db->SaveChanges();
        }

    // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "Location.City.Zip"));
    EXPECT_EQ(19342, checkValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledArrayProperties)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t iArrayOfPoint3d;
    uint32_t iArrayOfString;
    uint32_t iArrayOfInt;
    if (true)
    {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);

        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfPoint3d, "ArrayOfPoint3d"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfString, "ArrayOfString"));
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfInt, "ArrayOfInt"));

        // Set an array property
        EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfString, 0, 4));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfString", ECN::ECValue("first"), PropertyArrayIndex(1)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfString", ECN::ECValue("second"), PropertyArrayIndex(2)));

        EXPECT_EQ(DgnDbStatus::Success, el.AddPropertyArrayItems(iArrayOfPoint3d, 2));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(1, 1, 1)), PropertyArrayIndex(0)));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2, 2, 2)), PropertyArrayIndex(1)));
        EXPECT_NE(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2, 2, 2)), PropertyArrayIndex(2)));

        EXPECT_EQ(DgnDbStatus::Success, el.AddPropertyArrayItems(iArrayOfInt, 300));
        for (auto i = 0; i < 300; ++i)
        {
            EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfInt", ECN::ECValue(i), PropertyArrayIndex(i)));
        }
        // Clear all array property index from memory before clear first Get them
        ECN::ECValue checkValue;
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(1, 1, 1)));
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));
        EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(2, 2, 2)));
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.ClearPropertyArray(iArrayOfPoint3d));
        EXPECT_NE(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());
        EXPECT_NE(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));
        EXPECT_TRUE(checkValue.IsNull());
        EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfPoint3d, 0, 1));
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(1, 1, 1)), PropertyArrayIndex(0)));
        EXPECT_NE(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfPoint3d", ECN::ECValue(DPoint3d::From(2, 2, 2)), PropertyArrayIndex(1)));

        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);

        ASSERT_TRUE(persistentEl.IsValid());

        // Check that we see the stored value in memory
        checkValue.Clear();

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
        EXPECT_STREQ("first", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
        EXPECT_STREQ("second", checkValue.ToString().c_str());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
        EXPECT_TRUE(checkValue.IsNull());

        EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(1, 1, 1)));

        EXPECT_NE(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));

        for (auto i = 0; i < 300; ++i)
        {
            EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfInt", PropertyArrayIndex(i)));
            EXPECT_EQ(i, checkValue.GetInteger());
        }

        elementId = persistentEl->GetElementId();
        m_db->SaveChanges();
    }
    // Before updatation of element check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    ECN::ECValue checkValue;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
    EXPECT_STREQ("first", checkValue.ToString().c_str());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
    EXPECT_STREQ("second", checkValue.ToString().c_str());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
    ASSERT_TRUE(checkValue.IsNull());
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(0)));
    ASSERT_TRUE(checkValue.GetPoint3d().IsEqual(DPoint3d::From(1, 1, 1)));
    checkValue.Clear();
    ASSERT_NE(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfPoint3d", PropertyArrayIndex(1)));

    for (auto i = 0; i < 300; ++i)
         {
         ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfInt", PropertyArrayIndex(i)));
         ASSERT_EQ(i, checkValue.GetInteger());
        }
    }
    if (true)
        {
        // Get ready to modify the element
        TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
        ASSERT_TRUE(editEl.IsValid());
        // initially we still see the initial/stored value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
        EXPECT_STREQ("first", checkValue.ToString().c_str());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
        EXPECT_STREQ("second", checkValue.ToString().c_str());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
        EXPECT_TRUE(checkValue.IsNull());

        // Remove array item from memory
        EXPECT_EQ(DgnDbStatus::Success, editEl->RemovePropertyArrayItem(iArrayOfString, 1));
        //Verfiy the array item is removed from memory by getting its value
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(0)));
        EXPECT_TRUE(checkValue.IsNull());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
        EXPECT_STREQ("second", checkValue.ToString().c_str());
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(2)));
        EXPECT_TRUE(checkValue.IsNull());
        EXPECT_NE(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(3)));
        // Update the element
        DgnDbStatus stat = editEl->Update();
        ASSERT_EQ(DgnDbStatus::Success, stat);
        m_db->SaveChanges();
        }

     // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "ArrayOfString", PropertyArrayIndex(1)));
    EXPECT_STREQ("second", checkValue.ToString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GetSetAutoHandledStructArrayProperties)
    {
    SetupSeedProject();

    DgnElementId elementId;
    uint32_t iArrayOfStructs;
    ECN::ECValue checkValue;
    ECN::IECInstancePtr checkInstance;
    {
    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    TestElement el(params);

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfStructs, "ArrayOfStructs"));

    // Set an array property
    EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfStructs, 0, 4));

    EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfStructs"));
    EXPECT_TRUE(checkValue.IsArray());
    EXPECT_EQ(4, checkValue.GetArrayInfo().GetCount());

    for (int i = 0; i < 4; i++)
        {
        checkValue.Clear();
        EXPECT_EQ(DgnDbStatus::Success, el.GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(i)));
        EXPECT_TRUE(checkValue.IsNull());
        }

    ECN::ECClassCP locationStruct = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_LOCATION_STRUCT_CLASS_NAME);
    ASSERT_TRUE(nullptr != locationStruct);
    ECN::StandaloneECEnablerPtr locationEnabler = locationStruct->GetDefaultStandaloneEnabler();

    ECN::IECInstancePtr extonLocInstance = locationEnabler->CreateInstance().get();
    extonLocInstance->SetValue("Street", ECN::ECValue("690 Pennsylvania Drive"));
    extonLocInstance->SetValue("City.Name", ECN::ECValue("Exton"));
    extonLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    extonLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    extonLocInstance->SetValue("City.Zip", ECN::ECValue(19341));

    ECN::ECValue extonLocValue;
    extonLocValue.SetStruct(extonLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", extonLocValue, PropertyArrayIndex(1)));

    ECN::IECInstancePtr phillyLocInstance = locationEnabler->CreateInstance().get();
    phillyLocInstance->SetValue("Street", ECN::ECValue("1601 Cherry Street"));
    phillyLocInstance->SetValue("City.Name", ECN::ECValue("Philadelphia"));
    phillyLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    phillyLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    phillyLocInstance->SetValue("City.Zip", ECN::ECValue(19102));

    ECN::ECValue phillyLocValue;
    phillyLocValue.SetStruct(phillyLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", phillyLocValue, PropertyArrayIndex(2)));

    // Insert the element
    DgnDbStatus stat;
    DgnElementCPtr persistentEl = el.Insert(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);

    ASSERT_TRUE(persistentEl.IsValid());

    // Check that we see the stored value in memory
    checkValue.Clear();

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());

    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_FALSE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());

    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19341, checkValue.GetInteger());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_FALSE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());

    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));
    EXPECT_TRUE(checkValue.IsNull());

    elementId = persistentEl->GetElementId();
    m_db->SaveChanges();
    }

    // Before updating the element check what is stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    TestElementPtr element = m_db->Elements().GetForEdit<TestElement>(elementId);
    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());

    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());

    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());

    checkValue.Clear();
    ASSERT_EQ(DgnDbStatus::Success, element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));
    EXPECT_TRUE(checkValue.IsNull());
    EXPECT_TRUE(checkValue.IsStruct());
    }

    {
    // Get ready to modify the element
    TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
    ASSERT_TRUE(editEl.IsValid());
    // initially we still see the initial/stored value
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));
    EXPECT_TRUE(checkValue.IsNull());

    // Remove array item from memory
    EXPECT_EQ(DgnDbStatus::Success, editEl->RemovePropertyArrayItem(iArrayOfStructs, 1));
    //Verfiy the array item is removed from memory by getting its value
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0)));
    EXPECT_TRUE(checkValue.IsNull());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());

    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(2)));
    EXPECT_TRUE(checkValue.IsNull());
    EXPECT_NE(DgnDbStatus::Success, editEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(3)));

    // Update the element
    DgnDbStatus stat = editEl->Update();
    ASSERT_EQ(DgnDbStatus::Success, stat);
    m_db->SaveChanges();
    }

    // REALLY check that the stored value was changed
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr Element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(Element.IsValid());
    checkValue.Clear();
    EXPECT_EQ(DgnDbStatus::Success, Element->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1)));
    EXPECT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, OverrideAutohandledproperites)
    {
    SetupSeedProject();
    DgnElementId elementId;
    uint32_t boolean;
    if (true)
    {
        DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_CLASS_OVERRIDE_AUTOHADLEPROPERTIES));
        TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
        TestElement el(params);
        ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(boolean, "p2d"));
        // get property before setting
        ASSERT_TRUE(el.GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 0))));
        // set property
        EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p2d", DPoint2d::From(0, 9)));
        //get property after setting
        ASSERT_TRUE(el.GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
        // Insert the element
        DgnDbStatus stat;
        DgnElementCPtr persistentEl = el.Insert(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());
        elementId = persistentEl->GetElementId();
        // Check that what stored value in memory
        ASSERT_TRUE(persistentEl->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
     }
    BeFileName fileName = m_db->GetFileName();
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    // Check that what stored value in DB
    TestElementCPtr element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(element->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
    }
    if (true)
       {
       // Get ready to modify the element
       TestElementPtr editEl = m_db->Elements().GetForEdit<TestElement>(elementId);
       // initially we still see the initial/stored value
       ASSERT_TRUE(editEl->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 9))));
       EXPECT_EQ(DgnDbStatus::Success, editEl->SetPropertyValue("p2d", DPoint2d::From(0, 10)));
       // Update the element
       DgnDbStatus stat = editEl->Update();
       ASSERT_EQ(DgnDbStatus::Success, stat);
       }
    // check that the stored value
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    TestElementCPtr element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(element.IsValid());
    ASSERT_TRUE(element->GetPropertyValueDPoint2d("p2d").IsEqual((DPoint2d::From(0, 10))));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void compareDateTimes(DateTimeCR dateTime1, DateTimeCR dateTime2)
    {
    EXPECT_TRUE(dateTime1.Equals(dateTime2, false));
    EXPECT_EQ(dateTime1.GetYear(), dateTime2.GetYear());
    EXPECT_EQ(dateTime1.GetMonth(), dateTime2.GetMonth());
    EXPECT_EQ(dateTime1.GetDay(), dateTime2.GetDay());
    EXPECT_EQ(dateTime1.GetHour(), dateTime2.GetHour());
    EXPECT_EQ(dateTime1.GetMinute(), dateTime2.GetMinute());
    EXPECT_EQ(dateTime1.GetSecond(), dateTime2.GetSecond());
    EXPECT_EQ(dateTime1.GetMillisecond(), dateTime2.GetMillisecond());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ParentChildSameModel)
    {
    SetupSeedProject();

    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "testCategory");
    PhysicalModelPtr modelA = DgnDbTestUtils::InsertPhysicalModel(*m_db, "modelA");
    PhysicalModelPtr modelB = DgnDbTestUtils::InsertPhysicalModel(*m_db, "modelB");

    GenericPhysicalObjectPtr parentA = GenericPhysicalObject::Create(*modelA, categoryId);
    GenericPhysicalObjectPtr parentB = GenericPhysicalObject::Create(*modelB, categoryId);
    EXPECT_TRUE(parentA.IsValid());
    EXPECT_TRUE(parentB.IsValid());
    EXPECT_TRUE(parentA->Insert().IsValid());
    EXPECT_TRUE(parentB->Insert().IsValid());
    DgnElementId childIdA, childIdB, childIdC;
    DgnClassId parentRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);

    // test DgnElement::_OnChildInsert success
        {
        GenericPhysicalObjectPtr childA = GenericPhysicalObject::Create(*modelA, categoryId);
        GenericPhysicalObjectPtr childB = GenericPhysicalObject::Create(*modelB, categoryId);
        GenericPhysicalObjectPtr childC = GenericPhysicalObject::Create(*modelB, categoryId);
        EXPECT_TRUE(childA.IsValid());
        EXPECT_TRUE(childB.IsValid());
        EXPECT_TRUE(childC.IsValid());
        childA->SetParentId(parentA->GetElementId(), parentRelClassId); // Match
        childB->SetParentId(parentB->GetElementId(), parentRelClassId); // Match
        EXPECT_TRUE(childA->Insert().IsValid()) << "Expecting success because models of parent and child are the same";
        EXPECT_TRUE(childB->Insert().IsValid()) << "Expecting success because models of parent and child are the same";
        EXPECT_TRUE(childC->Insert().IsValid()) << "Expecting success because childC has no parent";
        EXPECT_TRUE(childA->GetParentId().IsValid());
        EXPECT_TRUE(childB->GetParentId().IsValid());
        EXPECT_FALSE(childC->GetParentId().IsValid());
        EXPECT_TRUE(childA->GetParentRelClassId().IsValid());
        EXPECT_TRUE(childB->GetParentRelClassId().IsValid());
        EXPECT_FALSE(childC->GetParentRelClassId().IsValid());
        childIdA = childA->GetElementId();
        childIdB = childB->GetElementId();
        childIdC = childC->GetElementId();
        }

    // test DgnElement::_OnChildInsert failure
        {
        GenericPhysicalObjectPtr childA = GenericPhysicalObject::Create(*modelA, categoryId);
        GenericPhysicalObjectPtr childB = GenericPhysicalObject::Create(*modelB, categoryId);
        EXPECT_TRUE(childA.IsValid());
        EXPECT_TRUE(childB.IsValid());
        childA->SetParentId(parentB->GetElementId(), parentRelClassId); // Mismatch
        childB->SetParentId(parentA->GetElementId(), parentRelClassId); // Mismatch
        DgnDbStatus insertStatusA, insertStatusB;
        BeTest::SetFailOnAssert(false);
        EXPECT_FALSE(childA->Insert(&insertStatusA).IsValid()) << "Expecting failure because models of parent and child are different";
        EXPECT_FALSE(childB->Insert(&insertStatusB).IsValid()) << "Expecting failure because models of parent and child are different";
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::WrongModel, insertStatusA);
        EXPECT_EQ(DgnDbStatus::WrongModel, insertStatusB);
        }

    // test SetParentId on existing elements
        {
        GenericPhysicalObjectPtr childA = m_db->Elements().GetForEdit<GenericPhysicalObject>(childIdA);
        GenericPhysicalObjectPtr childB = m_db->Elements().GetForEdit<GenericPhysicalObject>(childIdB);
        GenericPhysicalObjectPtr childC = m_db->Elements().GetForEdit<GenericPhysicalObject>(childIdC);
        EXPECT_TRUE(childA.IsValid());
        EXPECT_TRUE(childB.IsValid());
        EXPECT_TRUE(childC.IsValid());
        childA->SetParentId(parentB->GetElementId(), parentRelClassId); // Mismatch
        childB->SetParentId(parentA->GetElementId(), parentRelClassId); // Mismatch
        childC->SetParentId(parentA->GetElementId(), parentRelClassId); // Mismatch
        DgnDbStatus updateStatusA, updateStatusB, updateStatusC;
        BeTest::SetFailOnAssert(false);
        updateStatusA = childA->Update();
        updateStatusB = childB->Update();
        updateStatusC = childC->Update();
        BeTest::SetFailOnAssert(true);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusA);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusB);
        EXPECT_EQ(DgnDbStatus::WrongModel, updateStatusC);

        childC->SetParentId(parentB->GetElementId(), parentRelClassId); // Match
        updateStatusC = childC->Update();
        EXPECT_EQ(DgnDbStatus::Success, updateStatusC);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, FederationGuid)
    {
    SetupSeedProject();

    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestModel");
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");

    DgnElementId elementId;
    BeGuid federationGuid(true);
    BeGuid updatedFederationGuid(true);

    // Quick BeGuid tests
    EXPECT_TRUE(BeGuid(true).IsValid());
    EXPECT_FALSE(BeGuid().IsValid());
    EXPECT_NE(federationGuid, updatedFederationGuid);
    EXPECT_EQ(federationGuid, BeGuid(federationGuid.m_guid.u[0], federationGuid.m_guid.u[1]));
    EXPECT_EQ(federationGuid, BeGuid(federationGuid.m_guid.i[0], federationGuid.m_guid.i[1], federationGuid.m_guid.i[2], federationGuid.m_guid.i[3]));

    // test that FederationGuid is initialized as invalid
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_TRUE(element->GetFederationGuid().IsValid()) << "FederationGuid expected to be initialized as invalid";
        EXPECT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        EXPECT_TRUE(element->GetFederationGuid().IsValid()) << "FederationGuid expected to be initialized as invalid";
        }

    // test error conditions for QueryElementByFederationGuid
    EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(BeGuid()).IsValid()) << "Should not find an element by an invalid FederationGuid";
    EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(BeGuid(true)).IsValid()) << "Should not find an element by an unused FederationGuid";

    // flush cache and re-check element
        {
        m_db->Elements().ClearCache();
        DgnElementCPtr element = m_db->Elements().GetElement(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_TRUE(element->GetFederationGuid().IsValid());
        }

    // test when FederationGuid is specified
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        EXPECT_TRUE(element.IsValid());
        element->SetFederationGuid(federationGuid);
        EXPECT_TRUE(element->GetFederationGuid().IsValid()) << "FederationGuid should be valid after SetFederationGuid";
        EXPECT_EQ(element->GetFederationGuid(), federationGuid);
        EXPECT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        EXPECT_EQ(elementId.GetValue(), m_db->Elements().QueryElementByFederationGuid(federationGuid)->GetElementId().GetValue()) << "Should be able to query for an element by its FederationGuid";

        GenericPhysicalObjectPtr element2 = GenericPhysicalObject::Create(*model, categoryId);
        EXPECT_TRUE(element2.IsValid());
        element2->SetFederationGuid(federationGuid);
        EXPECT_FALSE(element->Insert().IsValid()) << "Cannot insert a second element with a duplicate FederationGuid";
        m_db->SaveChanges();
        }

    // flush cache and re-check element
        {
        m_db->Elements().ClearCache();
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_TRUE(element->GetFederationGuid().IsValid());
        EXPECT_EQ(element->GetFederationGuid(), federationGuid);

        element->SetFederationGuid(updatedFederationGuid);
        EXPECT_EQ(DgnDbStatus::Success, element->Update());
        m_db->SaveChanges();
        }

    // flush cache and verify set/update to updated value
        {
        m_db->Elements().ClearCache();
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_TRUE(element->GetFederationGuid().IsValid());
        EXPECT_EQ(element->GetFederationGuid(), updatedFederationGuid);
        EXPECT_TRUE(m_db->Elements().QueryElementByFederationGuid(updatedFederationGuid).IsValid());
        EXPECT_FALSE(m_db->Elements().QueryElementByFederationGuid(federationGuid).IsValid());

        element->SetFederationGuid(BeGuid()); // set FederationGuid to null
        EXPECT_EQ(DgnDbStatus::Success, element->Update());
        m_db->SaveChanges();
        }

    // flush cache and verify set/update to null value
        {
        m_db->Elements().ClearCache();
        DgnElementCPtr element = m_db->Elements().GetElement(elementId);
        EXPECT_TRUE(element.IsValid());
        EXPECT_FALSE(element->GetFederationGuid().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, PhysicalTypeCRUD)
    {
    SetupSeedProject();
    DefinitionModelPtr typeModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "PhysicalTypes");

    DgnClassId physicalTypeRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_PhysicalElementIsOfType);
    ASSERT_TRUE(physicalTypeRelClassId.IsValid());

    DgnElementId physicalTypeId[3];
    DgnElementId elementId;

    // insert some sample PhysicalTypes
    for (int32_t i=0; i<_countof(physicalTypeId); i++)
        {
        Utf8PrintfString name("PhysicalType%" PRIi32, i);
        TestPhysicalTypePtr physicalType = TestPhysicalType::Create(*typeModel, name.c_str());
        ASSERT_TRUE(physicalType.IsValid());
        physicalType->SetStringProperty(Utf8PrintfString("String%d", i).c_str());
        physicalType->SetIntProperty(i);
        ASSERT_TRUE(physicalType->Insert().IsValid());
        physicalTypeId[i] = physicalType->GetElementId();
        }

    // Insert a PhysicalType without an explicit handler
        {
        DgnClassId physicalTypeClassId = m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_CLASS_TestPhysicalTypeNoHandler);
        ASSERT_TRUE(physicalTypeClassId.IsValid());
        PhysicalType::CreateParams createParams(*m_db, typeModel->GetModelId(), physicalTypeClassId, PhysicalType::CreateCode(*typeModel, "TestPhysicalTypeNoHandler"));
        PhysicalTypePtr physicalType = PhysicalType::Create(createParams);
        ASSERT_TRUE(physicalType.IsValid());
        DgnElementId physicalTypeId = physicalType->Insert()->GetElementId();
        ASSERT_TRUE(physicalTypeId.IsValid());
        }

    // flush cache to make sure PhysicalTypes were inserted properly
        {
        m_db->Elements().ClearCache();
        for (int32_t i=0; i<_countof(physicalTypeId); i++)
            {
            TestPhysicalTypePtr physicalType = m_db->Elements().GetForEdit<TestPhysicalType>(physicalTypeId[i]);
            ASSERT_TRUE(physicalType.IsValid());
            ASSERT_STREQ(physicalType->GetStringProperty().c_str(), Utf8PrintfString("String%d", i).c_str());
            ASSERT_EQ(physicalType->GetIntProperty(), i);

            physicalType->SetStringProperty(Utf8PrintfString("Updated%d", i).c_str());
            physicalType->SetIntProperty(i+100);
            ASSERT_EQ(DgnDbStatus::Success, physicalType->Update());
            }
        }

    // flush cache to make sure PhysicalTypes were updated properly
        {
        m_db->Elements().ClearCache();
        for (int32_t i=0; i<_countof(physicalTypeId); i++)
            {
            TestPhysicalTypeCPtr physicalType = m_db->Elements().Get<TestPhysicalType>(physicalTypeId[i]);
            ASSERT_TRUE(physicalType.IsValid());
            ASSERT_STREQ(physicalType->GetStringProperty().c_str(), Utf8PrintfString("Updated%d", i).c_str());
            ASSERT_EQ(physicalType->GetIntProperty(), i+100);
            }
        }

    // create a PhysicalElement and set its PhysicalType
        {
        DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
        PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestModel");

        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        ASSERT_FALSE(element->GetPhysicalType().IsValid());
        element->SetTypeDefinition(physicalTypeId[0], physicalTypeRelClassId);
        ASSERT_TRUE(element->GetTypeDefinitionId().IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was inserted properly
        {
        m_db->Elements().ClearCache();
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), physicalTypeId[0].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(physicalTypeId[1], physicalTypeRelClassId));
        ASSERT_EQ(DgnDbStatus::Success, element->Update());
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was updated properly
        {
        m_db->Elements().ClearCache();
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetPhysicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), physicalTypeId[1].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(DgnElementId(), physicalTypeRelClassId));
        EXPECT_EQ(DgnDbStatus::Success, element->Update());
        }

    // flush cache to make sure the PhysicalElement's PhysicalType was cleared properly
        {
        m_db->Elements().ClearCache();
        GenericPhysicalObjectPtr element = m_db->Elements().GetForEdit<GenericPhysicalObject>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetPhysicalType().IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, GraphicalType2dCRUD)
    {
    SetupSeedProject();
    DefinitionModelPtr typesModel = DgnDbTestUtils::InsertDefinitionModel(*m_db, "GraphicalTypes");

    DgnClassId graphicalTypeRelClassId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_GraphicalElement2dIsOfType);
    ASSERT_TRUE(graphicalTypeRelClassId.IsValid());

    DgnElementId graphicalTypeId[3];
    DgnElementId elementId;

    // insert some sample GraphicalTypes
    for (int32_t i=0; i<_countof(graphicalTypeId); i++)
        {
        TestGraphicalType2dPtr graphicalType = TestGraphicalType2d::Create(*typesModel, Utf8PrintfString("GraphicalType%d", i).c_str());
        ASSERT_TRUE(graphicalType.IsValid());
        graphicalType->SetStringProperty(Utf8PrintfString("String%d", i).c_str());
        graphicalType->SetIntProperty(i);
        ASSERT_TRUE(graphicalType->Insert().IsValid());
        graphicalTypeId[i] = graphicalType->GetElementId();
        }

    // flush cache to make sure GraphicalTypes were inserted properly
        {
        m_db->Elements().ClearCache();
        for (int32_t i=0; i<_countof(graphicalTypeId); i++)
            {
            TestGraphicalType2dPtr graphicalType = m_db->Elements().GetForEdit<TestGraphicalType2d>(graphicalTypeId[i]);
            ASSERT_TRUE(graphicalType.IsValid());
            ASSERT_STREQ(graphicalType->GetStringProperty().c_str(), Utf8PrintfString("String%d", i).c_str());
            ASSERT_EQ(graphicalType->GetIntProperty(), i);

            graphicalType->SetStringProperty(Utf8PrintfString("Updated%d", i).c_str());
            graphicalType->SetIntProperty(i+100);
            ASSERT_EQ(DgnDbStatus::Success, graphicalType->Update());
            }
        }

    // flush cache to make sure GraphicalTypes were updated properly
        {
        m_db->Elements().ClearCache();
        for (int32_t i=0; i<_countof(graphicalTypeId); i++)
            {
            TestGraphicalType2dCPtr graphicalType = m_db->Elements().Get<TestGraphicalType2d>(graphicalTypeId[i]);
            ASSERT_TRUE(graphicalType.IsValid());
            ASSERT_STREQ(graphicalType->GetStringProperty().c_str(), Utf8PrintfString("Updated%d", i).c_str());
            ASSERT_EQ(graphicalType->GetIntProperty(), i+100);
            }
        }

    // create a TestElement2d and set its GraphicalType
        {
        DgnCategoryId categoryId = DgnDbTestUtils::InsertDrawingCategory(GetDgnDb(), "TestCategory");
        DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(GetDgnDb(), "DrawingListModel");
        DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "Drawing");
        DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);

        TestElement2dPtr element = TestElement2d::Create(GetDgnDb(), drawingModel->GetModelId(), categoryId, DgnCode(), 2.0);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        ASSERT_FALSE(element->GetGraphicalType().IsValid());
        element->SetTypeDefinition(graphicalTypeId[0], graphicalTypeRelClassId);
        ASSERT_TRUE(element->GetTypeDefinitionId().IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_TRUE(element->Insert().IsValid());
        elementId = element->GetElementId();
        }

    // flush cache to make sure the TestElement2d's GraphicalType was inserted properly
        {
        m_db->Elements().ClearCache();
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), graphicalTypeId[0].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(graphicalTypeId[1], graphicalTypeRelClassId));
        ASSERT_EQ(DgnDbStatus::Success, element->Update());
        }

    // flush cache to make sure the TestElement2d's GraphicalType was updated properly
        {
        m_db->Elements().ClearCache();
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_TRUE(element->GetGraphicalType().IsValid());
        ASSERT_EQ(element->GetTypeDefinitionId().GetValue(), graphicalTypeId[1].GetValue());

        ASSERT_EQ(DgnDbStatus::Success, element->SetTypeDefinition(DgnElementId(), graphicalTypeRelClassId));
        ASSERT_EQ(DgnDbStatus::Success, element->Update());
        }

    // flush cache to make sure the TestElement2d's GraphicalType was cleared properly
        {
        m_db->Elements().ClearCache();
        TestElement2dPtr element = m_db->Elements().GetForEdit<TestElement2d>(elementId);
        ASSERT_TRUE(element.IsValid());
        ASSERT_FALSE(element->GetGraphicalType().IsValid());
        ASSERT_FALSE(element->GetTypeDefinitionId().IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ElementIterator)
    {
    SetupSeedProject();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
    PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysicalModel");
    CodeSpecId codeSpecId = DgnDbTestUtils::InsertCodeSpec(*m_db, "TestCodeSpec");
    const int numPhysicalObjects=5;

    for (int i=0; i<numPhysicalObjects; i++)
        {
        GenericPhysicalObjectPtr element = GenericPhysicalObject::Create(*model, categoryId);
        ASSERT_TRUE(element.IsValid());
        Utf8PrintfString userLabel("UserLabel%d", i);
        element->SetUserLabel(userLabel.c_str());
        Utf8PrintfString codeValue("CodeValue%d", i);
        element->SetCode(CodeSpec::CreateCode(*m_db, "TestCodeSpec", codeValue));
        ASSERT_TRUE(element->Insert().IsValid());
        }

    ElementIterator iterator = m_db->Elements().MakeIterator(GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject));
    ASSERT_EQ(numPhysicalObjects, iterator.BuildIdSet<DgnElementId>().size());
    ASSERT_EQ(numPhysicalObjects, iterator.BuildIdList<DgnElementId>().size());

    bvector<DgnElementId> idList;
    iterator.BuildIdList(idList);
    ASSERT_EQ(numPhysicalObjects, idList.size());

    iterator = m_db->Elements().MakeIterator(GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject), "WHERE [UserLabel]='UserLabel1'");
    ASSERT_EQ(1, iterator.BuildIdSet<DgnElementId>().size());
    ASSERT_EQ(1, iterator.BuildIdList<DgnElementId>().size());

    int count=0;
    for (ElementIteratorEntryCR entry : m_db->Elements().MakeIterator(GENERIC_SCHEMA(GENERIC_CLASS_PhysicalObject), nullptr, "ORDER BY ECInstanceId"))
        {
        ASSERT_TRUE(entry.GetClassId().IsValid());
        ASSERT_EQ(entry.GetModelId(), model->GetModelId());
        Utf8PrintfString userLabel("UserLabel%d", count);
        ASSERT_STREQ(entry.GetUserLabel(), userLabel.c_str());
        Utf8PrintfString codeValue("CodeValue%d", count);
        ASSERT_STREQ(entry.GetCodeValue(), codeValue.c_str());
        ASSERT_STREQ(entry.GetCode().GetValueUtf8CP(), codeValue.c_str());
        ASSERT_EQ(entry.GetCode().GetCodeSpecId(), codeSpecId);
        count++;
        }
    ASSERT_EQ(numPhysicalObjects, count);
    }

/*---------------------------------------------------------------------------------**//**
* @return whether or not to output JSON for this geometry.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool examineGeometry(GeometricPrimitiveCR prim)
    {
    // this implementation tries to repro a valgrind tile agent issue on trimmed bsurfs
    auto bsurf = prim.GetAsMSBsplineSurface();
    if (bsurf.IsValid() && bsurf->GetNumBounds() > 0)
        {
        IFacetOptionsPtr options = IFacetOptions::Create();
        options->SetChordTolerance(0.20605140638233854);
        options->SetAngleTolerance(msGeomConst_piOver2);
        options->SetMaxPerFace(100);
        options->SetConvexFacetsRequired(true);
        options->SetCurvedSurfaceMaxPerFace(3);
        options->SetParamsRequired(true);
        options->SetNormalsRequired(true);
        options->SetEdgeChainsRequired(true);

        IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
        builder->Add(*bsurf);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void examineCollection(GeometryCollection const& collection, DgnDbR db, BeFile& outFile)
    {
    for (auto const& iter : collection)
        {
        if (GeometryCollection::Iterator::EntryType::GeometryPart == iter.GetEntryType())
            {
            auto geomPart = iter.GetGeometryPartCPtr();
            if (!geomPart.IsValid())
                continue;

            GeometryCollection partCollection(geomPart->GetGeometryStream(), db);
            partCollection.SetNestedIteratorContext(iter);
            examineCollection(partCollection, db, outFile);
            }
        else
            {
            auto geomPrim = iter.GetGeometryPtr();  // extract geometry from db
            if (!geomPrim.IsValid())
                continue;
            if (!examineGeometry(*geomPrim))
                continue;

            // append JSON for the filtered geometry to outFile
            IGeometryPtr geom;
            ICurvePrimitivePtr curvePrim;
            CurveVectorPtr curve;
            MSBsplineSurfacePtr bsurf;
            ISolidPrimitivePtr solidPrim;
            PolyfaceHeaderPtr mesh;
            if ((curvePrim = geomPrim->GetAsICurvePrimitive()).IsValid())
                geom = IGeometry::Create(curvePrim);
            else if ((curve = geomPrim->GetAsCurveVector()).IsValid())
                geom = IGeometry::Create(curve);
            else if ((bsurf = geomPrim->GetAsMSBsplineSurface()).IsValid())
                geom = IGeometry::Create(bsurf);
            else if ((solidPrim = geomPrim->GetAsISolidPrimitive()).IsValid())
                geom = IGeometry::Create(solidPrim);
            else if ((mesh = geomPrim->GetAsPolyfaceHeader()).IsValid())
                geom = IGeometry::Create(mesh);

            if (geom.IsValid() && geom->TryTransformInPlace(iter.GetGeometryToWorld()))
                {
                Utf8String json;
                if (IModelJson::TryGeometryToIModelJsonString(json, *geom))
                    {
                    size_t len = json.length();
                    ASSERT_EQ(len, (uint32_t) len);
                    outFile.Write(nullptr, json.c_str(), (uint32_t) len);
                    outFile.Write(nullptr, ",\r\n", 3);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Useful debugging tool for extracting geometry JSON from a local bim file.
* Keep fileNameIn/Out undefined so this test does nothing in automated test runs.
* To use, set fileNameIn/Out and customize examineGeometry().
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, ExamineKnownBimFile)
    {
    WString fileNameIn(L"");    // e.g., L"D:\\test.bim"
    WString fileNameOut(L"");   // e.g., L"D:\\test.imjs"
    if (fileNameIn.empty() || fileNameOut.empty())
        return;

    BeFile outFile;
    outFile.Create(fileNameOut.c_str());
    outFile.Write(nullptr, "[\r\n", 3);

    BeFileName bimFile(fileNameIn);
    OpenDb(m_db, bimFile, Db::OpenMode::Readonly, false);
    if (m_db.IsValid())
        {
        BeJsDocument val, opts;
        opts["wantGeometry"] = true;

        auto iter = m_db->Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_PhysicalElement));
        for (auto entry : iter)
            {
            auto elemId = entry.GetElementId();
            auto elem = m_db->Elements().Get<PhysicalElement>(elemId);
            if (elem.IsValid())
                examineCollection(GeometryCollection(*elem->ToGeometrySource()), *m_db, outFile);
            }
        }

    m_db->CloseDb();

    outFile.Write(nullptr, "]\r\n", 3);
    outFile.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, SimpleInsert)
    {
    SetupSeedProject();
    DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(*m_db, "TestCategory");
    SpatialLocationModelPtr spatialLocationModel = DgnDbTestUtils::InsertSpatialLocationModel(*m_db, "TestSpatialLocationModel");
    PhysicalModelPtr physicalModel = DgnDbTestUtils::InsertPhysicalModel(*m_db, "TestPhysicalModel");
    InformationRecordModelPtr informationRecordModel = DgnDbTestUtils::InsertInformationRecordModel(*m_db, "TestInformationRecordModel");

    TestSpatialLocationPtr spatialLocation1 = TestSpatialLocation::Create(*spatialLocationModel, categoryId);
    ASSERT_TRUE(spatialLocation1.IsValid());
    ASSERT_TRUE(spatialLocation1->Insert().IsValid()) << "SpatialLocationElements should be able to be inserted into a SpatialLocationModel";

    TestSpatialLocationPtr spatialLocation2 = TestSpatialLocation::Create(*physicalModel, categoryId);
    ASSERT_TRUE(spatialLocation2.IsValid());
    ASSERT_TRUE(spatialLocation2->Insert().IsValid()) << "SpatialLocationElements should be able to be inserted into a PhysicalModel";

    TestInformationRecordPtr informationRecord = TestInformationRecord::Create(*informationRecordModel);
    ASSERT_TRUE(informationRecord.IsValid());
    ASSERT_TRUE(informationRecord->Insert().IsValid()) << "InformationRecordElements should be able to be inserted into an InformationRecordModel";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, CreateSubjectChildElemet)
    {
    SetupSeedProject();
    SubjectCPtr rootSubject = m_db->Elements().GetRootSubject();
    ASSERT_TRUE(rootSubject.IsValid());
    SubjectPtr subele=Subject::Create(*rootSubject, "SubjectChild","Child1");
    ASSERT_TRUE(subele.IsValid());
    DgnElementCPtr ele=subele->Insert();
    ASSERT_TRUE(ele.IsValid());
    ASSERT_EQ(ele->GetModelId(), rootSubject->GetModelId());
    DgnElementCPtr elep = m_db->Elements().GetElement(ele->GetElementId());
    ASSERT_EQ(elep->GetDisplayLabel(),"SubjectChild");
    RefCountedCPtr<Subject> info=m_db->Elements().Get<Subject>(ele->GetElementId());
    ASSERT_EQ(info->GetDescription(),"Child1");

    SubjectCPtr subele2 = Subject::CreateAndInsert(*rootSubject, "SubjectChild2", "Child2");
    ASSERT_TRUE(subele2.IsValid());
    ASSERT_EQ(subele2->GetModelId(), rootSubject->GetModelId());
    elep = m_db->Elements().GetElement(subele2->GetElementId());
    ASSERT_EQ(elep->GetDisplayLabel(), "SubjectChild2");
    info = m_db->Elements().Get<Subject>(subele2->GetElementId());
    ASSERT_EQ(info->GetDescription(), "Child2");
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+--------
TEST_F(DgnElementTests, FromJson)
    {
    SetupSeedProject();

    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    ECN::ECValue checkValue;
    ECN::IECInstancePtr checkInstance = nullptr;
    DgnElementId elementId;

    bool const b = false;
    double const d = 3.14;
    int32_t const i = 50;
    int64_t const l = 100;
    DateTime const dt(2018, 2, 4);
    DateTime const dtUtc(2018, 2, 5);
    auto const p2d = DPoint2d::From(3, 5);
    auto const p3d = DPoint3d::From(3, 5, 7);
    Utf8String const s("test string");

    Utf8String const invalidProp("this should not be converted");
    Utf8String const customHandledProp("and neither should this");

    // Create JSON and set properties
    {
    TestElement el(params);

    BeJsDocument json;
    json["b"] = b;
    json["d"] = d;
    json["i"] = i;
    ECN::ECJsonUtilities::Int64ToJson(json["l"], l);

    ECN::ECJsonUtilities::DateTimeToJson(json["dt"], dt);
    ECN::ECJsonUtilities::DateTimeToJson(json["dtUtc"], dtUtc);

    ECN::ECJsonUtilities::Point2dToJson(json["p2d"], p2d);
    ECN::ECJsonUtilities::Point3dToJson(json["p3d"], p3d);

    json["s"] = s;

    json["ArrayOfStructs"].toArray();
    auto extonOffice = json["ArrayOfStructs"][0u];
    extonOffice["Street"] = "690 Pennsylvania Drive";
    extonOffice["City"]["Name"] = "Exton";
    extonOffice["City"]["State"] = "PA";
    extonOffice["City"]["Country"] = "US";
    extonOffice["City"]["Zip"] = 19341;

    auto phillyOffice = json["ArrayOfStructs"][1u];
    phillyOffice["Street"] = "1601 Cherry Street";
    phillyOffice["City"]["Name"] = "Philadelphia";
    phillyOffice["City"]["State"] = "PA";
    phillyOffice["City"]["Country"] = "US";
    phillyOffice["City"]["Zip"] = 19102;

    ASSERT_FALSE(ElementECPropertyAccessor(el, "invalidProp").IsValid());
    ASSERT_TRUE(ElementECPropertyAccessor(el, "invalidProp").IsAutoHandled());
    json["invalidProp"] = invalidProp;

    ASSERT_TRUE(ElementECPropertyAccessor(el, "TestElementProperty").IsValid());
    ASSERT_FALSE(ElementECPropertyAccessor(el, "TestElementProperty").IsAutoHandled());
    json["TestElementProperty"] = customHandledProp;

    // Populate element with json
    el.FromJson(json);

    // Insert the element
    DgnDbStatus stat;
    DgnElementCPtr persistentEl = el.Insert(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(persistentEl.IsValid());

    // Check in-memory Element that will be persisted
    // Test persisted values
    EXPECT_EQ(b, persistentEl->GetPropertyValueBoolean("b"));
    EXPECT_DOUBLE_EQ(d, persistentEl->GetPropertyValueDouble("d"));
    EXPECT_EQ(i, persistentEl->GetPropertyValueInt32("i"));
    ECN::ECValue longVal;
    persistentEl->GetPropertyValue(longVal, "l");
    ASSERT_TRUE(longVal.IsLong());
    EXPECT_EQ(l, longVal.GetLong());
    EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dt").Equals(dt, true));
    EXPECT_TRUE(persistentEl->GetPropertyValueDateTime("dtUtc").Equals(dtUtc, true));
    EXPECT_EQ(p2d, persistentEl->GetPropertyValueDPoint2d("p2d"));
    EXPECT_EQ(p3d, persistentEl->GetPropertyValueDPoint3d("p3d"));
    EXPECT_STREQ(s.c_str(), persistentEl->GetPropertyValueString("s").c_str());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19341, checkValue.GetInteger());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "invalidProp");
    EXPECT_TRUE(checkValue.IsNull());

    checkValue.Clear();
    persistentEl->GetPropertyValue(checkValue, "TestElementProperty");
    EXPECT_STREQ("", checkValue.GetUtf8CP());

    // Get Persistent ElementId and save to db
    elementId = persistentEl->GetElementId();
    m_db->SaveChanges();
    }

    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);

    // Retrieve properties
    {
    TestElementCPtr retrievedElement = m_db->Elements().Get<TestElement>(elementId);

    EXPECT_EQ(b, retrievedElement->GetPropertyValueBoolean("b"));
    EXPECT_DOUBLE_EQ(d, retrievedElement->GetPropertyValueDouble("d"));
    EXPECT_TRUE(retrievedElement->GetPropertyValueDateTime("dt").Equals(dt, true));
    EXPECT_TRUE(retrievedElement->GetPropertyValueDateTime("dtUtc").Equals(dtUtc, true));
    EXPECT_EQ(p2d, retrievedElement->GetPropertyValueDPoint2d("p2d"));
    EXPECT_EQ(p3d, retrievedElement->GetPropertyValueDPoint3d("p3d"));
    EXPECT_STREQ(s.c_str(), retrievedElement->GetPropertyValueString("s").c_str());

    checkValue.Clear();
    retrievedElement->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(0u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("690 Pennsylvania Drive", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Exton", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19341, checkValue.GetInteger());

    checkValue.Clear();
    retrievedElement->GetPropertyValue(checkValue, "ArrayOfStructs", PropertyArrayIndex(1u));
    ASSERT_FALSE(checkValue.IsNull());
    ASSERT_TRUE(checkValue.IsStruct());
    checkInstance = nullptr;
    checkInstance = checkValue.GetStruct();
    checkInstance->GetValue(checkValue, "Street");
    EXPECT_STREQ("1601 Cherry Street", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Name");
    EXPECT_STREQ("Philadelphia", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.State");
    EXPECT_STREQ("PA", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Country");
    EXPECT_STREQ("US", checkValue.GetUtf8CP());
    checkInstance->GetValue(checkValue, "City.Zip");
    EXPECT_EQ(19102, checkValue.GetInteger());

    checkValue.Clear();
    checkInstance->GetValue(checkValue, "TestElementProperty");
    EXPECT_TRUE(checkValue.IsNull());
    }

    m_db->CloseDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, RelatedElementToJson)
    {
    SetupSeedProject();

    DgnElementId elementId((uint64_t)0x123456789u);
    DgnClassId relClassId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME));
    DgnElement::RelatedElement related(elementId, relClassId);

    BeJsDocument actualJson;
    related.ToJson(*m_db, actualJson);
    EXPECT_TRUE(actualJson.isObject());

    Utf8String expectedId = elementId.ToHexStr();
    EXPECT_TRUE(actualJson[ECN::ECJsonUtilities::json_navId()].isString());
    EXPECT_STREQ(expectedId.c_str(), actualJson[ECN::ECJsonUtilities::json_navId()].asCString());

    Utf8CP expectedRelClassName = DPTEST_SCHEMA_NAME ":" DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME;
    EXPECT_TRUE(actualJson[ECN::ECJsonUtilities::json_navRelClassName()].isString());
    EXPECT_STREQ(expectedRelClassName, actualJson[ECN::ECJsonUtilities::json_navRelClassName()].asCString());

    m_db->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, RelatedElementFromJson)
    {
    SetupSeedProject();

    DgnElementId expectedElementId((uint64_t)0x987654321u);
    DgnClassId expectedRelClassId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME));

    Utf8String serializedElementId = expectedElementId.ToHexStr();
    BeJsDocument json;
    json[ECN::ECJsonUtilities::json_navId()] = serializedElementId;

    // Normal relClassName format - {schemaName}.{className}
        {
        json[ECN::ECJsonUtilities::json_navRelClassName()] = DPTEST_SCHEMA_NAME "." DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME;
        DgnElement::RelatedElement related;
        related.FromJson(*m_db, json);
        EXPECT_TRUE(expectedElementId == related.m_id);
        EXPECT_EQ(expectedRelClassId, related.m_relClassId);
        }

    // Should also support legacy relClassName format - {schemaName}:{className}
        {
        json[ECN::ECJsonUtilities::json_navRelClassName()] = DPTEST_SCHEMA_NAME ":" DPTEST_TEST_ELEMENT_DRIVES_ELEMENT_CLASS_NAME;
        DgnElement::RelatedElement related;
        related.FromJson(*m_db, json);
        EXPECT_TRUE(expectedElementId == related.m_id);
        EXPECT_EQ(expectedRelClassId, related.m_relClassId);
        }

    m_db->CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DgnElementTests, ToJson)
{
    Utf8String validJsonString(R"json({
  "arrayOfStructs": [
    {
      "city": {
        "country": "US",
        "name": "Exton",
        "state": "PA",
        "zip": 19341.0
      },
      "street": "690 Pennsylvania Drive"
    },
    {
      "city": {
        "country": "US",
        "name": "Philadelphia",
        "state": "PA",
        "zip": 19102.0
      },
      "street": "1601 Cherry Street"
    }
  ],
  "b": false,
  "category": "0x12",
  "classFullName": "DgnPlatformTest:TestElementWithNoHandler",
  "code": {
    "scope": "0x1",
    "spec": "0x1",
    "value": ""
  },
  "d": 3.14,
  "dt": "2018-02-04T00:00:00.000",
  "dtUtc": "2018-02-05T00:00:00.000Z",
  "i": 50.0,
  "iGeom": {
    "lineSegment": [
      [
        -21908.999,
        4111.625,
        0.0
      ],
      [
        -22956.749,
        4111.625,
        0.0
      ]
    ]
  },
  "id": "0x14",
  "l": 100.0,
  "model": "0x11",
  "p2d": {
    "x": 3.0,
    "y": 5.0
  },
  "p3d": {
    "x": 3.0,
    "y": 5.0,
    "z": 7.0
  },
  "placement": {
    "angles": null,
    "bbox": {
      "high": [
        -1.7976931348623157e+308,
        -1.7976931348623157e+308,
        -1.7976931348623157e+308
      ],
      "low": [
        1.7976931348623157e+308,
        1.7976931348623157e+308,
        1.7976931348623157e+308
      ]
    },
    "origin": [
      0.0,
      0.0,
      0.0
    ]
  },
  "s": "test string"
})json");

    Json::Value validJson;
    EXPECT_TRUE(Json::Reader::Parse(validJsonString, validJson));

    SetupSeedProject();

    DgnElementId elementId;
    DgnClassId classId(m_db->Schemas().GetClassId(DPTEST_SCHEMA_NAME, DPTEST_TEST_ELEMENT_WITHOUT_HANDLER_CLASS_NAME));
    TestElement::CreateParams params(*m_db, m_defaultModelId, classId, m_defaultCategoryId, Placement3d(), DgnCode());
    params.SetFederationGuid(BeGuid(false));

    bool const b = false;
    double const d = 3.14;
    int32_t const i = 50;
    int64_t const l = 100;
    DateTime const dt(2018, 2, 4);
    DateTime const dtUtc(2018, 2, 5);
    auto const p2d = DPoint2d::From(3, 5);
    auto const p3d = DPoint3d::From(3, 5, 7);
    uint32_t iArrayOfStructs;

    { // Create Element
    TestElement el(params);

    Json::Value lineSegmentObj(Json::ValueType::objectValue);
    Json::Value lineSegments(Json::ValueType::arrayValue);
    Json::Value lineSegment(Json::ValueType::arrayValue);
    lineSegment[0u] = -21908.999;
    lineSegment[1u] = 4111.625;
    lineSegment[2u] = 0.0;

    lineSegments[0u] = lineSegment;

    Json::Value lineSegment2(Json::ValueType::arrayValue);
    lineSegment2[0u] = -22956.749;
    lineSegment2[1u] = 4111.625;
    lineSegment2[2u] = 0.0;

    lineSegments[1u] = lineSegment2;

    lineSegmentObj["lineSegment"] = lineSegments;

    IGeometryPtr geom = ECN::ECJsonUtilities::JsonToIGeometry(lineSegmentObj);

    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("b", b));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("d", d));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dt", dt));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("dtUtc", dtUtc));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("i", i));
    ECN::ECValue val;
    val.SetIGeometry(*geom);
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("IGeom", val));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("l", l));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("s", "test string"));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p2d", p2d));
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("p3d", p3d));

    ASSERT_EQ(DgnDbStatus::Success, el.GetPropertyIndex(iArrayOfStructs, "ArrayOfStructs"));

    EXPECT_EQ(DgnDbStatus::Success, el.InsertPropertyArrayItems(iArrayOfStructs, 0, 4));

    ECN::ECClassCP locationStruct = m_db->Schemas().GetClass(DPTEST_SCHEMA_NAME, DPTEST_TEST_LOCATION_STRUCT_CLASS_NAME);
    ASSERT_TRUE(nullptr != locationStruct);
    ECN::StandaloneECEnablerPtr locationEnabler = locationStruct->GetDefaultStandaloneEnabler();

    ECN::IECInstancePtr extonLocInstance = locationEnabler->CreateInstance().get();
    extonLocInstance->SetValue("Street", ECN::ECValue("690 Pennsylvania Drive"));
    extonLocInstance->SetValue("City.Name", ECN::ECValue("Exton"));
    extonLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    extonLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    extonLocInstance->SetValue("City.Zip", ECN::ECValue(19341));

    ECN::ECValue extonLocValue;
    extonLocValue.SetStruct(extonLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", extonLocValue, PropertyArrayIndex(1)));

    ECN::IECInstancePtr phillyLocInstance = locationEnabler->CreateInstance().get();
    phillyLocInstance->SetValue("Street", ECN::ECValue("1601 Cherry Street"));
    phillyLocInstance->SetValue("City.Name", ECN::ECValue("Philadelphia"));
    phillyLocInstance->SetValue("City.State", ECN::ECValue("PA"));
    phillyLocInstance->SetValue("City.Country", ECN::ECValue("US"));
    phillyLocInstance->SetValue("City.Zip", ECN::ECValue(19102));

    ECN::ECValue phillyLocValue;
    phillyLocValue.SetStruct(phillyLocInstance.get());
    EXPECT_EQ(DgnDbStatus::Success, el.SetPropertyValue("ArrayOfStructs", phillyLocValue, PropertyArrayIndex(2)));

    // Check that we see the stored value in memory in the correct JSON format.
    //   Needed because we handle in-memory element data using ECJsonUtilities and from Db with JsonECSqlSelectAdapter for auto-handled properties.
    // TODO: Fix ECJsonUtilities
    // EXPECT_STREQ(el.ToJson().ToString().c_str(), validJson.ToString().c_str()); // Uses ECJsonUtilities

    // Insert the element
    DgnDbStatus stat;
    DgnElementCPtr persistentEl = el.Insert(&stat);
    ASSERT_EQ(DgnDbStatus::Success, stat);
    ASSERT_TRUE(persistentEl.IsValid());

    // Check that we see the stored value in memory in the correct JSON format.
    //   Needed because we handle in-memory element data using ECJsonUtilities and from Db with JsonECSqlSelectAdapter for auto-handled properties.
    BeJsDocument jsval;
    persistentEl->ToJson(jsval);
    EXPECT_TRUE(jsval.isExactEqual(validJson));

    // persist the element into the db
    elementId = persistentEl->GetElementId();
    m_db->SaveChanges();
    }

    // Before updating the element check what is stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);

    TestElementCPtr element = m_db->Elements().Get<TestElement>(elementId);
    ASSERT_TRUE(element.IsValid());

    // Check that we see the stored value in memory in the correct JSON format.
    //   Needed because we handle in-memory element data using ECJsonUtilities and from Db with JsonECSqlSelectAdapter for auto-handled properties.
    BeJsDocument jsval;
    element->ToJson(jsval);
    EXPECT_TRUE(jsval.isExactEqual(validJson));
    m_db->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, SetParentIdValidation)
    {
    SetupSeedProject();
    PhysicalModelPtr model = GetDefaultPhysicalModel();
    DgnModelId modelId = model->GetModelId();

    DgnElementCPtr parent = TestElement::Create(*m_db, modelId, m_defaultCategoryId)->Insert();
    TestElementPtr child = TestElement::Create(*m_db, parent->GetModelId(), m_defaultCategoryId);
    ASSERT_EQ(DgnDbStatus::Success, child->SetParentId(parent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements)));

    ASSERT_EQ(DgnDbStatus::WrongClass, child->SetParentId(parent->GetElementId(), m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Document)));
    }

//----------------------------------------------------------------------------------------
// @bsiclass
//----------------------------------------------------------------------------------------
struct ImodelJsTest : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnElementTests, AutoHandledGeometryJsonRoundTrip)
    {
    DEllipse3d ellipseData = DEllipse3d::From(1, 2, 3,   0, 0, 2,   0, 3, 0,   0.0, Angle::TwoPi());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc(ellipseData);
    IGeometryPtr inGeom = IGeometry::Create(ellipse);

    const auto AreAutoHandledPropsLoaded = [](DgnElementCR inElem)
        {
        struct DgnElementExposeAutoHandledPropsLoaded : DgnElement
            { bool AreAutoHandledPropsLoaded() const { return this->m_ecPropertyData != nullptr; } };
        return reinterpret_cast<const DgnElementExposeAutoHandledPropsLoaded&>(inElem).AreAutoHandledPropsLoaded();
        };

    DgnDbPtr db;
    BeFileName dbPath;
    DgnElementId elId;
        {
        db = DgnDbTestUtils::CreateIModel(L"AutoHandledGeometryProps.db", true);
        dbPath = db->GetFileName();
        ASSERT_TRUE(db.IsValid()) << "Failed to create AutoHandledGeometryProps test dgndb";

        auto schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
        ECN::SchemaKey bisCoreSchemaKey{"BisCore", 1, 0, 0};
        auto bisCoreSchema = db->GetSchemaLocater().LocateSchema(bisCoreSchemaKey, ECN::SchemaMatchType::Latest, *schemaReadContext);
        ECN::ECSchemaPtr testSchema;
        auto testSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
                <ECSchemaReference name="Units" version="01.00.05" alias="u"/>
                <ECSchemaReference name="Formats" version="01.00.00" alias="f"/>
                <KindOfQuantity typeName="LENGTH" persistenceUnit="u:M" presentationUnits="f:DefaultRealU(2)[u:M]" relativeError="0.0001"/>
                <ECEntityClass typeName="GeomHavingClass">
                    <BaseClass>bis:SpatialLocationElement</BaseClass>
                    <ECProperty propertyName="GeomProp" typeName="Bentley.Geometry.Common.IGeometry" kindOfQuantity="LENGTH"/>
                </ECEntityClass>
            </ECSchema>
        )xml";
        ASSERT_EQ(
            ECN::SchemaReadStatus::Success,
            ECN::ECSchema::ReadFromXmlString(testSchema, testSchemaXml, *schemaReadContext)
        ) << "Failed to read test schema";

        db->ImportSchemas({testSchema.get()});

        auto model = DgnDbTestUtils::InsertPhysicalModel(*db, "ThePhysicalPartition");
        auto categoryId = DgnDbTestUtils::InsertSpatialCategory(*db, "TheSpatialCategory");

        Json::Value inPropsJson{Json::objectValue};
        inPropsJson["classFullName"] = "TestSchema:GeomHavingClass";
        char modelStringBuf[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        modelStringBuf[BeInt64Id::ID_STRINGBUFFER_LENGTH - 1] = '\0';
        char categoryStringBuf[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        categoryStringBuf[BeInt64Id::ID_STRINGBUFFER_LENGTH - 1] = '\0';
        model->GetModelId().ToString(modelStringBuf, BeInt64Id::UseHex::Yes);
        categoryId.ToString(categoryStringBuf, BeInt64Id::UseHex::Yes);
        inPropsJson["model"] = modelStringBuf;
        inPropsJson["category"] = categoryStringBuf;
        Json::Value placementJson;
        Placement3d().ToJson(BeJsValue{placementJson});
        inPropsJson["placement"] = placementJson;
        inPropsJson["federationGuid"] = "00000000-0000-0000-0000-000000000000";
        DgnCode().ToJson(inPropsJson["code"]);
        Json::Value geomJson;
        ECN::ECJsonUtilities::IGeometryToJson(geomJson, *inGeom);
        inPropsJson["geomProp"] = geomJson;

        DgnElement::CreateParams params(*db, inPropsJson);
        ASSERT_TRUE(params.m_classId.IsValid());

        ElementHandlerP elHandler = dgn_ElementHandler::Element::FindHandler(*db, params.m_classId);
        ASSERT_NE(nullptr, elHandler);

        DgnElementPtr el = elHandler->Create(params);
        ASSERT_TRUE(el.IsValid());

        el->FromJson(inPropsJson);
        elId = db->Elements().Insert(*el)->GetElementId();

        // we just inserted the element, its auto handled props are in memory/preloaded
        // previously, the code path for producing json from the in-memory IGeometry-type property would output xml
        EXPECT_TRUE(AreAutoHandledPropsLoaded(*el));
        Json::Value outPropsJson;
        el->ToJson(outPropsJson);

        IGeometryPtr outGeom = ECN::ECJsonUtilities::JsonToIGeometry(outPropsJson["geomProp"]);
        ASSERT_TRUE(outGeom.IsValid());
        EXPECT_TRUE(outGeom->IsSameStructureAndGeometry(*inGeom));

        db->SaveChanges();
        }

    // close the db after leaving the scope of any elements we picked from it, so they were definitely released
    db->CloseDb();

        {
        BeSQLite::DbResult reopenStatus;
        db = DgnDb::OpenDgnDb(&reopenStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::Readonly));
        ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, reopenStatus);

        auto el = db->Elements().GetElement(elId);

        // now when we open it without an in-memory element, its auto handled props are not in-memory/preloaded
        EXPECT_FALSE(AreAutoHandledPropsLoaded(*el));
        Json::Value outPropsJson;
        el->ToJson(outPropsJson);

        IGeometryPtr outGeom = ECN::ECJsonUtilities::JsonToIGeometry(outPropsJson["geomProp"]);
        ASSERT_TRUE(outGeom.IsValid());
        EXPECT_TRUE(outGeom->IsSameStructureAndGeometry(*inGeom));
        }

    // close the db after leaving the scope of any elements we picked from it, so they were definitely released
    db->CloseDb();
    }
