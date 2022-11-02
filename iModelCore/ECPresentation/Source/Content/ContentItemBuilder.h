/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/Content.h>
#include "../Shared/ExtendedData.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ContentValuesFormatter
{
private:
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;

private:
    static rapidjson::Document GetFallbackValue(ECPropertyCR prop, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator);

    static rapidjson::Document GetFallbackPrimitiveValue(ECPropertyCR prop, PrimitiveType type, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator);
    rapidjson::Document GetFormattedPrimitiveValue(ECPropertyCR prop, PrimitiveType type, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator) const;

    static rapidjson::Document GetFallbackStructValue(IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator);
    rapidjson::Document GetFormattedStructValue(IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator) const;

    static rapidjson::Document GetFallbackArrayValue(ArrayECPropertyCR prop, IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator);
    rapidjson::Document GetFormattedArrayValue(ArrayECPropertyCR prop, IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator) const;

public:
    ContentValuesFormatter(IECPropertyFormatter const* formatter, ECPresentation::UnitSystem unitSystem)
        : m_propertyFormatter(formatter), m_unitSystem(unitSystem)
        {}
    rapidjson::Document GetFormattedValue(ECPropertyCR prop, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator) const;
    void LocalizeString(Utf8StringR) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentItemBuilder
{
    enum class ItemReadAction
        {
        Skip,
        ReadAllFields,
        ReadNestedFields,
        CreateNewItem,
        };

    enum class BeforeAddValueStatus
        {
        Skip,
        Continue,
        };

    struct NestedContentValues
        {
        bvector<std::unique_ptr<ContentItemBuilder>> values;
        Nullable<size_t> capturedCount;
        };

    struct Context
        {
        bset<ECInstanceKey> readInstanceKeys;
        };

private:
    std::shared_ptr<Context> m_context;
    SchemaManagerCR m_schemaManager;
    ContentValuesFormatter m_formatter;
    bvector<ECInstanceKey> m_primaryKeys;
    mutable ECClassCP m_primaryInstanceClass;
    mutable bool m_determinedPrimaryInstanceClass;
    bvector<ECInstanceKey> m_inputKeys;
    rapidjson::Document m_values;
    rapidjson::Document m_displayValues;
    std::map<Utf8String, NestedContentValues> m_nestedContentValues;
    bset<Utf8String> m_mergedFieldNames;
    LabelDefinitionCPtr m_displayLabel;
    Utf8String m_imageId;
    ContentSetItemExtendedData m_extendedData;

protected:
    SchemaManagerCR GetSchemaManager() const { return m_schemaManager; }
    ContentValuesFormatter const& GetValuesFormatter() const { return m_formatter; }
    bvector<ECInstanceKey>& GetPrimaryKeysR() { return m_primaryKeys; }
    bvector<ECInstanceKey>& GetInputKeysR() { return m_inputKeys; }
    bset<Utf8String>& GetMergedFieldNames() { return m_mergedFieldNames; }
    rapidjson::Document& GetValues() { return m_values; }
    rapidjson::Document& GetDisplayValues() { return m_displayValues; }
    std::map<Utf8String, NestedContentValues>& GetNestedContentValues() { return m_nestedContentValues; }
    void InvalidateInstanceClass() { m_primaryInstanceClass = nullptr; m_determinedPrimaryInstanceClass = false; }

protected:
    virtual ItemReadAction _GetActionForPrimaryKey(ECInstanceKeyCR key) const;
    virtual void _SetPrimaryKey(ECInstanceKeyCR key) { m_primaryKeys = {key}; InvalidateInstanceClass(); }
    virtual void _SetInputKey(ECInstanceKeyCR key) { m_inputKeys.push_back(key); }
    virtual void _SetLabel(LabelDefinitionCR definition) { m_displayLabel = &definition; }
    virtual void _SetImageId(Utf8String id) { m_imageId = id; }
    virtual void _SetRelatedInstanceKeys(bvector<ContentSetItemExtendedData::RelatedInstanceKey> const& keys) { GetExtendedData().SetRelatedInstanceKeys(keys); }
    virtual BeforeAddValueStatus _OnBeforeAddValue(Utf8CP name) { return BeforeAddValueStatus::Continue; }
    virtual void _AddValue(Utf8CP name, rapidjson::Value&& value, rapidjson::Value&& displayValue, ECPropertyCP);
    virtual std::unique_ptr<ContentItemBuilder> _CreateNestedContentItemBuilder() const { return std::make_unique<ContentItemBuilder>(nullptr, m_schemaManager, m_formatter); }
    virtual void _AddEmptyNestedContentValue(Utf8CP name);
    virtual ContentItemBuilder& _AddNestedContentValue(Utf8CP name, ECInstanceKey const&);
    virtual ContentItemBuilder* _GetNestedContentValue(Utf8CP name, ECInstanceKey const&) const;
    virtual void _CaptureNestedContentValueCounts();

public:
    ContentItemBuilder(std::shared_ptr<Context> context, SchemaManagerCR schemaManager, ContentValuesFormatter formatter)
        : m_context(context ? context : std::make_shared<Context>()), m_schemaManager(schemaManager), m_formatter(formatter), m_primaryInstanceClass(nullptr), m_determinedPrimaryInstanceClass(false)
        {
        m_values.SetObject();
        m_displayValues.SetObject();
        }
    ContentItemBuilder(std::shared_ptr<Context> context, SchemaManagerCR schemaManager, IECPropertyFormatter const* propertyFormatter, ECPresentation::UnitSystem unitSystem)
        : ContentItemBuilder(context, schemaManager, ContentValuesFormatter(propertyFormatter, unitSystem))
        {}
    virtual ~ContentItemBuilder() {}

    ItemReadAction GetActionForPrimaryKey(ECInstanceKeyCR key) const { return _GetActionForPrimaryKey(key); }

    void SetPrimaryKey(ECInstanceKey const& key) { _SetPrimaryKey(key); }
    bvector<ECInstanceKey> const& GetPrimaryKeys() const { return m_primaryKeys; }
    ECClassCP GetRecordClass() const;

    void SetInputKey(ECInstanceKey const& key) { _SetInputKey(key); }
    bvector<ECInstanceKey> const& GetInputKeys() const { return m_inputKeys; }

    void SetLabel(LabelDefinitionCR definition) { _SetLabel(definition); }
    LabelDefinitionCPtr GetLabel() const { return m_displayLabel; }

    void SetImageId(Utf8String id) { _SetImageId(id); }
    Utf8StringCR GetImageId() const { return m_imageId; }

    bset<Utf8String> const& GetMergedFieldNames() const { return m_mergedFieldNames; }

    void SetRelatedInstanceKeys(bvector<ContentSetItemExtendedData::RelatedInstanceKey> const& keys) { _SetRelatedInstanceKeys(keys); }
    ContentSetItemExtendedData& GetExtendedData() { return m_extendedData; }
    ContentSetItemExtendedData const& GetExtendedData() const { return m_extendedData; }

    void AddValue(Utf8CP name, Utf8CP rawAndDisplayValue, ECPropertyCP);
    void AddValue(Utf8CP name, ECPropertyCR ecProperty, IECSqlValue const& value);
    void AddNull(Utf8CP name, ECPropertyCP);
    void AddEmptyNestedContentValue(Utf8CP name) { _AddEmptyNestedContentValue(name); }
    ContentItemBuilder& AddNestedContentValue(Utf8CP name, ECInstanceKey const& key) { return _AddNestedContentValue(name, key); }
    ContentItemBuilder* GetNestedContentValue(Utf8CP name, ECInstanceKey const& key) const { return _GetNestedContentValue(name, key); }

    ContentSetItemPtr BuildItem();
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MergingContentItemBuilder : ContentItemBuilder
{
private:
    mutable bmap<Utf8String, size_t> m_nestedContentValueCounts;

private:
    void SetNestedContentValuesAsMerged(Utf8CP name, NestedContentValues& values);

protected:
    ItemReadAction _GetActionForPrimaryKey(ECInstanceKeyCR key) const override;
    void _SetPrimaryKey(ECInstanceKey const&) override;
    void _SetInputKey(ECInstanceKey const&) override;
    void _SetLabel(LabelDefinitionCR) override;
    void _SetImageId(Utf8String id) override;
    void _SetRelatedInstanceKeys(bvector<ContentSetItemExtendedData::RelatedInstanceKey> const&) override;
    BeforeAddValueStatus _OnBeforeAddValue(Utf8CP name) override;
    void _AddValue(Utf8CP name, rapidjson::Value&& value, rapidjson::Value&& displayValue, ECPropertyCP) override;
    std::unique_ptr<ContentItemBuilder> _CreateNestedContentItemBuilder() const override { return std::make_unique<MergingContentItemBuilder>(GetSchemaManager(), GetValuesFormatter()); }
    void _AddEmptyNestedContentValue(Utf8CP name) override;
    ContentItemBuilder& _AddNestedContentValue(Utf8CP name, ECInstanceKey const&) override;
    ContentItemBuilder* _GetNestedContentValue(Utf8CP name, ECInstanceKey const&) const override;
    void _CaptureNestedContentValueCounts() override;

public:
    MergingContentItemBuilder(SchemaManagerCR schemaManager, IECPropertyFormatter const* propertyFormatter, ECPresentation::UnitSystem unitSystem)
        : ContentItemBuilder(std::make_shared<Context>(), schemaManager, ContentValuesFormatter(propertyFormatter, unitSystem))
        {}
    MergingContentItemBuilder(SchemaManagerCR schemaManager, ContentValuesFormatter formatter)
        : ContentItemBuilder(std::make_shared<Context>(), schemaManager, formatter)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentValueHelpers
{
private:
    ContentValueHelpers() {}
public:
    static bpair<LabelDefinitionCPtr, ECInstanceKey> ParseNavigationPropertyValue(IECSqlValue const& value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
