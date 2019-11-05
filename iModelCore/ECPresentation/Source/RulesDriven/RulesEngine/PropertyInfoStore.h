/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include "ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
+===============+===============+===============+===============+===============+======*/
struct ClassPropertyOverridesInfo
{
    template<typename TValue>
    struct PrioritizedValue
        {
        int priority;
        TValue value;
        };

    template<typename TValue>
    using NullablePrioritizedValue = Nullable<PrioritizedValue<TValue>>;

    struct Overrides
    {
    private:
        int m_defaultPriority;
        NullablePrioritizedValue<bool> m_display;
        NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> m_editor;
        NullablePrioritizedValue<Utf8String> m_labelOverride;
        NullablePrioritizedValue<ContentDescriptor::Category> m_category;
    public:
        Overrides() : m_defaultPriority(0) {}
        Overrides(int defaultPriority) : m_defaultPriority(defaultPriority) {}
        void Merge(Overrides const& other);
        void SetDisplayOverride(bool ovr) {m_display = PrioritizedValue<bool>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<bool> const& GetDisplayOverride() const {return m_display;}
        void SetEditorOverride(std::shared_ptr<ContentFieldEditor const> ovr) {m_editor = PrioritizedValue<std::shared_ptr<ContentFieldEditor const>>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& GetEditorOverride() const {return m_editor;}
        void SetLabelOverride(Utf8String ovr) {m_labelOverride = PrioritizedValue<Utf8String>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<Utf8String> const& GetLabelOverride() const {return m_labelOverride;}
        void SetCategoryOverride(ContentDescriptor::Category ovr) {m_category = PrioritizedValue<ContentDescriptor::Category>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<ContentDescriptor::Category> const& GetCategoryOverride() const {return m_category;}
    };

private:
    bmap<ECClassCP, Overrides> m_defaultClassPropertyOverrides;
    bmap<Utf8String, Overrides> m_propertyOverrides;

private:
    template<typename TValue>
    NullablePrioritizedValue<TValue> const& GetOverrides(ECPropertyCR, Nullable<PrioritizedValue<TValue>> const&(*valuePicker)(Overrides const&)) const;

public:
    NullablePrioritizedValue<bool> const& GetDisplayOverride(ECPropertyCR) const;
    NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& GetContentFieldEditorOverride(ECPropertyCR) const;
    NullablePrioritizedValue<Utf8String> const& GetLabelOverride(ECPropertyCR) const;
    NullablePrioritizedValue<ContentDescriptor::Category> const& GetCategoryOverride(ECPropertyCR) const;
    bmap<Utf8String, Overrides> const& GetPropertyOverrides() const {return m_propertyOverrides;}
    void SetClassOverrides(ECClassCP ecClass, Overrides ovr) {m_defaultClassPropertyOverrides[ecClass] = ovr;}
    void SetPropertyOverrides(Utf8StringCR ecPropertyName, Overrides ovr) {m_propertyOverrides[ecPropertyName] = ovr;}
    void Merge(ClassPropertyOverridesInfo const&);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct PropertyInfoStore
{
private:
    ECSchemaHelper const& m_schemaHelper;
    bmap<ECClassCP, ClassPropertyOverridesInfo> m_perClassPropertyOverrides;
    mutable bmap<ECClassCP, ClassPropertyOverridesInfo> m_aggregatedOverrides; // property overrides, including base class properties

private:
    void CollectPropertyOverrides(ECClassCP ecClass, PropertySpecificationCR spec, PropertyCategorySpecificationsList const&);
    void InitPropertyOverrides(ContentSpecificationCP specification, ContentModifierList const& contentModifiers);
    ClassPropertyOverridesInfo const& GetOverrides(ECClassCR ecClass) const;
    
public:
    PropertyInfoStore(ECSchemaHelper const& helper, PresentationRuleSetCR ruleset, ContentSpecificationCP spec);
    bool ShouldDisplay(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr) const;
    std::shared_ptr<ContentFieldEditor const> GetPropertyEditor(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr) const;
    Utf8String GetLabelOverride(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr) const;
    ContentDescriptor::Category GetCategoryOverride(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr, PropertyCategorySpecificationsList const* = nullptr) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
