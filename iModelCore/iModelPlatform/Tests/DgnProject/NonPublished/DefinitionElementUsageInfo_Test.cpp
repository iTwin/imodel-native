/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DPTEST

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DefinitionElementUsageInfoTests : public DgnDbTestFixture
    {
    DgnElementId m_borderId;
    DgnElementId m_prototypeId;
    DgnElementId m_sheetId;

    DgnElementId InsertDefinition(Utf8CP className, Utf8CP label)
        {
        DgnClassId classId = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, className);
        auto handler = dgn_ElementHandler::Element::FindHandler(*m_db, classId);
        if (nullptr == handler)
            return DgnElementId();

        DgnElement::CreateParams params(*m_db, DgnModel::DictionaryId(), classId, DgnCode::CreateEmpty(), label);
        auto element = handler->Create(params);
        if (!element.IsValid())
            return DgnElementId();
        return element->Insert().IsValid() ? element->GetElementId() : DgnElementId();
        }

    bool InsertSheetHierarchy()
        {
        if (nullptr == m_db->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_SheetPrototype))
            return false;

        m_borderId = InsertDefinition(BIS_CLASS_SheetBorderPrototype, "B");
        m_prototypeId = InsertDefinition(BIS_CLASS_SheetPrototype, "P");
        if (!m_borderId.IsValid() || !m_prototypeId.IsValid())
            return false;

        auto prototype = m_db->Elements().GetForEdit<DgnElement>(m_prototypeId);
        DgnClassId borderRel = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, "SheetPrototypeHasSheetBorderPrototype");
        if (DgnDbStatus::Success != prototype->SetPropertyValue("Border", m_borderId, borderRel) || DgnDbStatus::Success != prototype->Update())
            return false;

        DocumentListModelPtr sheetList = DgnDbTestUtils::InsertDocumentListModel(*m_db, "SheetList");
        auto sheet = Sheet::Element::Create(*sheetList, 1.0, DPoint2d::From(1.0, 1.0), "S");
        DgnClassId sheetRel = m_db->Schemas().GetClassId(BIS_ECSCHEMA_NAME, "SheetHasSheetPrototype");
        if (DgnDbStatus::Success != sheet->SetPropertyValue("SheetPrototype", m_prototypeId, sheetRel))
            return false;
        auto persistent = sheet->Insert();
        m_sheetId = persistent.IsValid() ? persistent->GetElementId() : DgnElementId();
        return m_sheetId.IsValid();
        }

    DgnElementIdSet UsedIds(DgnElementIdSet const& ids, std::shared_ptr<BeSQLite::IdSet<BeInt64Id>> exclude = nullptr)
        {
        auto info = DefinitionElementUsageInfo::Create(*m_db, ids, exclude);
        return info.IsValid() ? info->GetUsedIds() : DgnElementIdSet();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefinitionElementUsageInfoTests, SheetPrototypeAndBorderUsage)
    {
    SetupSeedProject();
    ASSERT_TRUE(InsertSheetHierarchy()) << "BisCore must contain SheetPrototype (>= 01.00.27)";

    DgnElementIdSet both;
    both.insert(m_borderId);
    both.insert(m_prototypeId);

    // Referenced prototype and border are in use, and stay in the "other" bucket
    DgnElementIdSet used = UsedIds(both);
    EXPECT_TRUE(used.Contains(m_borderId));
    EXPECT_TRUE(used.Contains(m_prototypeId));

    BeJsDocument json;
    DefinitionElementUsageInfo::Create(*m_db, both)->ToJson(json);
    BeJsConst other = json["otherDefinitionElementIds"];
    ASSERT_TRUE(other.isArray());
    ASSERT_EQ(2u, other.size());
    bool hasBorder = false, hasPrototype = false;
    other.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst id)
        {
        hasBorder |= (id.asString() == m_borderId.ToHexStr());
        hasPrototype |= (id.asString() == m_prototypeId.ToHexStr());
        return false;
        });
    EXPECT_TRUE(hasBorder);
    EXPECT_TRUE(hasPrototype);

    // Excluding the referencers makes them unused
    auto excludeSheet = std::make_shared<BeSQLite::IdSet<BeInt64Id>>();
    excludeSheet->insert(m_sheetId);
    used = UsedIds(both, excludeSheet);
    EXPECT_FALSE(used.Contains(m_prototypeId));
    EXPECT_TRUE(used.Contains(m_borderId));

    auto excludePrototype = std::make_shared<BeSQLite::IdSet<BeInt64Id>>();
    excludePrototype->insert(m_prototypeId);
    used = UsedIds(both, excludePrototype);
    EXPECT_FALSE(used.Contains(m_borderId));
    EXPECT_TRUE(used.Contains(m_prototypeId));

    // Unreferenced prototype/border are not in use
    ASSERT_EQ(DgnDbStatus::Success, m_db->Elements().Get<DgnElement>(m_sheetId)->Delete());
    used = UsedIds(both);
    EXPECT_FALSE(used.Contains(m_prototypeId));
    EXPECT_TRUE(used.Contains(m_borderId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefinitionElementUsageInfoTests, BulkDeleteHonorsSheetPrototypeUsage)
    {
    SetupSeedProject();
    ASSERT_TRUE(InsertSheetHierarchy()) << "BisCore must contain SheetPrototype (>= 01.00.27)";

    // Prototype is referenced by Sheet, which is outside the delete set
    DgnElementIdSet ids;
    ids.insert(m_prototypeId);
    auto result = m_db->Elements().DeleteElements(ids);
    EXPECT_EQ(BulkDeleteElementsStatus::ConstraintViolation, result.status);
    ASSERT_EQ(1u, result.failures.size());
    EXPECT_EQ(m_prototypeId, result.failures[0].elementId);
    EXPECT_EQ(BulkDeleteFailureReason::DefinitionInUse, result.failures[0].reason);
    EXPECT_TRUE(m_db->Elements().GetElement(m_prototypeId).IsValid());

    // The reference must not have been silently cleared
    DgnElementCPtr sheet = m_db->Elements().GetElement(m_sheetId);
    ASSERT_TRUE(sheet.IsValid());
    EXPECT_EQ(m_prototypeId, sheet->GetPropertyValueId<DgnElementId>("SheetPrototype"));

    // {Prototype, Border} with Sheet outside: nothing is deleted, so P keeps its Border
    ids.insert(m_borderId);
    result = m_db->Elements().DeleteElements(ids);
    EXPECT_EQ(BulkDeleteElementsStatus::ConstraintViolation, result.status);
    EXPECT_TRUE(m_db->Elements().GetElement(m_prototypeId).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(m_borderId).IsValid());

    // {Sheet, Prototype}: every referencer is in the delete set
    ids.clear();
    ids.insert(m_prototypeId);
    ids.insert(m_sheetId);
    result = m_db->Elements().DeleteElements(ids);
    EXPECT_EQ(BulkDeleteElementsStatus::Success, result.status);
    EXPECT_FALSE(m_db->Elements().GetElement(m_sheetId).IsValid());
    EXPECT_FALSE(m_db->Elements().GetElement(m_prototypeId).IsValid());
    EXPECT_TRUE(m_db->Elements().GetElement(m_borderId).IsValid());

    // Border is no longer referenced
    DgnElementIdSet border;
    border.insert(m_borderId);
    result = m_db->Elements().DeleteElements(border);
    EXPECT_EQ(BulkDeleteElementsStatus::Success, result.status);
    EXPECT_FALSE(m_db->Elements().GetElement(m_borderId).IsValid());
    }
