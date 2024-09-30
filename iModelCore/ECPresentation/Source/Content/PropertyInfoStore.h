/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "../Shared/ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct ConcretePropertyCategoryRef;
struct SchemaPropertyCategoryRef;
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PropertyCategoryRef
{
protected:
    virtual bool _IsRootCategoryRef() const {return false;}
    virtual bool _IsDefaultParentCategoryRef() const {return false;}
    virtual ConcretePropertyCategoryRef const* _AsConcreteCategoryRef() const {return nullptr;}
    virtual SchemaPropertyCategoryRef const* _AsSchemaCategoryRef() const {return nullptr;}
public:
    virtual ~PropertyCategoryRef() {}
    bool IsRootCategoryRef() const {return _IsRootCategoryRef();}
    bool IsDefaultParentCategoryRef() const {return _IsDefaultParentCategoryRef();}
    ConcretePropertyCategoryRef const* AsConcreteCategoryRef() const {return _AsConcreteCategoryRef();}
    SchemaPropertyCategoryRef const* AsSchemaCategoryRef() const {return _AsSchemaCategoryRef();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct VirtualPropertyCategoryRef : PropertyCategoryRef
{
private:
    enum Types { ROOT, DEFAULT_PARENT };
    Types m_type;
protected:
    bool _IsRootCategoryRef() const override {return ROOT == m_type;}
    bool _IsDefaultParentCategoryRef() const override {return DEFAULT_PARENT == m_type;}
public:
    VirtualPropertyCategoryRef(Types t) : m_type(t) {}
    static std::unique_ptr<VirtualPropertyCategoryRef> CreateRootCategoryRef() {return std::make_unique<VirtualPropertyCategoryRef>(ROOT);}
    static std::unique_ptr<VirtualPropertyCategoryRef> CreateDefaultParentCategoryRef() {return std::make_unique<VirtualPropertyCategoryRef>(DEFAULT_PARENT);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SchemaPropertyCategoryRef : PropertyCategoryRef
{
private:
    Utf8String m_categoryName;
protected:
    SchemaPropertyCategoryRef const* _AsSchemaCategoryRef() const override {return this;}
public:
    SchemaPropertyCategoryRef(Utf8String categoryName)
        : m_categoryName(std::move(categoryName))
        {}
    Utf8StringCR GetCategoryName() const {return m_categoryName;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ConcretePropertyCategoryRef : PropertyCategoryRef
{
private:
    std::unique_ptr<ContentDescriptor::Category> m_category;
protected:
    ConcretePropertyCategoryRef const* _AsConcreteCategoryRef() const override {return this;}
public:
    ConcretePropertyCategoryRef(std::unique_ptr<ContentDescriptor::Category> category)
        : m_category(std::move(category))
        {}
    ContentDescriptor::Category const& GetCategory() const {return *m_category;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CategoryOverrideInfo
{
private:
    bvector<std::shared_ptr<PropertyCategoryRef>> m_categories;
public:
    CategoryOverrideInfo(bvector<std::shared_ptr<PropertyCategoryRef>> categories)
        : m_categories(std::move(categories))
        {}
    PropertyCategoryRef const* GetRequestedCategory() const {return m_categories.empty() ? nullptr : m_categories.back().get();}
    bvector<std::shared_ptr<PropertyCategoryRef>> const& GetCategoriesStack() const {return m_categories;}
};

/*=================================================================================**//**
* @bsiclass
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
        NullablePrioritizedValue<BoolOrString> m_display;
        NullablePrioritizedValue<std::shared_ptr<ContentFieldRenderer const>> m_renderer;
        NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> m_editor;
        NullablePrioritizedValue<Utf8String> m_labelOverride;
        NullablePrioritizedValue<PropertyCategoryIdentifier const*> m_category;
        NullablePrioritizedValue<bool> m_doNotHideOtherPropertiesOnDisplayOverride;
        NullablePrioritizedValue<bool> m_isReadOnly;
        NullablePrioritizedValue<int> m_priority;
    public:
        Overrides() : m_defaultPriority(0) {}
        Overrides(int defaultPriority) : m_defaultPriority(defaultPriority) {}
        void Merge(Overrides const& other);
        void SetDisplayOverride(BoolOrString ovr) {m_display = PrioritizedValue<BoolOrString>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<BoolOrString> const& GetDisplayOverride() const {return m_display;}
        void SetRendererOverride(std::shared_ptr<ContentFieldRenderer const> ovr) {m_renderer = PrioritizedValue<std::shared_ptr<ContentFieldRenderer const>>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<std::shared_ptr<ContentFieldRenderer const>> const& GetRendererOverride() const {return m_renderer;}
        void SetEditorOverride(std::shared_ptr<ContentFieldEditor const> ovr) {m_editor = PrioritizedValue<std::shared_ptr<ContentFieldEditor const>>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& GetEditorOverride() const {return m_editor;}
        void SetLabelOverride(Utf8String ovr) {m_labelOverride = PrioritizedValue<Utf8String>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<Utf8String> const& GetLabelOverride() const {return m_labelOverride;}
        void SetCategoryOverride(PropertyCategoryIdentifier const* ovr) {m_category = PrioritizedValue<PropertyCategoryIdentifier const*>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<PropertyCategoryIdentifier const*> const& GetCategoryOverride() const {return m_category;}
        void SetDoNotHideOtherPropertiesOnDisplayOverride(bool ovr) {m_doNotHideOtherPropertiesOnDisplayOverride = PrioritizedValue<bool>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<bool> const& GetDoNotHideOtherPropertiesOnDisplayOverride() const {return m_doNotHideOtherPropertiesOnDisplayOverride;}
        void SetReadOnlyOverride(bool ovr) {m_isReadOnly = PrioritizedValue<bool>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<bool> const& GetReadOnlyOverride() const {return m_isReadOnly;}
        void SetPriorityOverride(int ovr) {m_priority = PrioritizedValue<int>({ m_defaultPriority, ovr });}
        NullablePrioritizedValue<int> const& GetPriorityOverride() const {return m_priority;}
    };

private:
    bmap<ECClassCP, Overrides> m_defaultClassPropertyOverrides;
    bmap<Utf8String, Overrides> m_propertyOverrides;
    PropertyCategorySpecificationsList m_availableCategories;

private:
    template<typename TValue>
    NullablePrioritizedValue<TValue> const& GetOverrides(ECClassCR propertyClass, Utf8StringCR propertyName, Nullable<PrioritizedValue<TValue>> const& (*valuePicker)(Overrides const&)) const;

public:
    NullablePrioritizedValue<BoolOrString> const& GetDisplayOverride(ECPropertyCR) const;
    NullablePrioritizedValue<std::shared_ptr<ContentFieldRenderer const>> const& GetContentFieldRendererOverride(ECPropertyCR) const;
    NullablePrioritizedValue<std::shared_ptr<ContentFieldRenderer const>> const& GetContentFieldRendererOverride(ECClassCR, Utf8StringCR) const;
    NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& GetContentFieldEditorOverride(ECPropertyCR) const;
    NullablePrioritizedValue<std::shared_ptr<ContentFieldEditor const>> const& GetContentFieldEditorOverride(ECClassCR, Utf8StringCR) const;
    NullablePrioritizedValue<Utf8String> const& GetLabelOverride(ECPropertyCR) const;
    NullablePrioritizedValue<PropertyCategoryIdentifier const*> const& GetCategoryOverride(ECPropertyCR) const;
    NullablePrioritizedValue<bool> const& GetDoNotHideOtherPropertiesOnDisplayOverride(ECPropertyCR) const;
    NullablePrioritizedValue<bool> const& GetReadOnlyOverride(ECPropertyCR) const;
    NullablePrioritizedValue<int> const& GetPriorityOverride(ECPropertyCR) const;
    bmap<Utf8String, Overrides> const& GetPropertyOverrides() const {return m_propertyOverrides;}
    PropertyCategorySpecificationsList const& GetAvailableCategories() const { return m_availableCategories; }
    void SetClassOverrides(ECClassCP ecClass, Overrides ovr) {m_defaultClassPropertyOverrides[ecClass] = ovr;}
    void SetPropertyOverrides(Utf8StringCR ecPropertyName, Overrides ovr) {m_propertyOverrides[ecPropertyName] = ovr;}
    void SetAsAvailableCategories(PropertyCategorySpecificationsList const& categories) { m_availableCategories = categories; }
    void Merge(ClassPropertyOverridesInfo const&);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PropertyInfoStore
{
    struct CategoryOverridesCacheKey
    {
    private:
        PropertyCategoryIdentifierType m_type;
        Utf8String m_categoryId;
        Utf8String m_className;
    public:
        CategoryOverridesCacheKey(PropertyCategoryIdentifier const& identifier, Utf8String className) : m_type(identifier.GetType()), m_className(className)
            {
            if (auto idIdentifier = identifier.AsIdIdentifier())
                m_categoryId = Utf8PrintfString("id: %s", idIdentifier->GetCategoryId().c_str());
            else if (auto schemaCategoryIdentifier = identifier.AsSchemaCategoryIdentifier())
                m_categoryId = Utf8PrintfString("sc: %s", schemaCategoryIdentifier->GetCategoryName().c_str());
            else
                m_categoryId = "";
            }
        bool operator<(CategoryOverridesCacheKey const& other) const
            {
            NUMERIC_LESS_COMPARE(m_type, other.m_type);
            STR_LESS_COMPARE(m_categoryId.c_str(), other.m_categoryId.c_str());
            STR_LESS_COMPARE(m_className.c_str(), other.m_className.c_str());
            return false;
            }
    };

private:
    ECSchemaHelper const& m_schemaHelper;
    IPropertyCategorySupplierCP m_categorySupplier;
    bmap<ECClassCP, ClassPropertyOverridesInfo> m_perClassPropertyOverrides;
    mutable bmap<ECClassCP, ClassPropertyOverridesInfo> m_aggregatedOverrides; // property overrides, including base class properties
    mutable bmap<CategoryOverridesCacheKey, CategoryOverrideInfo> m_categoryOverridesCache;

private:
    void CollectPropertyOverrides(ECClassCP ecClass, PropertySpecificationCR spec);
    void CollectCategories(ECClassCP ecClass, PropertyCategorySpecificationsList const& categorySpecifications);
    void InitPropertyOverrides(ContentSpecificationCP specification, bvector<ContentModifierCP> const& contentModifiers);
    ClassPropertyOverridesInfo const& GetOverrides(ECClassCR ecClass) const;
    CategoryOverrideInfo const* GetCategoryOverride(PropertyCategoryIdentifier const&, ECClassCP, PropertyCategorySpecificationsList const&) const;

public:
    PropertyInfoStore(ECSchemaHelper const& helper, bvector<ContentModifierCP> const& contentModifiers, ContentSpecificationCP spec, IPropertyCategorySupplierCP categorySupplier);
    bool ShouldDisplay(ECPropertyCR, ECClassCR, std::function<ExpressionContextPtr()> const& expressionContextFactory, PropertySpecificationsList const& = PropertySpecificationsList()) const;
    std::shared_ptr<ContentFieldRenderer const> GetPropertyRenderer(ECClassCR, Utf8StringCR propertyName, PropertySpecificationCP = nullptr) const;
    std::shared_ptr<ContentFieldEditor const> GetPropertyEditor(ECClassCR, Utf8StringCR propertyName, PropertySpecificationCP = nullptr) const;
    Utf8String GetLabelOverride(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr) const;
    CategoryOverrideInfo const* GetCategoryOverride(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr, PropertyCategorySpecificationsList const* = nullptr) const;
    CategoryOverrideInfo const* GetCategoryOverride(ECClassCP, CalculatedPropertiesSpecificationCR) const;
    Nullable<bool> GetReadOnlyOverride(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr) const;
    Nullable<int32_t> GetPriorityOverride(ECPropertyCR, ECClassCR, PropertySpecificationCP = nullptr) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
