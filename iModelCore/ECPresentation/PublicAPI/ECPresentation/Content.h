/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/DataSource.h>
#include <ECPresentation/ExtendedData.h>
#include <ECPresentation/KeySet.h>
#include <ECPresentation/LabelDefinition.h>
#include <ECPresentation/RulesetVariables.h>
#include <ECPresentation/Rules/RelatedPropertiesSpecification.h>
#include <ECPresentation/Rules/InstanceLabelOverride.h>

#include <ECDb/ECInstanceId.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define CONTENTRECORD_MERGED_VALUE_FORMAT   "*** %s ***"

//=======================================================================================
//! A struct that describes current selection.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct SelectionInfo : RefCountedBase
{
private:
    Utf8String m_selectionProviderName;
    bool m_isSubSelection;
    uint64_t m_timestamp;

protected:
    SelectionInfo(Utf8String providerName, bool isSubSelection, uint64_t timestamp)
        : m_selectionProviderName(providerName), m_isSubSelection(isSubSelection), m_timestamp(timestamp)
        {}
    ~SelectionInfo() {}

public:
    //! Create a new instance of SelectionInfo
    //! @param[in] providerName Name of the selection provider which last changed the selection.
    //! @param[in] isSubSelection Did the last selection change happen in sub-selection.
    //! @param[in] timestamp Timestamp of when the selection event happened.
    static SelectionInfoPtr Create(Utf8String providerName, bool isSubSelection, uint64_t timestamp = BeTimeUtilities::GetCurrentTimeAsUnixMillis())
        {
        return new SelectionInfo(providerName, isSubSelection, timestamp);
        }

    //! Compare this selection event info object with the supplied one.
    ECPRESENTATION_EXPORT bool operator==(SelectionInfo const& other) const;
    bool operator!=(SelectionInfo const& other) const {return !operator==(other);}

    //! Compare this selection event info object with the supplied one.
    ECPRESENTATION_EXPORT bool operator<(SelectionInfo const& other) const;

    //! Get the name of the selection source which caused the last selection change.
    Utf8StringCR GetSelectionProviderName() const {return m_selectionProviderName;}

    //! Did the last selection change happen in sub-selection.
    bool IsSubSelection() const {return m_isSubSelection;}

    // Timestamp of when the selection event happened.
    uint64_t GetTimestamp() const {return m_timestamp;}
};

//=======================================================================================
//! Flags that control content format.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
enum class ContentFlags
    {
    KeysOnly =              1,      //!< Each content record has only ECInstanceKey and no data
    ShowImages =            1 << 1, //!< Each content record additionally has an image id
    ShowLabels =            1 << 2, //!< Each content record additionally has a label
    MergeResults =          1 << 3, //!< All content records are merged into a single record
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    DistinctValues =        1 << 4, //!< Content has only distinct values
#endif
    NoFields =              1 << 5, //!< Doesnt create property or calculated fields. Can be used in conjunction with @e ShowLabels.
    ExcludeEditingData =    1 << 6, //!< Should editing data be excluded from the content. This flag increases performance and should be used when requesting data for read-only cases.
    SkipInstancesCheck =    1 << 7, //!< Skip instances' check when creating content with ContentRelatedInstances specification.
    IncludeInputKeys =      1 << 8, //!< Should ContentSetItems be associated with related input keys
    DescriptorOnly =        1 << 9, //!< Produce content descriptor that is not intended for querying content. Allows the implementation to omit certain operations to make obtaining content descriptor faster.
    };

//=======================================================================================
//! Displayed content types. Affects how the content is formatted, e.g.
//! the ContentFlags
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct ContentDisplayType
    {
    //! Unknown content type.
    ECPRESENTATION_EXPORT static const Utf8CP Undefined;

    //! Grid or table view content type. By default adds `ShowLabels` flag.
    ECPRESENTATION_EXPORT static const Utf8CP Grid;

    //! Property pane content type. By default adds `MergeResults` flag.
    ECPRESENTATION_EXPORT static const Utf8CP PropertyPane;

    //! List view content type. By default adds `NoFields`, `KeysOnly`, `ShowLabels` and `SkipInstancesCheck` flags.
    ECPRESENTATION_EXPORT static const Utf8CP List;

    //! Content type for graphic content controls, e.g. the viewport.
    //! By default adds `NoFields`, `KeysOnly` and `SkipInstancesCheck` flags.
    ECPRESENTATION_EXPORT static const Utf8CP Graphics;
    };

//=======================================================================================
//! Data structure that describes an ECClass in ContentDescriptor. In addition to the class
//! itself the structure holds its relationship path to the primary ECClass, paths to related
//! and navigation property classes and classes of related instances.
//!
//! Dependencies are related as follows:
//!
//!                                                                 +----------------------------+
//!                                        paths to related         | Related Properties Path 1  |
//!                                        property classes         | Related Properties Path 2  |
//!                                      +------------------------> | ...                        |
//!                                      |                          | Related Properties Path n  |
//!                                      |                          +----------------------------+
//!                                      | paths to related         | Related Instance Class 1   |
//!                                      | instance classes         | Related Instance Class 2   |
//! Input Class <---------- Select Class +------------------------> | ...                        |
//!            path to input             |                          | Related Instance Class n   |
//!                class                 |                          +----------------------------+
//!                                      | paths to navigation      | Navigation Property Path 1 |
//!                                      | property target classes  | Navigation Property Path 2 |
//!                                      +------------------------> | ...                        |
//!                                                                 | Navigation Property Path n |
//!                                                                 +----------------------------+
//!
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct SelectClassInfo
{
private:
    SelectClassWithExcludes<ECClass> m_selectClass;
    RelatedClassPath m_pathFromInputToSelectClass;
    bvector<RelatedClassPath> m_relatedPropertyPaths;
    bvector<RelatedClass> m_navigationPropertyClasses;
    bvector<RelatedClassPath> m_relatedInstancePaths;

public:
    //! Constructor. Creates an invalid object.
    SelectClassInfo() {}
    //! Constructor. Creates an information instance with the specified ECClass.
    SelectClassInfo(SelectClassWithExcludes<ECClass> const& selectClass) : m_selectClass(selectClass) {}
    SelectClassInfo(SelectClassWithExcludes<ECClass>&& selectClass) : m_selectClass(std::move(selectClass)) {}
    SelectClassInfo(ECClassCR ecClass, Utf8String alias, bool isSelectPolymorphic) : m_selectClass(SelectClassWithExcludes<ECClass>(ecClass, alias, isSelectPolymorphic)) {}
    //! Returns whether this info is equal to the supplied one.
    bool Equals(SelectClassInfo const& other) const
        {
        return m_selectClass == other.m_selectClass
            && m_pathFromInputToSelectClass == other.m_pathFromInputToSelectClass
            && m_relatedPropertyPaths == other.m_relatedPropertyPaths
            && m_navigationPropertyClasses == other.m_navigationPropertyClasses
            && m_relatedInstancePaths == other.m_relatedInstancePaths;
        }
    //! Equals operator override.
    bool operator==(SelectClassInfo const& other) const {return Equals(other);}
    //! NotEquals operator override.
    bool operator!=(SelectClassInfo const& other) const {return !Equals(other);}

    //! Get the select ECClass.
    SelectClassWithExcludes<ECClass> const& GetSelectClass() const {return m_selectClass;}
    SelectClassWithExcludes<ECClass>& GetSelectClass() {return m_selectClass;}

    //! Get the input ECClass.
    ECClassCP GetInputClass() const {return m_pathFromInputToSelectClass.empty() ? nullptr : m_pathFromInputToSelectClass.front().GetSourceClass();}

    //! Path from input to select ECClass.
    RelatedClassPath const& GetPathFromInputToSelectClass() const {return m_pathFromInputToSelectClass;}
    SelectClassInfo& SetPathFromInputToSelectClass(RelatedClassPath path) {m_pathFromInputToSelectClass = path; return *this;}

    //! Get paths to related property ECClasses.
    bvector<RelatedClassPath> const& GetRelatedPropertyPaths() const {return m_relatedPropertyPaths;}
    bvector<RelatedClassPath>& GetRelatedPropertyPaths() {return m_relatedPropertyPaths;}
    //! Set paths to related property ECClasses.
    SelectClassInfo& SetRelatedPropertyPaths(bvector<RelatedClassPath> propertyPaths) {m_relatedPropertyPaths = propertyPaths; return *this;}

    //! Get navigation property ECClasses.
    bvector<RelatedClass> const& GetNavigationPropertyClasses() const {return m_navigationPropertyClasses;}
    bvector<RelatedClass>& GetNavigationPropertyClasses() {return m_navigationPropertyClasses;}
    //! Set navigation property ECClasses.
    SelectClassInfo& SetNavigationPropertyClasses(bvector<RelatedClass> classes) {m_navigationPropertyClasses = classes; return *this;}

    //! Get related classes of related instances.
    bvector<RelatedClassPath> const& GetRelatedInstancePaths() const {return m_relatedInstancePaths;}
    bvector<RelatedClassPath>& GetRelatedInstancePaths() {return m_relatedInstancePaths;}
    //! Set related classes of related instances.
    SelectClassInfo& SetRelatedInstancePaths(bvector<RelatedClassPath> paths) {m_relatedInstancePaths = paths; return *this;}
};

//=======================================================================================
//! Describes content field renderer.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct ContentFieldRenderer
{
private:
    Utf8String m_name;

public:
    ContentFieldRenderer(Utf8String name) : m_name(name) {}

    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
    Utf8StringCR GetName() const {return m_name;}
    ECPRESENTATION_EXPORT bool operator<(ContentFieldRenderer const&) const;

    static ContentFieldRenderer* FromSpec(CustomRendererSpecificationCR);
};

//=======================================================================================
//! Describes content field editor.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct ContentFieldEditor
{
    struct Params
    {
    protected:
        virtual Utf8CP _GetName() const = 0;
        virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType*) const = 0;
        virtual Params* _Clone() const = 0;
        virtual int _CompareTo(Params const& other) const {return strcmp(GetName(), other.GetName());}
    public:
        virtual ~Params() {}
        Utf8CP GetName() const {return _GetName();}
        rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, allocator);}
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
        Params* Clone() const {return _Clone();}
        bool Equals(Params const& other) const {return _CompareTo(other) == 0;}
        bool operator==(Params const& other) const {return Equals(other);}
        bool operator<(Params const& other) const {return _CompareTo(other) < 0;}
    };

private:
    Utf8String m_name;
    bvector<Params const*> m_params;

public:
    ContentFieldEditor() {}
    ContentFieldEditor(Utf8String name) : m_name(name) {}
    ContentFieldEditor(ContentFieldEditor const& other)
        : m_name(other.m_name)
        {
        for (Params const* otherParams : other.m_params)
            m_params.push_back(otherParams->Clone());
        }
    ContentFieldEditor(ContentFieldEditor&& other)
        : m_name(std::move(other.m_name)), m_params(other.m_params)
        {
        other.m_params.clear();
        }
    static ContentFieldEditor* FromSpec(PropertyEditorSpecificationCR spec);
    ECPRESENTATION_EXPORT ~ContentFieldEditor();
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT bool Equals(ContentFieldEditor const&) const;
    ECPRESENTATION_EXPORT bool operator<(ContentFieldEditor const&) const;
    ECPRESENTATION_EXPORT ContentFieldEditor& operator=(ContentFieldEditor const&);
    ECPRESENTATION_EXPORT ContentFieldEditor& operator=(ContentFieldEditor&&);
    bvector<Params const*> const& GetParams() const {return m_params;}
    bvector<Params const*>& GetParams() {return m_params;}
    Utf8StringCR GetName() const {return m_name;}
};

//=======================================================================================
//! Describes the content: fields, sorting, filtering, format. Users may change
//! @ref ContentDescriptor to control what content they get and how they get it.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ContentDescriptor : RefCountedBase
{
    //===================================================================================
    //! A struct that describes a @ref Field category.
    // @bsiclass
    //===================================================================================
    struct Category : std::enable_shared_from_this<Category>
    {
    private:
        Utf8String m_name;
        Utf8String m_label;
        Utf8String m_description;
        Utf8String m_rendererName;
        bool m_shouldExpand;
        int m_priority;
        std::weak_ptr<Category> m_parentCategory;
        bvector<std::shared_ptr<Category>> m_childCategories;

    public:
        //! Constructor. Creates an invalid category.
        Category() : m_shouldExpand(false), m_priority(0) {}

        //! Constructor.
        //! @param[in] name Name of the category.
        //! @param[in] label Label of the category.
        //! @param[in] description Description of the category.
        //! @param[in] priority Priority of the category.
        //! @param[in] shouldExpand Should this category be auto-expanded.
        Category(Utf8String name, Utf8String label, Utf8String description, int priority, bool shouldExpand = false, Utf8String rendererName = "")
            : m_name(name), m_label(label), m_priority(priority), m_description(description), m_shouldExpand(shouldExpand), m_rendererName(rendererName)
            {}

        //! Is this category equal to the supplied one.
        ECPRESENTATION_EXPORT bool Equals(Category const& other, bool compareParents = true, bool compareChildren = true) const;
        bool operator==(Category const& other) const {return Equals(other);}
        bool operator!=(Category const& other) const {return !Equals(other);}

        //! Is this category valid
        bool IsValid() const {return !m_name.empty();}

        //! Serialize this category to JSON.
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

        //! Get the name of the category.
        Utf8StringCR GetName() const {return m_name;}
        //! Set the name of the category.
        void SetName(Utf8String value) {m_name = value;}

        //! Get the label of the category.
        Utf8StringCR GetLabel() const {return m_label;}
        //! Set the label of the category.
        void SetLabel(Utf8String value) {m_label = value;}

        //! Get the priority of the category.
        int GetPriority() const {return m_priority;}
        //! Set the priority of the category.
        void SetPriority(int value) {m_priority = value;}

        //! Get the description of the category.
        Utf8StringCR GetDescription() const {return m_description;}
        //! Set the description of the category.
        void SetDescription(Utf8String value) {m_description = value;}

        //! Get custom renderer name for this category.
        Utf8StringCR GetRendererName() const {return m_rendererName;}
        //! Set custom renderer name for this category.
        void SetRendererName(Utf8String value) {m_rendererName = value;}

        //! Should this category be automatically expanded.
        bool ShouldExpand() const {return m_shouldExpand;}
        //! Set whether this category should be automatically expanded.
        void SetShouldExpand(bool value) {m_shouldExpand = value;}

        Category const* GetParentCategory() const {return m_parentCategory.lock().get();}
        void SetParentCategory(std::shared_ptr<Category> newParentCategory)
            {
            auto oldParentCategory = m_parentCategory.lock();
            if (newParentCategory != oldParentCategory)
                {
                auto thisPtr = shared_from_this();
                if (oldParentCategory)
                    oldParentCategory->GetChildCategories().erase(std::remove(oldParentCategory->GetChildCategories().begin(), oldParentCategory->GetChildCategories().end(), thisPtr));
                m_parentCategory = newParentCategory;
                if (newParentCategory)
                    newParentCategory->GetChildCategories().push_back(thisPtr);
                }
            }

        bvector<std::shared_ptr<Category>> const& GetChildCategories() const {return m_childCategories;}
        bvector<std::shared_ptr<Category>>& GetChildCategories() {return m_childCategories;}
    };

    //===================================================================================
    //! Describes a single ECProperty that's included in a @ref Field.
    // @bsiclass
    //===================================================================================
    struct Property
    {
        ECPRESENTATION_EXPORT static int const DEFAULT_PRIORITY;
    private:
        Utf8String m_prefix;
        ECClassCP m_propertyClass;
        ECPropertyCP m_property;

    private:
        ECPRESENTATION_EXPORT static PrimitiveECPropertyCP GetPrimitiveProperty(StructECPropertyCR, Utf8StringCR accessString);

    public:
        //! Constructor. Creates a property for a primitive ECProperty.
        //! @param[in] prefix Class alias that's used to query this property.
        //! @param[in] propertyClass The exact class of this property.
        //! @param[in] ecProperty The ECProperty that's wrapped by this struct.
        Property(Utf8String prefix, ECClassCR propertyClass, ECPropertyCR ecProperty)
            : m_prefix(prefix), m_propertyClass(&propertyClass), m_property(&ecProperty)
            {}

        //! Is this struct equal to the supplied one.
        ECPRESENTATION_EXPORT bool operator==(Property const& other) const;

        //! Serialize this struct to JSON.
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

        //! Get the class alias used to query this property.
        Utf8CP GetPrefix() const {return m_prefix.c_str();}
        //! Set the class alias used to query this property.
        void SetPrefix(Utf8String prefix) {m_prefix = prefix;}

        // Get the exact class that the root wrapped property belongs to.
        ECClassCR GetPropertyClass() const {return *m_propertyClass;}

        //! Get the wrapped property.
        ECPropertyCR GetProperty() const {return *m_property;}

        //! Get the priority of this property.
        int GetPriority() const {return m_property->GetPriority();}
    };

    struct DisplayLabelField;
    struct ECPropertiesField;
    struct CalculatedPropertyField;
    struct NestedContentField;
    //===================================================================================
    //! Describes a single content field. A field is usually represented as a grid column
    //! or a property pane row.
    // @bsiclass
    //===================================================================================
    struct Field
    {
        struct EXPORT_VTABLE_ATTRIBUTE TypeDescription : RefCountedBase
            {
            private:
                Utf8String m_typeName;
            protected:
                TypeDescription(Utf8String typeName) : m_typeName(typeName) {}
                ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const;
            public:
                ECPRESENTATION_EXPORT static RefCountedPtr<TypeDescription> Create(ECPropertyCR);
                Utf8StringCR GetTypeName() const {return m_typeName;}
                rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, allocator);}
                ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
        };
        typedef RefCountedPtr<TypeDescription> TypeDescriptionPtr;

        //===================================================================================
        // @bsiclass
        //===================================================================================
        struct PrimitiveTypeDescription : TypeDescription
        {
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        public:
            PrimitiveTypeDescription(Utf8String type) : TypeDescription(type) {}
        };

        //===================================================================================
        // @bsiclass
        //===================================================================================
        struct ArrayTypeDescription : TypeDescription
        {
        private:
            TypeDescriptionPtr m_memberType;
        private:
            ECPRESENTATION_EXPORT static Utf8String CreateTypeName(TypeDescription const& memberType);
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        public:
            ArrayTypeDescription(TypeDescription& memberType) : TypeDescription(CreateTypeName(memberType)), m_memberType(&memberType) {}
            TypeDescriptionPtr GetMemberType() const {return m_memberType;}
        };

        //===================================================================================
        // @bsiclass
        //===================================================================================
        struct StructTypeDescription : TypeDescription
        {
        private:
            ECStructClassCR m_struct;
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        public:
            StructTypeDescription(ECStructClassCR structClass) : TypeDescription(structClass.GetName()), m_struct(structClass) {}
            ECStructClassCR GetStruct() const {return m_struct;}
        };

        //===================================================================================
        // @bsiclass
        //===================================================================================
        struct NestedContentTypeDescription : TypeDescription
        {
        private:
            NestedContentField const& m_field;
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        public:
            NestedContentTypeDescription(ContentDescriptor::NestedContentField const& field) : TypeDescription(field.GetContentClass().GetDisplayLabel()), m_field(field) {}
            NestedContentField const& GetNestedContentField() const {return m_field;}
        };

    private:
        std::shared_ptr<Category const> m_category;
        Utf8String m_uniqueName;
        Utf8String m_label;
        ContentFieldRenderer const *m_renderer;
        ContentFieldEditor const* m_editor;
        NestedContentField const* m_parent;
        mutable TypeDescriptionPtr m_type;

    protected:
        Field() : m_renderer(nullptr), m_editor(nullptr) {}
        virtual DisplayLabelField* _AsDisplayLabelField() {return nullptr;}
        virtual DisplayLabelField const* _AsDisplayLabelField() const {return nullptr;}
        virtual CalculatedPropertyField* _AsCalculatedPropertyField() {return nullptr;}
        virtual CalculatedPropertyField const* _AsCalculatedPropertyField() const {return nullptr;}
        virtual ECPropertiesField* _AsPropertiesField() {return nullptr;}
        virtual ECPropertiesField const* _AsPropertiesField() const {return nullptr;}
        virtual NestedContentField* _AsNestedContentField() {return nullptr;}
        virtual NestedContentField const* _AsNestedContentField() const {return nullptr;}
        virtual Field* _Clone() const = 0;
        virtual TypeDescriptionPtr _CreateTypeDescription() const = 0;
        virtual bool _IsReadOnly() const = 0;
        virtual bool _IsVisible() const {return true;}
        virtual bool _Equals(Field const& other) const
            {
            Category const* thisCategory = GetCategory().get();
            Category const* otherCategory = other.GetCategory().get();
            return (thisCategory == nullptr && otherCategory == nullptr || thisCategory != nullptr && otherCategory != nullptr && thisCategory->Equals(*otherCategory, false, false))
                && m_uniqueName.Equals(other.m_uniqueName)
                && GetLabel().Equals(other.GetLabel());
            }
        virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const = 0;
        virtual int _GetPriority() const = 0;
        virtual void _OnFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo) {}
        virtual bool _OnFieldRemoved(Field const&) {return false;}
        virtual Utf8String _CreateName() const = 0;

    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label The label of this field.
        //! @param[in] renderer The custom renderer for this field.
        //! @param[in] editor The custom editor for this field.
        Field(std::shared_ptr<Category const> category, Utf8String label, ContentFieldRenderer const* renderer = nullptr, ContentFieldEditor const* editor = nullptr)
            : m_category(category), m_label(label), m_renderer(nullptr), m_editor(nullptr), m_parent(nullptr)
            {
            if (renderer)
                m_renderer = new ContentFieldRenderer(*renderer);

            if (editor)
                m_editor = new ContentFieldEditor(*editor);
            }

        //! Constructor.
        Field(std::shared_ptr<Category const> category, Utf8String uniqueName, Utf8String label, ContentFieldRenderer const* renderer = nullptr, ContentFieldEditor const* editor = nullptr)
            : Field(category, label, renderer, editor)
            {
            m_uniqueName = uniqueName;
            }

        //! Copy constructor.
        Field(Field const& other)
            : m_category(other.m_category), m_uniqueName(other.m_uniqueName), m_label(other.m_label), m_renderer(nullptr), m_editor(nullptr), m_parent(other.m_parent)
            {
            if (nullptr != other.m_renderer)
                m_renderer = new ContentFieldRenderer(*other.m_renderer);

            if (nullptr != other.m_editor)
                m_editor = new ContentFieldEditor(*other.m_editor);
            }

        //! Move constructor.
        Field(Field&& other)
            : m_category(std::move(other.m_category)), m_uniqueName(std::move(other.m_uniqueName)), m_label(std::move(other.m_label)),
            m_renderer(other.m_renderer), m_editor(other.m_editor), m_parent(other.m_parent)
            {
            other.m_renderer = nullptr;
            other.m_editor = nullptr;
            }

        //! Virtual destructor.
        virtual ~Field()
            {
            DELETE_AND_CLEAR(m_renderer);
            DELETE_AND_CLEAR(m_editor);
            }

        //! Clone this field.
        Field* Clone() const {return _Clone();}

        //! Serialize this field to JSON.
        rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(ctx, allocator);}
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

        //! Is this field equal to the supplied one.
        bool operator==(Field const& other) const {return _Equals(other);}
        //! Is this field not equal to the supplied one.
        bool operator!=(Field const& other) const {return !_Equals(other);}

        //! Is this a display label field.
        bool IsDisplayLabelField() const {return nullptr != _AsDisplayLabelField();}
        //! Get this field as @ref DisplayLabelField
        DisplayLabelField* AsDisplayLabelField() {return _AsDisplayLabelField();}
        //! Get this field as @ref DisplayLabelField
        DisplayLabelField const* AsDisplayLabelField() const {return _AsDisplayLabelField();}

        //! Is this an ECProperty field.
        bool IsPropertiesField() const {return nullptr != _AsPropertiesField();}
        //! Get this field as an @ref ECPropertiesField
        ECPropertiesField* AsPropertiesField() {return _AsPropertiesField();}
        //! Get this field as an @ref ECPropertiesField
        ECPropertiesField const* AsPropertiesField() const {return _AsPropertiesField();}

        //! Is this a calculated field.
        bool IsCalculatedPropertyField() const {return nullptr != _AsCalculatedPropertyField();}
        //! Get this field as a @ref CalculatedPropertyField
        CalculatedPropertyField* AsCalculatedPropertyField() {return _AsCalculatedPropertyField();}
        //! Get this field as a @ref CalculatedPropertyField
        CalculatedPropertyField const* AsCalculatedPropertyField() const {return _AsCalculatedPropertyField();}

        //! Is this a nested content field.
        bool IsNestedContentField() const {return nullptr != _AsNestedContentField();}
        //! Get this field as a @ref NestedContentField
        NestedContentField* AsNestedContentField() {return _AsNestedContentField();}
        //! Get this field as a @ref NestedContentField
        NestedContentField const* AsNestedContentField() const {return _AsNestedContentField();}

        //! Get the category of this field.
        std::shared_ptr<Category const> GetCategory() const {return m_category;}
        //! Set the category for this field.
        void SetCategory(std::shared_ptr<Category const> category) {m_category = category;}

        //! Create a default value for this field's name.
        Utf8String CreateName() const {return _CreateName();}
        //! Set unique name for this field.
        void SetUniqueName(Utf8String name) {m_uniqueName = name;}
        //! Getet unique name of this field.
        Utf8StringCR GetUniqueName() const {return m_uniqueName;}

        //! Get the label of this field.
        Utf8StringCR GetLabel() const {return m_label;}
        //! Set the label for this field.
        void SetLabel(Utf8String label) {m_label = label;}

        //! Get the renderer of this field.
        ContentFieldRenderer const* GetRenderer() const {return m_renderer;}
        //! Set the renderer for this field.
        //! @note The field takes ownership of the renderer object.
        void SetRenderer(ContentFieldRenderer const* renderer) {m_renderer = renderer;}

        //! Get the editor of this field.
        ContentFieldEditor const* GetEditor() const {return m_editor;}
        //! Set the editor for this field.
        //! @note The field takes ownership of the editor object.
        void SetEditor(ContentFieldEditor const* editor) {m_editor = editor;}

        //! Get field's priority.
        int GetPriority() const {return _GetPriority();}

        //! Is field read only
        bool IsReadOnly() const {return _IsReadOnly();}

        //! Get parent field
        NestedContentField const* GetParent() const {return m_parent;}
        void SetParent(NestedContentField const* parent) {m_parent = parent;}

        //! Get field's type information.
        ECPRESENTATION_EXPORT TypeDescription const& GetTypeDescription() const;
    };

    //===================================================================================
    //! Describes a single content display label field. This field is usually included in
    //! content for the @ref ContentDisplayType::Grid display type.
    // @bsiclass
    //===================================================================================
    struct DisplayLabelField : Field
    {
        ECPRESENTATION_EXPORT static const Utf8CP NAME;

    private:
        int m_priority;
        bmap<ECClassCP, bvector<InstanceLabelOverride const*>> m_labelOverrideSpecs;

    private:
        ECPRESENTATION_EXPORT static bmap<ECClassCP, bvector<InstanceLabelOverride const*>> CloneLabelOverrideValueSpecs(bmap<ECClassCP, bvector<InstanceLabelOverride const*>> const&);

    protected:
        DisplayLabelField* _AsDisplayLabelField() override {return this;}
        DisplayLabelField const* _AsDisplayLabelField() const override {return this;}
        Field* _Clone() const override {return new DisplayLabelField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        int _GetPriority() const override {return m_priority;}
        bool _IsReadOnly() const override {return true;}
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        Utf8String _CreateName() const override {return NAME;}

    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label The label of this field.
        //! @param[in] priority Field priority.
        DisplayLabelField(std::shared_ptr<Category const> category, Utf8String label, int priority = Property::DEFAULT_PRIORITY)
            : Field(category, NAME, label), m_priority(priority)
            {}
        DisplayLabelField(DisplayLabelField const& other) : Field(other), m_priority(other.m_priority), m_labelOverrideSpecs(CloneLabelOverrideValueSpecs(other.m_labelOverrideSpecs)) {}
        DisplayLabelField(DisplayLabelField&& other) : Field(std::move(other)), m_priority(other.m_priority) { m_labelOverrideSpecs.swap(other.m_labelOverrideSpecs);}
        ECPRESENTATION_EXPORT ~DisplayLabelField();

        //! Set the priority for this field.
        void SetPriority(int priority) {m_priority = priority;}

        //! Get a map of label override specifications
        bmap<ECClassCP, bvector<InstanceLabelOverride const*>> const& GetLabelOverrideSpecs() const {return m_labelOverrideSpecs;}

        //! Set label override specifications' map
        void SetLabelOverrideSpecs(bmap<ECClassCP, bvector<InstanceLabelOverride const*>> const& specs) {m_labelOverrideSpecs = CloneLabelOverrideValueSpecs(specs);}
    };

    //===================================================================================
    //! Describes a single content calculated property field. Calculated fields use
    //! ECExpressions to determine the value.
    // @bsiclass
    //===================================================================================
    struct CalculatedPropertyField : Field
    {
    private:
        int m_priority;
        Utf8String m_valueExpression;
        ECClassCP m_class;
        Utf8String m_requestedName;

    protected:
        CalculatedPropertyField* _AsCalculatedPropertyField() override {return this;}
        CalculatedPropertyField const* _AsCalculatedPropertyField() const override {return this;}
        Field* _Clone() const override {return new CalculatedPropertyField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        int _GetPriority() const override {return m_priority;}
        bool _IsReadOnly() const override {return true;}
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        Utf8String _CreateName() const override {return m_requestedName;}

    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label Field label.
        //! @param[in] name Unique field name.
        //! @param[in] valueExpression Value ECExpression.
        //! @param[in] ecClass Entity class this field is intended for.
        //! @param[in] priority Field priority.
        CalculatedPropertyField(std::shared_ptr<Category const> category, Utf8String label, Utf8String name, Utf8String valueExpression, ECClassCP ecClass, int priority = Property::DEFAULT_PRIORITY)
            : Field(category, label), m_requestedName(name), m_valueExpression(valueExpression), m_class(ecClass), m_priority(priority)
            {}

        Utf8StringCR GetRequestedName() const {return m_requestedName;}

        //! Get the ECExpression used to calculate field's value.
        Utf8String const& GetValueExpression() const {return m_valueExpression;}

        //! Get the class this field is intended for.
        ECClassCP GetClass() const {return m_class;}

        //! Set the priority for this field.
        void SetPriority(int priority) {m_priority = priority;}
    };

    //===================================================================================
    //! Describes a single content field which is based on ECProperties. The field should
    //! be based on one or more properties of the similar type.
    // @bsiclass
    //===================================================================================
    struct ECPropertiesField : Field
    {
    private:
        bvector<Property> m_properties;
        mutable std::unordered_map<ECClassCP, bvector<Property const*>> m_matchingPropertiesCache;
        mutable BeMutex m_matchingPropertiesCacheMutex;
        Nullable<bool> m_isReadOnly;
        Nullable<int> m_priority;

    protected:
        ECPropertiesField* _AsPropertiesField() override {return this;}
        ECPropertiesField const* _AsPropertiesField() const override {return this;}
        Field* _Clone() const override {return new ECPropertiesField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        ECPRESENTATION_EXPORT bool _IsReadOnly() const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT int _GetPriority() const override;
        ECPRESENTATION_EXPORT Utf8String _CreateName() const override;
    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label The label of this field.
        //! @param[in] renderer The custom renderer for this field.
        //! @param[in] editor The custom editor for this field.
        ECPropertiesField(std::shared_ptr<Category const> category, Utf8String label, ContentFieldRenderer const* renderer = nullptr, ContentFieldEditor const* editor = nullptr,
            Nullable<bool> isReadOnly = nullptr, Nullable<int> priority = nullptr)
            : Field(category, label, renderer, editor), m_isReadOnly(isReadOnly), m_priority(priority)
            {}

        //! Constructor.
        ECPropertiesField(std::shared_ptr<Category const> category, Utf8String uniqueName, Utf8String label, ContentFieldRenderer const* renderer = nullptr, ContentFieldEditor const* editor = nullptr,
            Nullable<bool> isReadOnly = nullptr, Nullable<int> priority = nullptr)
            : Field(category, uniqueName, label, renderer, editor), m_isReadOnly(isReadOnly), m_priority(priority)
            {}

        //! Constructor. Creates a field with a single @ref Property.
        //! @param[in] category The category of this field.
        //! @param[in] prop Property that this field is based on.
        ECPRESENTATION_EXPORT ECPropertiesField(std::shared_ptr<Category const> category, Property const& prop);

        //! Constructor.
        ECPropertiesField(std::shared_ptr<Category const> category, Utf8String uniqueName, Property const& prop)
            : ECPropertiesField(category, prop)
            {
            SetUniqueName(uniqueName);
            }

        //! Copy constructor.
        ECPropertiesField(ECPropertiesField const& other) : Field(other), m_properties(other.m_properties), m_isReadOnly(other.m_isReadOnly), m_priority(other.m_priority) {}

        //! Move constructor.
        ECPropertiesField(ECPropertiesField&& other) : Field(std::move(other)), m_properties(std::move(other.m_properties)), m_isReadOnly(std::move(other.m_isReadOnly)), m_priority(std::move(other.m_priority)) {}

        //! Is this field equal to the supplied one.
        bool operator==(ECPropertiesField const& other) const {return Field::operator==(other) && m_properties == other.m_properties && m_isReadOnly == other.m_isReadOnly && m_priority == other.m_priority;}

        //! Does this field contain composite properties (structs or arrays)
        ECPRESENTATION_EXPORT bool IsCompositePropertiesField() const;

        //! Add property to this field.
        void AddProperty(Property prop) {m_properties.push_back(prop);}
        //! Get the properties that this field is based on.
        bvector<Property> const& GetProperties() const {return m_properties;}
        bvector<Property>& GetProperties() {return m_properties;}
        //! Find properties that match the supplied class. If nullptr is supplied,
        //! all properties are returned.
        bvector<Property const*> const& FindMatchingProperties(ECClassCP) const;

        //! Does this field contain the given property
        ECPRESENTATION_EXPORT bool ContainsProperty(ECPropertyCR) const;
    };

    struct CompositeContentField;
    struct RelatedContentField;
    //===================================================================================
    //! Describes a single content field which by itself describes content. Creating content
    //! for this field requires executing a separate query. Examples of such content could be getting
    //! content for related instances or composite structures like arrays or structs.
    // @bsiclass
    //===================================================================================
    struct NestedContentField : Field
    {
    private:
        bvector<Field*> m_fields;
        int m_priority;
        bool m_autoExpand;

    protected:
        NestedContentField* _AsNestedContentField() override {return this;}
        NestedContentField const* _AsNestedContentField() const override {return this;}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        bool _IsReadOnly() const override {return true;}
        ECPRESENTATION_EXPORT virtual bool _Equals(Field const& other) const override;
        int _GetPriority() const override {return m_priority;}
        ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
        ECPRESENTATION_EXPORT virtual Utf8String _CreateName() const override;
        virtual ECClassCR _GetContentClass() const = 0;
        virtual Utf8StringCR _GetContentClassAlias() const = 0;
        virtual CompositeContentField* _AsCompositeContentField() {return nullptr;}
        virtual RelatedContentField* _AsRelatedContentField() {return nullptr;}

        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label The label of this field.
        //! @param[in] fields A list of fields which this field consists from.
        //! @param[in] autoExpand Flag specifying if this field should be expanded.
        //! @param[in] priority Priority of the field
        NestedContentField(std::shared_ptr<Category const> category, Utf8String label, bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : NestedContentField(category, "", label, fields, autoExpand, priority)
            {}

        //! Constructor.
        NestedContentField(std::shared_ptr<Category const> category, Utf8String uniqueName, Utf8String label, bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : Field(category, uniqueName, label), m_fields(fields), m_priority(priority), m_autoExpand(autoExpand)
            {
            for (auto field : fields)
                field->SetParent(this);
            }

        //! Copy constructor.
        NestedContentField(NestedContentField const& other)
            : Field(other), m_priority(other.m_priority), m_autoExpand(other.m_autoExpand)
            {
            for (Field const* field : other.m_fields)
                m_fields.push_back(field->Clone());
            }

        //! Move constructor.
        NestedContentField(NestedContentField&& other)
            : Field(std::move(other)), m_fields(std::move(other.m_fields)), m_priority(other.m_priority), m_autoExpand(other.m_autoExpand)
            {}

    public:
        //! Destructor
        ~NestedContentField() {ClearFields();}

        CompositeContentField* AsCompositeContentField() {return _AsCompositeContentField();}
        CompositeContentField const* AsCompositeContentField() const {return const_cast<NestedContentField*>(this)->AsCompositeContentField();}
        RelatedContentField* AsRelatedContentField() {return _AsRelatedContentField();}
        RelatedContentField const* AsRelatedContentField() const {return const_cast<NestedContentField*>(this)->AsRelatedContentField();}

        //! Get the content class whose content is returned by this field.
        ECClassCR GetContentClass() const {return _GetContentClass();}

        //! Get alias of the content class
        Utf8StringCR GetContentClassAlias() const {return _GetContentClassAlias();}

        //! A list of fields which this field consists from.
        bvector<Field*> const& GetFields() const {return m_fields;}
        bvector<Field*>& GetFields() {return m_fields;}

        //! Remove a single field
        void RemoveField(bvector<Field*>::const_iterator const& iter)
            {
            if (m_fields.end() != iter)
                {
                delete* iter;
                m_fields.erase(iter);
                }
            }
        void RemoveField(std::function<bool(Field const&)> const& pred)
            {
            auto iter = std::find_if(m_fields.begin(), m_fields.end(), [&pred](Field const* f){return pred(*f);});
            RemoveField(iter);
            }
        void RemoveField(Field const& field) {RemoveField([&field](Field const& f){return &f == &field;});}

        //! Replace matching field with another one
        void ReplaceField(std::function<bool(Field const&)> const& pred, Field& replacement)
            {
            auto iter = std::find_if(m_fields.begin(), m_fields.end(), [&pred](Field const* f){return pred(*f);});
            if (m_fields.end() != iter)
                {
                delete* iter;
                *iter = &replacement;
                }
            else
                {
                m_fields.push_back(&replacement);
                }
            }
        void ReplaceField(Field const& remove, Field& replacement) {ReplaceField([&remove](Field const& f){return &f == &remove;}, replacement);}

        //! Clear all fields
        void ClearFields()
            {
            for (Field const* field : m_fields)
                delete field;
            m_fields.clear();
            }

        //! Should this field be automatically expanded.
        bool ShouldAutoExpand() const {return m_autoExpand;}
        };

    //===================================================================================
    //! Describes a single content field which contains composite (struct, array) content.
    // @bsiclass
    //===================================================================================
    struct CompositeContentField : NestedContentField
    {
    private:
        ECClassCR m_contentClass;
        Utf8String m_contentClassAlias;
    protected:
        ECClassCR _GetContentClass() const override {return m_contentClass;}
        Utf8StringCR _GetContentClassAlias() const override {return m_contentClassAlias;}
        virtual CompositeContentField* _AsCompositeContentField() override {return this;}
        Field* _Clone() const override {return new CompositeContentField(*this);}
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;
    public:
        CompositeContentField(std::shared_ptr<Category const> category, Utf8String label, ECClassCR contentClass, Utf8String contentClassAlias,
            bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : NestedContentField(category, label, fields, autoExpand, priority), m_contentClass(contentClass),
            m_contentClassAlias(contentClassAlias)
            {}
        CompositeContentField(std::shared_ptr<Category const> category, Utf8String uniqueName, Utf8String label, ECClassCR contentClass, Utf8String contentClassAlias,
            bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : NestedContentField(category, uniqueName, label, fields, autoExpand, priority), m_contentClass(contentClass),
            m_contentClassAlias(contentClassAlias)
            {}
        CompositeContentField(CompositeContentField const& other)
            : NestedContentField(other), m_contentClass(other.m_contentClass), m_contentClassAlias(other.m_contentClassAlias)
            {}
        CompositeContentField(CompositeContentField&& other)
            : NestedContentField(std::move(other)), m_contentClass(other.m_contentClass), m_contentClassAlias(std::move(other.m_contentClassAlias))
            {}
    };

    //===================================================================================
    //! Describes a single content field which contains related instances' content.
    // @bsiclass
    //===================================================================================
    struct RelatedContentField : NestedContentField
    {
    private:
        RelatedClassPath m_pathFromSelectClassToContentClass;
        std::unordered_set<ECClassCP> m_actualSourceClasses;
        RelationshipMeaning m_relationshipMeaning;
        bool m_isRelationshipField;

    private:
        Utf8String CreateRelationshipName() const;

    protected:
        ECClassCR _GetContentClass() const override {return m_pathFromSelectClassToContentClass.back().GetTargetClass().GetClass();}
        Utf8StringCR _GetContentClassAlias() const override {return m_pathFromSelectClassToContentClass.back().GetTargetClass().GetAlias();}
        virtual RelatedContentField* _AsRelatedContentField() override {return this;}
        Field* _Clone() const override {return new RelatedContentField(*this);}
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT Utf8String _CreateName() const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(ECPresentationSerializerContextR, rapidjson::Document::AllocatorType* allocator) const override;

    public:
        RelatedContentField(std::shared_ptr<Category const> category, Utf8String label, RelatedClassPath pathFromSelectClassToContentClass,
            bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY, bool isRelationshipField = false)
            : RelatedContentField(category, "", label, pathFromSelectClassToContentClass, fields, autoExpand, priority, isRelationshipField)
            {}
        RelatedContentField(std::shared_ptr<Category const> category, Utf8String uniqueName, Utf8String label, RelatedClassPath pathFromSelectClassToContentClass,
            bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY, bool isRelationshipField = false)
            : NestedContentField(category, uniqueName, label, fields, autoExpand, priority), m_pathFromSelectClassToContentClass(pathFromSelectClassToContentClass),
            m_relationshipMeaning(RelationshipMeaning::RelatedInstance), m_isRelationshipField(isRelationshipField)
            {}
        RelatedContentField(RelatedContentField const& other)
            : NestedContentField(other), m_pathFromSelectClassToContentClass(other.m_pathFromSelectClassToContentClass), m_relationshipMeaning(other.m_relationshipMeaning),
            m_isRelationshipField(other.m_isRelationshipField)
            {}
        RelatedContentField(RelatedContentField&& other)
            : NestedContentField(std::move(other)), m_pathFromSelectClassToContentClass(std::move(other.m_pathFromSelectClassToContentClass)), m_relationshipMeaning(other.m_relationshipMeaning),
            m_isRelationshipField(other.m_isRelationshipField)
            {}

        //! Path from the select class to content class.
        RelatedClassPath& GetPathFromSelectToContentClass() {return m_pathFromSelectClassToContentClass;}
        RelatedClassPath const& GetPathFromSelectToContentClass() const {return m_pathFromSelectClassToContentClass;}
        void SetPathFromSelectToContentClass(RelatedClassPath path) {m_pathFromSelectClassToContentClass = path;}

        //! Actual source subclasses of source in 'path from select to content class'
        std::unordered_set<ECClassCP> const& GetActualSourceClasses() const {return m_actualSourceClasses;}
        std::unordered_set<ECClassCP>& GetActualSourceClasses() {return m_actualSourceClasses;}
        void SetActualSourceClasses(std::unordered_set<ECClassCP> classes) {m_actualSourceClasses = classes;}

        void SetRelationshipMeaning(RelationshipMeaning relationshipMeaning) {m_relationshipMeaning = relationshipMeaning;}
        RelationshipMeaning GetRelationshipMeaning() const {return m_relationshipMeaning;}

        ECPRESENTATION_EXPORT bool IsXToMany() const;

        Utf8CP GetSelectClassAlias() const {return "related";}

        bool IsRelationshipField() const {return m_isRelationshipField;}
        ECClassCR GetRelationshipClass() const {return m_pathFromSelectClassToContentClass.back().GetRelationship().GetClass();}
        Utf8StringCR GetRelationshipClassAlias() const {return m_pathFromSelectClassToContentClass.back().GetRelationship().GetAlias();}
    };

    //===================================================================================
    // @bsiclass
    //===================================================================================
    struct ECPropertiesFieldKey
    {
    private:
        ECPropertyCP m_property;
        ECClassCP m_propertyClass;
        ValueKind m_valueKind;
        Utf8String m_fieldLabel;
        Utf8String m_type;
        ContentFieldRenderer const* m_fieldRenderer;
        ContentFieldEditor const* m_fieldEditor;
        KindOfQuantityCP m_koq;
        Utf8String m_categoryName;
    public:
        ECPropertiesFieldKey()
            : m_property(nullptr), m_propertyClass(nullptr), m_fieldRenderer(nullptr), m_fieldEditor(nullptr), m_valueKind(VALUEKIND_Uninitialized), m_koq(nullptr)
            {}
        ECPropertiesFieldKey(ECPropertyCR property, ECClassCR propertyClass, Utf8String fieldLabel, Utf8String categoryName, ContentFieldRenderer const* renderer, ContentFieldEditor const* editor)
            : m_property(&property), m_propertyClass(&propertyClass), m_fieldLabel(fieldLabel), m_categoryName(categoryName), m_fieldRenderer(renderer), m_fieldEditor(editor)
            {
            m_valueKind = m_property->GetIsPrimitive() ? VALUEKIND_Primitive
                : m_property->GetIsNavigation() ? VALUEKIND_Navigation
                : m_property->GetIsArray() ? VALUEKIND_Array
                : m_property->GetIsStruct() ? VALUEKIND_Struct
                : VALUEKIND_Uninitialized;
            m_type = m_property->GetTypeName();
            m_koq = m_property->GetKindOfQuantity();
            }
        ECPropertiesFieldKey(Property const& prop, Utf8String fieldLabel, Utf8String categoryName, ContentFieldRenderer const* renderer, ContentFieldEditor const* editor)
            : ECPropertiesFieldKey(prop.GetProperty(), prop.GetPropertyClass(), fieldLabel, categoryName, renderer, editor)
            {}
        bool operator<(ECPropertiesFieldKey const& rhs) const;
        ValueKind GetValueKind() const {return m_valueKind;}
        Utf8CP GetName() const {return m_property->GetName().c_str();}
        Utf8CP GetLabel() const {return m_fieldLabel.c_str();}
        Utf8CP GetType() const {return m_type.c_str();}
        KindOfQuantityCP GetKoq() const {return m_koq;}
        ECClassCP GetClass() const {return m_propertyClass;}
        Utf8StringCR GetCategoryName() const {return m_categoryName;}
        ContentFieldRenderer const* GetRenderer() const {return m_fieldRenderer;}
        ContentFieldEditor const* GetEditor() const {return m_fieldEditor;}
    };

private:
    PresentationRuleSetCPtr m_ruleset;
    RulesetVariables m_rulesetVariables;
    Utf8String m_preferredDisplayType;
    bvector<SelectClassInfo> m_classes;
    bmap<Utf8String, bvector<size_t>> m_specificationClasses; // content specification hash => [indexes in `m_classes`]
    bvector<std::shared_ptr<Category const>> m_categories;
    bvector<Field*> m_fields;
    bmap<ECPropertiesFieldKey, ECPropertiesField*> m_fieldsMap;
    mutable Nullable<size_t> m_totalFieldsCount;
    int m_sortingFieldIndex;
    SortDirection m_sortDirection;
    int m_contentFlags;
    Utf8String m_fieldsFilterExpression;
    std::shared_ptr<InstanceFilterDefinition> m_instanceFilter;
    Utf8String m_connectionId;
    INavNodeKeysContainerCPtr m_inputKeys;
    SelectionInfoCPtr m_selectionInfo;
    UnitSystem m_unitSystem;

private:
    ECPRESENTATION_EXPORT ContentDescriptor(IConnectionCR, PresentationRuleSetCR, RulesetVariables, INavNodeKeysContainerCR);
    ECPRESENTATION_EXPORT ContentDescriptor(ContentDescriptorCR other);
    ECPRESENTATION_EXPORT int GetFieldIndex(Utf8CP name) const;
    void OnFlagAdded(ContentFlags flag);
    void OnFlagRemoved(ContentFlags flag);
    void OnECPropertiesFieldRemoved(ECPropertiesField const& field);
    void OnECPropertiesFieldAdded(ECPropertiesField& field);
    void UpdateSelectClasses();
    static void IterateFields(bvector<Field*> const& fields, std::function<bool(Field&)> const& handler)
        {
        for (auto field : fields)
            {
            if (!handler(*field))
                return;
            if (field->IsNestedContentField())
                IterateFields(field->AsNestedContentField()->GetFields(), handler);
            }
        }

protected:
    ECPRESENTATION_EXPORT ~ContentDescriptor();

public:
    //! Creates a content descriptor.
    static ContentDescriptorPtr Create(IConnectionCR connection, PresentationRuleSetCR ruleset, RulesetVariables rulesetVariables, INavNodeKeysContainerCR inputKeys)
        {
        return new ContentDescriptor(connection, ruleset, rulesetVariables, inputKeys);
        }

    //! Copies the supplied content descriptor.
    //! @param[in] other The descriptor to copy.
    static ContentDescriptorPtr Create(ContentDescriptorCR other) {return new ContentDescriptor(other);}

    //! Serializes this descriptor to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

    //! Is this desciptor equal to the supplied one.
    ECPRESENTATION_EXPORT bool Equals(ContentDescriptorCR other) const;

    //! Merge this descriptor with the supplied one.
    ECPRESENTATION_EXPORT void MergeWith(ContentDescriptorCR other);

    //! Get the preferred display type which this descriptor is created for.
    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    void SetPreferredDisplayType(Utf8String value) {m_preferredDisplayType = value;}

    //! Get node keys which this descriptor is created for.
    INavNodeKeysContainerCR GetInputNodeKeys() const {return *m_inputKeys;}
    //! Get connection ID which this descriptor is created for.
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    //! Get selection info which this descriptor is created with. (returns nullptr if no selection info was provided)
    SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo.get();}
    //! Set selection info which this descriptor is created with.
    void SetSelectionInfo(SelectionInfo const& selectionInfo) {m_selectionInfo = &selectionInfo;}

    PresentationRuleSetCR GetRuleset() const {return *m_ruleset;}
    RulesetVariables const& GetRulesetVariables() const {return m_rulesetVariables;}

    UnitSystem GetUnitSystem() const {return m_unitSystem;}
    void SetUnitSystem(UnitSystem value) {m_unitSystem = value;}

    //! Add a select class to this descriptor and associate it with given specification hash
    ECPRESENTATION_EXPORT void AddSelectClass(SelectClassInfo selectClass, Utf8StringCR contentSpecificationHash);
    //! Get information about ECClasses built using specification with given hash
    ECPRESENTATION_EXPORT bvector<SelectClassInfo const*> GetSelectClasses(Utf8StringCR contentSpecificationHash) const;
    //! Get information about ECClasses which the descriptor consists from.
    bvector<SelectClassInfo> const& GetSelectClasses() const {return m_classes;}

    bvector<std::shared_ptr<Category const>> const& GetCategories() const {return m_categories;}
    bvector<std::shared_ptr<Category const>>& GetCategories() {return m_categories;}

    //! Get the fields in this descriptor (excluding system ones).
    ECPRESENTATION_EXPORT bvector<Field*> GetVisibleFields() const;
    //! Get the fields in this descriptor (including system ones).
    bvector<Field*> const& GetAllFields() const {return m_fields;}
    bvector<Field*>& GetAllFields() {return m_fields;}
    ECPRESENTATION_EXPORT size_t GetTotalFieldsCount() const;

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    //! Returns a field if descriptor has DistinctValues flag set, has a single field and that field qualifies to be used with DistinctValues flag. Returns nullptr otherwise.
    Field const* GetDistinctField() const;
#endif

    void IterateFields(std::function<bool(Field&)> const& handler) {IterateFields(GetAllFields(), handler);}

    ECPRESENTATION_EXPORT Field const* FindField(IContentFieldMatcherCR) const;

    //! Add field to this descriptor. The descriptor takes ownership of the field.
    ECPRESENTATION_EXPORT Field& AddRootField(Field& field);
    //! Find ECProperties field in this descriptor by property.
    ECPropertiesField* FindECPropertiesField(Property const& prop, Utf8StringCR fieldLabel, Category const* category, ContentFieldRenderer const* renderer, ContentFieldEditor const* editor) {return FindECPropertiesField(prop.GetProperty(), prop.GetPropertyClass(), fieldLabel, category, renderer, editor);}
    //! Find ECProperties field in this descriptor by property.
    ECPropertiesField* FindECPropertiesField(ECPropertyCR prop, ECClassCR propClass, Utf8StringCR fieldLabel, Category const* category, ContentFieldRenderer const* renderer, ContentFieldEditor const* editor);

    //! Remove a field from this descriptor.
    ECPRESENTATION_EXPORT bool RemoveRootField(bvector<Field*>::const_iterator const&);
    ECPRESENTATION_EXPORT bool RemoveRootField(std::function<bool(Field const&)> const&);
    ECPRESENTATION_EXPORT bool RemoveRootField(Field const&);

    ECPRESENTATION_EXPORT bool ExclusivelyIncludeFields(bvector<Field const*> const&);
    ECPRESENTATION_EXPORT bool ExcludeFields(bvector<Field const*> const&);
    ECPRESENTATION_EXPORT void ClearFields();

    //! Get display label field.
    DisplayLabelField* GetDisplayLabelField() const {return !m_fields.empty() && m_fields[0]->IsDisplayLabelField() ? m_fields[0]->AsDisplayLabelField() : nullptr;}

    //! Get the sorting field used to sort content.
    Field const* GetSortingField() const {return (m_sortingFieldIndex < 0 || m_sortingFieldIndex >= (int)m_fields.size()) ? nullptr : m_fields[m_sortingFieldIndex];}
    //! Get the sorting field index used to sort content.
    int GetSortingFieldIndex() const {return m_sortingFieldIndex;}
    //! Set the sorting field by index.
    void SetSortingField(int index) {m_sortingFieldIndex = index;}
    //! Set the sorting field by name.
    void SetSortingField(Utf8CP name) {m_sortingFieldIndex = GetFieldIndex(name);}

    //! Set sorting direction.
    void SetSortDirection(SortDirection direction) {m_sortDirection = direction;}
    //! Get sorting direction.
    SortDirection GetSortDirection() const {return m_sortDirection;}

    //! Set fields filtering ECExpression.
    void SetFieldsFilterExpression(Utf8String expression) {m_fieldsFilterExpression = expression;}
    //! Get fields filtering ECExpression.
    Utf8StringCR GetFieldsFilterExpression() const {return m_fieldsFilterExpression;}

    //! Set instance filter definition.
    void SetInstanceFilter(std::shared_ptr<InstanceFilterDefinition> filter) {m_instanceFilter = filter;}
    //! Get instance filter definition.
    std::shared_ptr<InstanceFilterDefinition> GetInstanceFilter() const {return m_instanceFilter;}

    //! Get the content flags.
    //! @see ContentFlags
    int GetContentFlags() const {return m_contentFlags;}
    //! Set the content flags.
    //! @see ContentFlags
    ECPRESENTATION_EXPORT void SetContentFlags(int flags);
    //! Add a content flag.
    ECPRESENTATION_EXPORT void AddContentFlag(ContentFlags flag);
    //! Remove a content flag.
    ECPRESENTATION_EXPORT void RemoveContentFlag(ContentFlags flag);
    //! Does this descriptor have the supplied content flag.
    ECPRESENTATION_EXPORT bool HasContentFlag(ContentFlags flag) const;
    //! Should content include images.
    bool ShowImages() const {return HasContentFlag(ContentFlags::ShowImages) && !HasContentFlag(ContentFlags::KeysOnly);}
    //! Should content include display labels.
    bool ShowLabels() const {return HasContentFlag(ContentFlags::ShowLabels) && !HasContentFlag(ContentFlags::KeysOnly);}
    //! Should the content be merged into a single record.
    bool MergeResults() const {return HasContentFlag(ContentFlags::MergeResults);}
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    //! Should only distinct values be returned
    bool OnlyDistinctValues() const {return HasContentFlag(ContentFlags::DistinctValues);}
#endif
};

//=======================================================================================
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct IContentFieldMatcher
{
protected:
    virtual bool _Matches(ContentDescriptor::Field const& field) const = 0;
    virtual std::unique_ptr<IContentFieldMatcher> _Clone() const = 0;
public:
    virtual ~IContentFieldMatcher() {}
    bool Matches(ContentDescriptor::Field const& field) const {return _Matches(field);}
    std::unique_ptr<IContentFieldMatcher> Clone() const {return _Clone();}
};

//=======================================================================================
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct NeverMatchingContentFieldMatcher : IContentFieldMatcher
{
protected:
    bool _Matches(ContentDescriptor::Field const& field) const override {return false;}
    std::unique_ptr<IContentFieldMatcher> _Clone() const override {return std::make_unique<NeverMatchingContentFieldMatcher>();}
public:
    NeverMatchingContentFieldMatcher() {}
};

//=======================================================================================
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct GenericContentFieldMatcher : IContentFieldMatcher
{
private:
    std::function<bool(ContentDescriptor::Field const&)> m_matcher;
protected:
    bool _Matches(ContentDescriptor::Field const& field) const override {return m_matcher(field);}
    std::unique_ptr<IContentFieldMatcher> _Clone() const override {return std::make_unique<GenericContentFieldMatcher>(m_matcher);}
public:
    GenericContentFieldMatcher(std::function<bool(ContentDescriptor::Field const&)> matcher) : m_matcher(matcher) {}
};

//=======================================================================================
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct CombinedContentFieldMatcher : IContentFieldMatcher
{
private:
    bvector<std::unique_ptr<IContentFieldMatcher>> m_matchers;
protected:
    bool _Matches(ContentDescriptor::Field const& field) const override {return std::any_of(m_matchers.begin(), m_matchers.end(), [&field](auto const& matcher){return matcher->Matches(field);});}
    std::unique_ptr<IContentFieldMatcher> _Clone() const override
        {
        bvector<std::unique_ptr<IContentFieldMatcher>> matchers;
        std::transform(m_matchers.begin(), m_matchers.end(), std::back_inserter(matchers), [](auto const& matcher){return matcher->Clone();});
        return std::make_unique<CombinedContentFieldMatcher>(std::move(matchers));
        }
public:
    CombinedContentFieldMatcher(bvector<std::unique_ptr<IContentFieldMatcher>>&& matchers) : m_matchers(std::move(matchers)) {}
};

//=======================================================================================
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct NamedContentFieldMatcher : IContentFieldMatcher
{
private:
    Utf8String m_fieldName;
protected:
    bool _Matches(ContentDescriptor::Field const& field) const override {return field.GetUniqueName().Equals(m_fieldName);}
    std::unique_ptr<IContentFieldMatcher> _Clone() const override {return std::make_unique<NamedContentFieldMatcher>(m_fieldName);}
public:
    NamedContentFieldMatcher(Utf8String fieldName) : m_fieldName(fieldName) {}
};

//=======================================================================================
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct PropertiesContentFieldMatcher : IContentFieldMatcher
{
private:
    ECPropertyCR m_property;
    RelatedClassPath m_pathFromSelectToPropertyClass;
private:
    ECPRESENTATION_EXPORT static bool SelectToPropertyPathsMatch(RelatedClassPathCR, ContentDescriptor::Field const&);
protected:
    bool _Matches(ContentDescriptor::Field const& field) const override
        {
        return field.IsPropertiesField()
            && field.AsPropertiesField()->ContainsProperty(m_property)
            && SelectToPropertyPathsMatch(m_pathFromSelectToPropertyClass, field);
        }
    std::unique_ptr<IContentFieldMatcher> _Clone() const override {return std::make_unique<PropertiesContentFieldMatcher>(m_property, m_pathFromSelectToPropertyClass);}
public:
    PropertiesContentFieldMatcher(ECPropertyCR prop, RelatedClassPathCR pathFromSelectToPropertyClass)
        : m_property(prop), m_pathFromSelectToPropertyClass(pathFromSelectToPropertyClass)
        {}
};

//=======================================================================================
//! A struct that represents a single content record.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct ContentSetItem : RefCountedBase, RapidJsonExtendedDataHolder<>
{
    //===================================================================================
    //! Describes ContentSetItem parts which may be serialized.
    // @bsiclass
    //===================================================================================
    enum SerializationFlags
        {
        SERIALIZE_DisplayLabel  = 1 << 0,
        SERIALIZE_ImageId       = 1 << 1,
        SERIALIZE_Values        = 1 << 2,
        SERIALIZE_DisplayValues = 1 << 3,
        SERIALIZE_ClassInfo     = 1 << 4,
        SERIALIZE_PrimaryKeys   = 1 << 5,
        SERIALIZE_MergedFieldNames = 1 << 6,
        SERIALIZE_FieldPropertyInstanceKeys = 1 << 7,
        SERIALIZE_UsersExtendedData = 1 << 8,
        SERIALIZE_InputKeys     = 1 << 9,
        SERIALIZE_All           = ((1 << 10) - 1),
        };

    //===================================================================================
    //! Data structure containing a pair of ECPropertiesField and an index of a property
    //! in that field.
    // @bsiclass
    //===================================================================================
    struct FieldPropertyIdentifier
    {
    private:
        Utf8String m_fieldName;
        size_t m_propertyIndex;
    public:
        FieldPropertyIdentifier() : m_propertyIndex(-1) {}
        FieldPropertyIdentifier(ContentDescriptor::ECPropertiesField const& field, size_t propertyIndex)
            : m_fieldName(field.GetUniqueName()), m_propertyIndex(propertyIndex)
            {}
        bool operator<(FieldPropertyIdentifier const& other) const
            {
            if (m_propertyIndex != other.m_propertyIndex)
                return m_propertyIndex < other.m_propertyIndex;
            return m_fieldName.CompareTo(other.m_fieldName) < 0;
            }
        Utf8StringCR GetFieldName() const {return m_fieldName;}
        size_t GetPropertyIndex() const {return m_propertyIndex;}
    };
    typedef bmap<FieldPropertyIdentifier, bvector<ECClassInstanceKey>> FieldPropertyInstanceKeyMap;

private:
    ECClassCP m_class;
    bvector<ECClassInstanceKey> m_inputKeys;
    bvector<ECClassInstanceKey> m_keys;
    LabelDefinitionCPtr m_displayLabelDefinition;
    Utf8String m_imageId;
    bmap<Utf8String, bvector<ContentSetItemPtr>> m_nestedContent;
    rapidjson::Document m_values;
    rapidjson::Document m_displayValues;
    rapidjson::Document m_extendedData;
    FieldPropertyInstanceKeyMap m_fieldPropertyInstanceKeys;
    bvector<Utf8String> m_mergedFieldNames;

private:
    ContentSetItem(bvector<ECClassInstanceKey> inputKeys, bvector<ECClassInstanceKey> keys, LabelDefinitionCR displayLabelDefinition, Utf8String imageId,
        bmap<Utf8String, bvector<ContentSetItemPtr>> nestedContent, rapidjson::Document&& values, rapidjson::Document&& displayValues,
        bvector<Utf8String> mergedFieldNames, FieldPropertyInstanceKeyMap fieldPropertyInstanceKeys)
        : m_class(nullptr), m_keys(std::move(keys)), m_displayLabelDefinition(&displayLabelDefinition), m_imageId(imageId), m_nestedContent(nestedContent),
        m_values(std::move(values)), m_displayValues(std::move(displayValues)), m_extendedData(rapidjson::kObjectType),
        m_mergedFieldNames(mergedFieldNames), m_fieldPropertyInstanceKeys(fieldPropertyInstanceKeys), m_inputKeys(std::move(inputKeys))
        {}
public:
    bmap<Utf8String, bvector<ContentSetItemPtr>>& GetNestedContent() {return m_nestedContent;}
    bmap<Utf8String, bvector<ContentSetItemPtr>> const& GetNestedContent() const {return m_nestedContent;}
    rapidjson::Document& GetValues() {return m_values;}
    rapidjson::Document const& GetValues() const { return m_values; }
    rapidjson::Document& GetDisplayValues() {return m_displayValues;}
    rapidjson::Document const& GetDisplayValues() const { return m_displayValues; }
    rapidjson::Document& GetExtendedData() { return m_extendedData; }
    bvector<Utf8String>& GetMergedFieldNames() {return m_mergedFieldNames;}
    bvector<ECClassInstanceKey>& GetKeys() {return m_keys;}
    FieldPropertyInstanceKeyMap const& GetFieldInstanceKeys() const {return m_fieldPropertyInstanceKeys;}

public:
    //! Creates a @ref ContentSetItem.
    //! @param[in] inputKeys The keys which describe which given input key this content item is related to.
    //! @param[in] keys The keys which describe whose values this item contains.
    //! @param[in] displayLabelDefinition The definition of this content item label.
    //! @param[in] imageId The image ID for this item.
    //! @param[in] nestedContent Nested content items grouped by nesting field name.
    //! @param[in] values The values map.
    //! @param[in] displayValues The display values map.
    //! @param[in] mergedFieldNames Names of merged fields in this record.
    //! @param[in] fieldPropertyInstanceKeys ECClassInstanceKeys of related instances for each field in this record.
    static ContentSetItemPtr Create(bvector<ECClassInstanceKey> inputKeys, bvector<ECClassInstanceKey> keys, LabelDefinitionCR displayLabelDefinition, Utf8String imageId,
        bmap<Utf8String, bvector<ContentSetItemPtr>> nestedContent,  rapidjson::Document&& values, rapidjson::Document&& displayValues,
        bvector<Utf8String> mergedFieldNames, FieldPropertyInstanceKeyMap fieldPropertyInstanceKeys)
        {
        return new ContentSetItem(std::move(inputKeys), std::move(keys), displayLabelDefinition, imageId, nestedContent,
            std::move(values), std::move(displayValues), mergedFieldNames, fieldPropertyInstanceKeys);
        }

    //! Serialize this item to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, int flags = SERIALIZE_All, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(int flags = SERIALIZE_All, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    //! Get user's extended data map
    RapidJsonAccessor GetUsersExtendedData() const {return RapidJsonAccessor(m_extendedData);}
    ECPRESENTATION_EXPORT void AddUsersExtendedData(Utf8CP key, ECValueCR value);

    //! Get keys of ECInstances that we provided as input for producing this content set item.
    //!
    //! Only gets set when ContentSetItem is created with `ContentFlags::IncludeInputKeys` flag.
    //!
    //! For `SelectedNodeInstances` and `ContentInstancesOfSpecificClasses` specifications result of this
    //! always matches result of `GetKeys()`. More than 1 item in the array is expected only when content
    //! is created with `ContentFlats::MergeResults`.
    bvector<ECClassInstanceKey> const& GetInputKeys() const {return m_inputKeys;}
    bvector<ECClassInstanceKey>& GetInputKeys() {return m_inputKeys;}

    //! Get keys of ECInstances whose values this item contains.
    bvector<ECClassInstanceKey> const& GetKeys() const {return m_keys;}

    //! Get names of merged fields in this record.
    bvector<Utf8String> const& GetMergedFieldNames() const {return m_mergedFieldNames;}

    //! Are the values of field with the specified name merged in this record.
    ECPRESENTATION_EXPORT bool IsMerged(Utf8StringCR fieldName) const;

    //! Get the ECInstance keys whose values are contained in the field with the specified name.
    ECPRESENTATION_EXPORT bvector<ECClassInstanceKey> const& GetPropertyValueKeys(FieldPropertyIdentifier const&) const;

    //! Get the ECClass whose values are contained in this record.
    //! @note May be null when the record contains multiple merged values of different classes.
    ECClassCP GetClass() const {return m_class;}
    //! Set the ECClass whose values are contained in this record.
    void SetClass(ECClassCP ecClass) {m_class = ecClass;}

    //! Get the display label definition of this item.
    LabelDefinitionCR GetDisplayLabelDefinition() const {return *m_displayLabelDefinition;}

    //! Get the image ID of this item.
    Utf8StringCR GetImageId() const {return m_imageId;}
    //! Set the image ID for this item.
    void SetImageId(Utf8String imageId) {m_imageId = imageId;}
};

//=======================================================================================
//! A struct that contains the @ref ContentDescriptor and a @ref IDataSource of @ref ContentSetItem
//! objects which are based on that descriptor.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct Content : RefCountedBase
{
private:
    ContentDescriptorCPtr m_descriptor;
    IDataSourcePtr<ContentSetItemCPtr> m_contentSource;

private:
    Content(ContentDescriptorCR descriptor, IDataSource<ContentSetItemCPtr>& contentSource) : m_descriptor(&descriptor), m_contentSource(&contentSource) {}

public:
    //! Create the @ref Content object.
    //! @param[in] descriptor The descriptor used to create the content.
    //! @param[in] contentSource The source of @ref ContentSetItem objects.
    static ContentPtr Create(ContentDescriptorCR descriptor, IDataSource<ContentSetItemCPtr>& contentSource) {return new Content(descriptor, contentSource);}

    //! Get the descriptor that was used to create the @ref ContentSetItem objects.
    ContentDescriptorCR GetDescriptor() const {return *m_descriptor;}

    //! Get the content set.
    DataContainer<ContentSetItemCPtr> GetContentSet() const {return DataContainer<ContentSetItemCPtr>(*m_contentSource);}

    //! Serialize this object to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct DisplayValueGroup : RefCountedBase
{
private:
    Utf8String m_displayValue;
    rapidjson::Document::AllocatorType m_rawValuesAllocator;
    bvector<rapidjson::Value> m_rawValues;
public:
    DisplayValueGroup(Utf8String displayValue) : m_displayValue(displayValue) {}
    Utf8StringCR GetDisplayValue() const {return m_displayValue;}
    bvector<rapidjson::Value> const& GetRawValues() const {return m_rawValues;}
    bvector<rapidjson::Value>& GetRawValues() {return m_rawValues;}
    rapidjson::Document::AllocatorType& GetRawValuesAllocator() {return m_rawValuesAllocator;}
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! Holds information about a single changed instance.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct ChangedECInstanceInfo
{
private:
    ECClassCP m_primaryInstanceClass;
    BeSQLite::EC::ECInstanceId m_primaryInstanceId;
    ECClassCP m_changedInstanceClass;
    BeSQLite::EC::ECInstanceId m_changedInstanceId;
    RelatedClassPath m_pathToPrimary;

public:
    ChangedECInstanceInfo(ECClassCR primaryInstanceClass, BeSQLite::EC::ECInstanceId primaryInstanceId)
        : m_primaryInstanceClass(&primaryInstanceClass), m_primaryInstanceId(primaryInstanceId), m_changedInstanceClass(nullptr)
        {}
    ChangedECInstanceInfo(ECClassCR primaryInstanceClass, BeSQLite::EC::ECInstanceId primaryInstanceId,
        ECClassCR changedInstanceClass, BeSQLite::EC::ECInstanceId changedInstanceId, RelatedClassPath pathToPrimaryInstance)
        : m_primaryInstanceClass(&primaryInstanceClass), m_primaryInstanceId(primaryInstanceId),
        m_changedInstanceClass(&changedInstanceClass), m_changedInstanceId(changedInstanceId), m_pathToPrimary(pathToPrimaryInstance)
        {}

    ECClassCR GetPrimaryInstanceClass() const {return *m_primaryInstanceClass;}
    BeSQLite::EC::ECInstanceId GetPrimaryInstanceId() const {return m_primaryInstanceId;}

    ECClassCR GetChangedInstanceClass() const {return (nullptr != m_changedInstanceClass) ? *m_changedInstanceClass : *m_primaryInstanceClass;}
    BeSQLite::EC::ECInstanceId GetChangedInstanceId() const {return (nullptr != m_changedInstanceClass) ? m_changedInstanceId : m_primaryInstanceId;}
    RelatedClassPath const& GetPathToPrimaryInstance() const {return m_pathToPrimary;}
};

//=======================================================================================
//! Holds the result on ECInstance change.
// @bsiclass
//=======================================================================================
struct ECInstanceChangeResult
{
private:
    BentleyStatus m_status;
    ECValue m_changedValue;
    Utf8String m_errorMessage;

private:
    ECInstanceChangeResult(BentleyStatus status) : m_status(status) {}

public:
    ECPRESENTATION_EXPORT static ECInstanceChangeResult Success(ECValue changedValue);
    ECPRESENTATION_EXPORT static ECInstanceChangeResult Error(Utf8String message);
    ECPRESENTATION_EXPORT static ECInstanceChangeResult Ignore(Utf8String reason = "");

    ECInstanceChangeResult(ECInstanceChangeResult const& other)
        : m_status(other.m_status), m_changedValue(other.m_changedValue), m_errorMessage(other.m_errorMessage)
        {}
    ECInstanceChangeResult& operator=(ECInstanceChangeResult const& other)
        {
        m_status = other.m_status;
        m_changedValue = other.m_changedValue;
        m_errorMessage = other.m_errorMessage;
        return *this;
        }

    ECInstanceChangeResult(ECInstanceChangeResult&& other)
        : m_status(other.m_status), m_changedValue(std::move(other.m_changedValue)), m_errorMessage(std::move(other.m_errorMessage))
        {}
    ECInstanceChangeResult& operator=(ECInstanceChangeResult&& other)
        {
        m_status = other.m_status;
        m_changedValue = std::move(other.m_changedValue);
        m_errorMessage = std::move(other.m_errorMessage);
        return *this;
        }

    BentleyStatus GetStatus() const {return m_status;}
    ECValueCR GetChangedValue() const {return m_changedValue;}
    Utf8StringCR GetErrorMessage() const {return m_errorMessage;}
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! An interface for a ECProperty value formatter.
//! @note Formatter methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct IECPropertyFormatter
{
protected:
    //! @see GetFormattedPropertyValue
    virtual BentleyStatus _GetFormattedPropertyValue(Utf8StringR, ECPropertyCR, ECValueCR, UnitSystem) const = 0;
    virtual BentleyStatus _GetFormattedPropertyLabel(Utf8StringR, ECPropertyCR, ECClassCR, RelatedClassPath const&, RelationshipMeaning) const = 0;
    virtual Formatting::Format const* _GetActiveFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystem) const = 0;

public:
    //! Virtual destructor.
    virtual ~IECPropertyFormatter() {}

    //! Formats the supplied ECValue.
    //! @param[out] formattedValue The formatted value.
    //! @param[in] ecProperty The property whose value is being formatted.
    //! @param[in] ecValue The value to format.
    //! @param[in] unitSystem Unit system to use when formatting
    //! @return SUCCESS if the value was successfully formatted.
    BentleyStatus GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue, UnitSystem unitSystem) const
        {
        return _GetFormattedPropertyValue(formattedValue, ecProperty, ecValue, unitSystem);
        }

    //! Returns formatted label for the supplied ECProperty.
    //! @param[out] formattedLabel The formatted property label.
    //! @param[in] ecProperty The property whose label is being formatted.
    //! @param[in] propertyClass The class of ECProperty whose label is being formatted.
    //! @param[in] relatedClassPath Relationship path from the displayed ECInstance class to the supplied ECProperty class.
    //! @param[in] relationshipMeaning Meaning of relationship between displayed ECInstace class and ECProperty class.
    //! @return SUCCESS if the label was successfully formatted.
    BentleyStatus GetFormattedPropertyLabel(Utf8StringR formattedLabel, ECPropertyCR ecProperty, ECClassCR propertyClass,
        RelatedClassPath const& pathFromSelectToPropertyClass, RelationshipMeaning relationshipMeaning) const
        {
        return _GetFormattedPropertyLabel(formattedLabel, ecProperty, propertyClass, pathFromSelectToPropertyClass, relationshipMeaning);
        }

    //! Returns kind of quantity presentation format matching supplied unit system.
    //! @param[in] koq Kind of quantity whose presentation format to look for.
    //! @param[in] unitSystemt Unit system that presentation format should match.
    //! @return Formatting::Format const* pointing to the kind of quantity format that matches supplied unitSystem or default presentation format.
    Formatting::Format const* GetActiveFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystem) const
        {
        return _GetActiveFormat(koq, unitSystem);
        }
};

//=======================================================================================
//! An interface for a ECProperty value formatter.
//! @note Formatter methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct DefaultPropertyFormatter : IECPropertyFormatter
{
private:
    std::map<std::pair<Utf8String, UnitSystem>, std::shared_ptr<Formatting::Format>> m_defaultFormats;
protected:
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyEnumFormatting(Utf8StringR, ECPropertyCR, ECValueCR) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyKoqFormatting(Utf8StringR, ECPropertyCR, ECValueCR, UnitSystem) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyPoint3dFormatting(Utf8StringR, DPoint3dCR) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyPoint2dFormatting(Utf8StringR, DPoint2dCR) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyDoubleFormatting(Utf8StringR, double) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyDateTimeFormatting(Utf8StringR, DateTimeCR) const;
protected:
    ECPRESENTATION_EXPORT virtual BentleyStatus _GetFormattedPropertyValue(Utf8StringR, ECPropertyCR, ECValueCR, UnitSystem) const override;
    ECPRESENTATION_EXPORT virtual BentleyStatus _GetFormattedPropertyLabel(Utf8StringR, ECPropertyCR, ECClassCR, RelatedClassPath const&, RelationshipMeaning) const override;
    ECPRESENTATION_EXPORT virtual Formatting::Format const* _GetActiveFormat(KindOfQuantityCR koq, ECPresentation::UnitSystem unitSystem) const override;
    public:
     DefaultPropertyFormatter(std::map<std::pair<Utf8String, UnitSystem>, std::shared_ptr<Formatting::Format>> defaultFormats = std::map<std::pair<Utf8String, UnitSystem>, std::shared_ptr<Formatting::Format>>())
        {
        m_defaultFormats = defaultFormats;
        }
    void SetDefaultFormats(std::map<std::pair<Utf8String, UnitSystem>, std::shared_ptr<Formatting::Format>> defaultFormats)
        {
        m_defaultFormats = defaultFormats;
        }
};

//=======================================================================================
//! Interface for a category supplier which is used to determine categories for
//! @ref ContentDescriptor::Field.
//! @note Supplied methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct IPropertyCategorySupplier
{
protected:
    virtual std::unique_ptr<ContentDescriptor::Category> _CreateDefaultCategory() const = 0;
    virtual std::unique_ptr<ContentDescriptor::Category> _CreateECClassCategory(ECClassCR) const = 0;
    virtual std::unique_ptr<ContentDescriptor::Category> _CreatePropertyCategory(ECPropertyCR) const = 0;

public:
    //! Virtual destructor.
    virtual ~IPropertyCategorySupplier() {}

    std::unique_ptr<ContentDescriptor::Category> CreateDefaultCategory() const {return _CreateDefaultCategory();}

    //! Called to create a category based on specific ECClass
    std::unique_ptr<ContentDescriptor::Category> CreateECClassCategory(ECClassCR ecClass) const {return _CreateECClassCategory(ecClass);}

    //! Called to create a category based on ECProperty's category. Should return invalid category if property is not categorized.
    std::unique_ptr<ContentDescriptor::Category> CreatePropertyCategory(ECPropertyCR ecProperty) const {return _CreatePropertyCategory(ecProperty);}
};

//=======================================================================================
//! Default property category supplier that uses the "Category" custom attribute to determine
//! the ECProperty category. If no such custom attribute exists, the "Miscellaneous" category
//! is returned.
//! @ingroup GROUP_Presentation_Content
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefaultCategorySupplier : IPropertyCategorySupplier
{
protected:
    ECPRESENTATION_EXPORT virtual std::unique_ptr<ContentDescriptor::Category> _CreateDefaultCategory() const override;
    ECPRESENTATION_EXPORT virtual std::unique_ptr<ContentDescriptor::Category> _CreateECClassCategory(ECClassCR) const override;
    ECPRESENTATION_EXPORT virtual std::unique_ptr<ContentDescriptor::Category> _CreatePropertyCategory(ECPropertyCR) const override;
public:
    DefaultCategorySupplier() {}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
