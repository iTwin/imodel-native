/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <ECPresentation/ECPresentationTypes.h>
#include <ECPresentation/DataSource.h>
#include <ECPresentation/ExtendedData.h>
#include <ECPresentation/KeySet.h>
#include <ECPresentation/LabelDefinition.h>
#include <ECPresentation/RulesDriven/Rules/RelatedPropertiesSpecification.h>
#include <ECPresentation/RulesDriven/Rules/InstanceLabelOverride.h>

#include <ECDb/ECInstanceId.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define CONTENTRECORD_MERGED_VALUE_FORMAT   "*** %s ***"

//=======================================================================================
//! A struct that describes current selection.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                04/2016
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
// @bsiclass                                    Grigas.Petraitis                07/2016
//=======================================================================================
enum class ContentFlags
    {
    KeysOnly =              1,      //!< Each content record has only ECInstanceKey and no data
    ShowImages =            1 << 1, //!< Each content record additionally has an image id
    ShowLabels =            1 << 2, //!< Each content record additionally has a label
    MergeResults =          1 << 3, //!< All content records are merged into a single record
    DistinctValues =        1 << 4, //!< Content has only distinct values
    NoFields =              1 << 5, //!< Doesnt create property or calculated fields. Can be used in conjunction with @e ShowLabels.
    ExcludeEditingData =    1 << 6, //!< Should editing data be excluded from the content. This flag increases performance and should be used when requesting data for read-only cases.
    SkipInstancesCheck =    1 << 7, //!< Skip instances' check when creating content with ContentRelatedInstances specification.
    };

//=======================================================================================
//! Displayed content types. Affects how the content is formatted, e.g.
//! the ContentFlags
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                05/2016
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
// @bsiclass                                    Grigas.Petraitis                05/2016
//=======================================================================================
struct SelectClassInfo
{
private:
    SelectClassWithExcludes m_selectClass;
    RelatedClassPath m_pathFromInputToSelectClass;
    bvector<RelatedClassPath> m_relatedPropertyPaths;
    bvector<RelatedClass> m_navigationPropertyClasses;
    bvector<RelatedClassPath> m_relatedInstancePaths;

public:
    //! Constructor. Creates an invalid object.
    SelectClassInfo() {}
    //! Constructor. Creates an information instance with the specified ECClass.
    SelectClassInfo(SelectClassWithExcludes const& selectClass) : m_selectClass(selectClass) {}
    SelectClassInfo(SelectClassWithExcludes&& selectClass) : m_selectClass(std::move(selectClass)) {}
    SelectClassInfo(ECClassCR ecClass, bool isSelectPolymorphic) : m_selectClass(SelectClassWithExcludes(ecClass, isSelectPolymorphic)) {}
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
    SelectClassWithExcludes const& GetSelectClass() const {return m_selectClass;}
    SelectClassWithExcludes& GetSelectClass() {return m_selectClass;}

    //! Get the input ECClass.
    ECClassCP GetInputClass() const {return m_pathFromInputToSelectClass.empty() ? nullptr : m_pathFromInputToSelectClass.front().GetSourceClass();}

    //! Path from input to select ECClass.
    RelatedClassPath const& GetPathFromInputToSelectClass() const {return m_pathFromInputToSelectClass;}
    void SetPathFromInputToSelectClass(RelatedClassPath path) {m_pathFromInputToSelectClass = path;}

    //! Get paths to related property ECClasses.
    bvector<RelatedClassPath> const& GetRelatedPropertyPaths() const {return m_relatedPropertyPaths;}
    //! Set paths to related property ECClasses.
    void SetRelatedPropertyPaths(bvector<RelatedClassPath> propertyPaths) {m_relatedPropertyPaths = propertyPaths;}

    //! Get navigation property ECClasses.
    bvector<RelatedClass> const& GetNavigationPropertyClasses() const {return m_navigationPropertyClasses;}
    //! Set navigation property ECClasses.
    void SetNavigationPropertyClasses(bvector<RelatedClass> classes) {m_navigationPropertyClasses = classes;}

    //! Get related classes of related instances.
    bvector<RelatedClassPath> const& GetRelatedInstancePaths() const {return m_relatedInstancePaths;}
    bvector<RelatedClassPath>& GetRelatedInstancePaths() {return m_relatedInstancePaths;}
    //! Set related classes of related instances.
    void SetRelatedInstancePaths(bvector<RelatedClassPath> paths) {m_relatedInstancePaths = paths;}
};

//=======================================================================================
//! Describes content field editor.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct ContentFieldEditor
{
    struct Params
    {
    protected:
        virtual Utf8CP _GetName() const = 0;
        virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType*) const = 0;
        virtual Params* _Clone() const = 0;
        virtual int _CompareTo(Params const& other) const {return strcmp(GetName(), other.GetName());}
    public:
        virtual ~Params() {}
        Utf8CP GetName() const {return _GetName();}
        rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(allocator);}
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
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ContentDescriptor : RefCountedBase
{
    //===================================================================================
    //! A struct that describes a @ref Field category.
    // @bsiclass                                    Grigas.Petraitis            09/2016
    //===================================================================================
    struct Category
    {
    private:
        Utf8String m_name;
        Utf8String m_label;
        Utf8String m_description;
        bool m_shouldExpand;
        int m_priority;

    public:
        //! Constructor. Creates an invalid category.
        Category() : m_shouldExpand(false), m_priority(0) {}

        //! Copy constructor.
        Category(Category const& other) : m_name(other.m_name), m_label(other.m_label), m_priority(other.m_priority), m_description(other.m_description), m_shouldExpand(other.m_shouldExpand) {}

        //! Move constructor.
        Category(Category&& other) : m_name(std::move(other.m_name)), m_label(std::move(other.m_label)), m_priority(other.m_priority), m_description(other.m_description), m_shouldExpand(other.m_shouldExpand) {}

        //! Constructor.
        //! @param[in] name Name of the category.
        //! @param[in] label Label of the category.
        //! @param[in] description Description of the category.
        //! @param[in] priority Priority of the category.
        //! @param[in] shouldExpand Should this category be auto-expanded.
        Category(Utf8String name, Utf8String label, Utf8String description, int priority, bool shouldExpand = false) : m_name(name), m_label(label), m_priority(priority), m_description(description), m_shouldExpand(shouldExpand) {}

        //! Copy-assignment operator.
        Category& operator=(Category const& other) {m_name = other.m_name; m_label = other.m_label; m_priority = other.m_priority; m_description = other.m_description;  m_shouldExpand = other.m_shouldExpand; return *this;}

        //! Is this category equal to the supplied one.
        bool operator==(Category const& other) const {return m_name == other.m_name && m_label == other.m_label && m_priority == other.m_priority && m_description == other.m_description && m_shouldExpand == other.m_shouldExpand;}

        //! Is this category valid
        bool IsValid() const {return !m_name.empty();}

        //! Serialize this category to JSON.
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

        //! Should this category be automatically expanded.
        bool ShouldExpand() const {return m_shouldExpand;}
        //! Set whether this category should be automatically expanded.
        void SetShouldExpand(bool value) {m_shouldExpand = value;}

    public:
        ECPRESENTATION_EXPORT static Category GetDefaultCategory();
        ECPRESENTATION_EXPORT static Category GetFavoriteCategory();
        ECPRESENTATION_EXPORT static Category FromSpec(PropertyCategorySpecificationCR);
        ECPRESENTATION_EXPORT static Category FromSpec(Utf8StringCR categoryId, PropertyCategorySpecificationsList const&);
    };

    //===================================================================================
    //! Describes a single ECProperty that's included in a @ref Field.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct Property
    {
        ECPRESENTATION_EXPORT static int const DEFAULT_PRIORITY;
    private:
        Utf8String m_prefix;
        ECClassCP m_propertyClass;
        ECPropertyCP m_property;
        RelatedClassPath m_pathFromSelectToPropertyClass;
        RelationshipMeaning m_relationshipMeaning;

    private:
        ECPRESENTATION_EXPORT static PrimitiveECPropertyCP GetPrimitiveProperty(StructECPropertyCR, Utf8StringCR accessString);

    public:
        //! Constructor. Creates a property for a primitive ECProperty.
        //! @param[in] prefix Class alias that's used to query this property.
        //! @param[in] propertyClass The exact class of this property.
        //! @param[in] ecProperty The ECProperty that's wrapped by this struct.
        Property(Utf8String prefix, ECClassCR propertyClass, ECPropertyCR ecProperty)
            : m_prefix(prefix), m_propertyClass(&propertyClass), m_property(&ecProperty), m_relationshipMeaning(RelationshipMeaning::SameInstance)
            {}

        //! Is this struct equal to the supplied one.
        ECPRESENTATION_EXPORT bool operator==(Property const& other) const;

        //! Serialize this struct to JSON.
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

        //! Get the class alias used to query this property.
        Utf8CP GetPrefix() const {return m_prefix.c_str();}
        //! Set the class alias used to query this property.
        void SetPrefix(Utf8String prefix) {m_prefix = prefix;}

        // Get the exact class that the root wrapped property belongs to.
        ECClassCR GetPropertyClass() const {return *m_propertyClass;}

        //! Get the wrapped property.
        ECPropertyCR GetProperty() const {return *m_property;}

        //! Is this a related property.
        bool IsRelated() const {return !m_pathFromSelectToPropertyClass.empty();}
        //! Make this a related property.
        //! @param[in] path The relationship path that describes the relationship between this property and main properties.
        //! @param[in] meaning The relationship meaning.
        void SetIsRelated(RelatedClassPath path, RelationshipMeaning meaning) {m_pathFromSelectToPropertyClass = path; m_relationshipMeaning = meaning;}
        //! Make this a related property.
        //! @param[in] relatedClass The related class object that describes the relationship between this property and main properties.
        //! @param[in] meaning The relationship meaning.
        void SetIsRelated(RelatedClass relatedClass, RelationshipMeaning meaning) {m_pathFromSelectToPropertyClass.clear(); m_pathFromSelectToPropertyClass.push_back(relatedClass); m_relationshipMeaning = meaning;}
        //! Get the relationship path that describes the relationship between this property and main properties.
        RelatedClassPathCR GetPathFromSelectToPropertyClass() const {return m_pathFromSelectToPropertyClass;}
        //! Get the relationship meaning. Always SameInstance if this is not a related property.
        RelationshipMeaning GetRelationshipMeaning() const {return m_relationshipMeaning;}
        //! Get the priority of this property.
        int GetPriority() const {return m_property->GetPriority();}
    };

    struct DisplayLabelField;
    struct ECPropertiesField;
    struct CalculatedPropertyField;
    struct NestedContentField;
    struct SystemField;
    //===================================================================================
    //! Describes a single content field. A field is usually represented as a grid column
    //! or a property pane row.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct Field
    {
        struct EXPORT_VTABLE_ATTRIBUTE TypeDescription : RefCountedBase
        {
        private:
            Utf8String m_typeName;
        protected:
            TypeDescription(Utf8String typeName) : m_typeName(typeName) {}
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const;
        public:
            ECPRESENTATION_EXPORT static RefCountedPtr<TypeDescription> Create(ECPropertyCR);
            Utf8StringCR GetTypeName() const {return m_typeName;}
            rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(allocator);}
        };
        typedef RefCountedPtr<TypeDescription> TypeDescriptionPtr;

        //===================================================================================
        // @bsiclass                                    Grigas.Petraitis            09/2017
        //===================================================================================
        struct PrimitiveTypeDescription : TypeDescription
        {
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        public:
            PrimitiveTypeDescription(Utf8String type) : TypeDescription(type) {}
        };

        //===================================================================================
        // @bsiclass                                    Grigas.Petraitis            09/2017
        //===================================================================================
        struct ArrayTypeDescription : TypeDescription
        {
        private:
            TypeDescriptionPtr m_memberType;
        private:
            ECPRESENTATION_EXPORT static Utf8String CreateTypeName(TypeDescription const& memberType);
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        public:
            ArrayTypeDescription(TypeDescription& memberType) : TypeDescription(CreateTypeName(memberType)), m_memberType(&memberType) {}
            TypeDescriptionPtr GetMemberType() const {return m_memberType;}
        };

        //===================================================================================
        // @bsiclass                                    Grigas.Petraitis            09/2017
        //===================================================================================
        struct StructTypeDescription : TypeDescription
        {
        private:
            ECStructClassCR m_struct;
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        public:
            StructTypeDescription(ECStructClassCR structClass) : TypeDescription(structClass.GetName()), m_struct(structClass) {}
            ECStructClassCR GetStruct() const {return m_struct;}
        };

        //===================================================================================
        // @bsiclass                                    Grigas.Petraitis            09/2017
        //===================================================================================
        struct NestedContentTypeDescription : TypeDescription
        {
        private:
            NestedContentField const& m_field;
        protected:
            ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        public:
            NestedContentTypeDescription(ContentDescriptor::NestedContentField const& field) : TypeDescription(field.GetContentClass().GetDisplayLabel()), m_field(field) {}
            NestedContentField const& GetNestedContentField() const {return m_field;}
        };

    private:
        Category m_category;
        Utf8String m_uniqueName;
        Utf8String m_label;
        ContentFieldEditor const* m_editor;
        mutable TypeDescriptionPtr m_type;

    protected:
        Field() : m_editor(nullptr) {}
        virtual DisplayLabelField* _AsDisplayLabelField() {return nullptr;}
        virtual DisplayLabelField const* _AsDisplayLabelField() const {return nullptr;}
        virtual CalculatedPropertyField* _AsCalculatedPropertyField() {return nullptr;}
        virtual CalculatedPropertyField const* _AsCalculatedPropertyField() const {return nullptr;}
        virtual ECPropertiesField* _AsPropertiesField() {return nullptr;}
        virtual ECPropertiesField const* _AsPropertiesField() const {return nullptr;}
        virtual NestedContentField* _AsNestedContentField() {return nullptr;}
        virtual NestedContentField const* _AsNestedContentField() const {return nullptr;}
        virtual SystemField* _AsSystemField() {return nullptr;}
        virtual SystemField const* _AsSystemField() const {return nullptr;}
        virtual Field* _Clone() const = 0;
        virtual TypeDescriptionPtr _CreateTypeDescription() const = 0;
        virtual bool _IsReadOnly() const = 0;
        virtual bool _IsVisible() const {return true;}
        virtual bool _Equals(Field const& other) const {return GetCategory() == other.GetCategory() && m_uniqueName.Equals(other.m_uniqueName) && GetLabel().Equals(other.GetLabel());}
        virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const = 0;
        virtual int _GetPriority() const = 0;
        virtual void _OnFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo) {}
        virtual bool _OnFieldRemoved(Field const&) {return false;}
        virtual Utf8String _CreateName() const = 0;

    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label The label of this field.
        //! @param[in] editor The custom editor for this field.
        Field(Category category, Utf8String label, ContentFieldEditor const* editor = nullptr)
            : m_category(category), m_label(label), m_editor(nullptr)
            {
            if (editor)
                m_editor = new ContentFieldEditor(*editor);
            }

        //! Constructor.
        Field(Category category, Utf8String uniqueName, Utf8String label, ContentFieldEditor const* editor = nullptr)
            : Field(category, label, editor)
            {
            m_uniqueName = uniqueName;
            }

        //! Copy constructor.
        Field(Field const& other)
            : m_category(other.m_category), m_uniqueName(other.m_uniqueName), m_label(other.m_label), m_editor(other.m_editor)
            {
            if (nullptr != other.m_editor)
                m_editor = new ContentFieldEditor(*other.m_editor);
            }

        //! Move constructor.
        Field(Field&& other)
            : m_category(std::move(other.m_category)), m_uniqueName(std::move(other.m_uniqueName)), m_label(std::move(other.m_label)),
            m_editor(std::move(other.m_editor))
            {}

        //! Virtual destructor.
        virtual ~Field() {DELETE_AND_CLEAR(m_editor);}

        //! Clone this field.
        Field* Clone() const {return _Clone();}

        //! Serialize this field to JSON.
        rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const {return _AsJson(allocator);}

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

        //! Is this a system field.
        bool IsSystemField() const {return nullptr != _AsSystemField();}
        //! Get this field as a @ref SystemField
        SystemField* AsSystemField() {return _AsSystemField();}
        //! Get this field as a @ref SystemField
        SystemField const* AsSystemField() const {return _AsSystemField();}

        //! Get the category of this field.
        Category const& GetCategory() const {return m_category;}
        //! Set the category for this field.
        void SetCategory(Category category) {m_category = category;}

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

        //! Get the editor of this field.
        ContentFieldEditor const* GetEditor() const {return m_editor;}
        //! Set the editor for this field.
        //! @note The field takes ownership of the editor object.
        void SetEditor(ContentFieldEditor const* editor) {m_editor = editor;}

        //! Get field's priority.
        int GetPriority() const {return _GetPriority();}

        //! Is field read only
        bool IsReadOnly() const {return _IsReadOnly();}

        //! Get field's type information.
        ECPRESENTATION_EXPORT TypeDescription const& GetTypeDescription() const;

        //! Call this after related sibling fields were remapped (e.g. after copying)
        void NotifyFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo) {_OnFieldsCloned(fieldsRemapInfo);}

        //! Call this after a related sibling field is removed from descriptor.
        //! @return True if this field should also be removed.
        bool NotifyFieldRemoved(Field const& field) {return _OnFieldRemoved(field);}
    };

    //===================================================================================
    //! Describes a single content display label field. This field is usually included in
    //! content for the @ref ContentDisplayType::Grid display type.
    // @bsiclass                                    Grigas.Petraitis            10/2016
    //===================================================================================
    struct DisplayLabelField : Field
    {
        ECPRESENTATION_EXPORT static const Utf8CP NAME;

    private:
        int m_priority;
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> m_labelOverrideValueSpecs;

    private:
        ECPRESENTATION_EXPORT static bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> CloneLabelOverrideValueSpecs(bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> const&);

    protected:
        DisplayLabelField* _AsDisplayLabelField() override {return this;}
        DisplayLabelField const* _AsDisplayLabelField() const override {return this;}
        Field* _Clone() const override {return new DisplayLabelField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        int _GetPriority() const override {return m_priority;}
        bool _IsReadOnly() const override {return true;}
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        Utf8String _CreateName() const override {return NAME;}

    public:
        //! Constructor.
        //! @param[in] label The label of this field.
        //! @param[in] priority Field priority.
        DisplayLabelField(Utf8String label, int priority = Property::DEFAULT_PRIORITY)
            : Field(Category::GetDefaultCategory(), NAME, label), m_priority(priority)
            {}
        DisplayLabelField(DisplayLabelField const& other) : Field(other), m_priority(other.m_priority), m_labelOverrideValueSpecs(CloneLabelOverrideValueSpecs(other.m_labelOverrideValueSpecs)) {}
        DisplayLabelField(DisplayLabelField&& other) : Field(other), m_priority(other.m_priority) {m_labelOverrideValueSpecs.swap(other.m_labelOverrideValueSpecs);}
        ECPRESENTATION_EXPORT ~DisplayLabelField();

        //! Set the priority for this field.
        void SetPriority(int priority) {m_priority = priority;}

        //! Get a map of label override value specifications
        bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> const& GetOverrideValueSpecs() const {return m_labelOverrideValueSpecs;}

        //! Set label override value specifications' map
        void SetOverrideValueSpecs(bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> const& specs) {m_labelOverrideValueSpecs = CloneLabelOverrideValueSpecs(specs);}
    };

    //===================================================================================
    //! Describes a single content calculated property field. Calculated fields use
    //! ECExpressions to determine the value.
    // @bsiclass                                     Tautvydas.Zinys            10/2016
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
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        Utf8String _CreateName() const override {return m_requestedName;}

    public:
        //! Constructor.
        //! @param[in] label Field label. Supports localization.
        //! @param[in] name Unique field name.
        //! @param[in] valueExpression Value ECExpression. Supports localization.
        //! @param[in] ecClass Entity class this field is intended for.
        //! @param[in] priority Field priority.
        CalculatedPropertyField(Utf8String label, Utf8String name, Utf8String valueExpression, ECClassCP ecClass, int priority = Property::DEFAULT_PRIORITY)
            : Field(Category::GetDefaultCategory(), label), m_requestedName(name), m_valueExpression(valueExpression), m_class(ecClass), m_priority(priority)
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
    // @bsiclass                                    Grigas.Petraitis            10/2016
    //===================================================================================
    struct ECPropertiesField : Field
    {
    private:
        bvector<Property> m_properties;
        mutable std::unordered_map<ECClassCP, bvector<Property const*>> m_matchingPropertiesCache;

    protected:
        ECPropertiesField* _AsPropertiesField() override {return this;}
        ECPropertiesField const* _AsPropertiesField() const override {return this;}
        Field* _Clone() const override {return new ECPropertiesField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        ECPRESENTATION_EXPORT bool _IsReadOnly() const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT int _GetPriority() const override;
        ECPRESENTATION_EXPORT Utf8String _CreateName() const override;
    public:
        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label The label of this field.
        //! @param[in] editor The custom editor for this field.
        ECPropertiesField(Category category, Utf8String label, ContentFieldEditor const* editor = nullptr)
            : Field(category, label, editor)
            {}

        //! Constructor.
        ECPropertiesField(Category category, Utf8String uniqueName, Utf8String label, ContentFieldEditor const* editor = nullptr)
            : Field(category, uniqueName, label, editor)
            {}

        //! Constructor. Creates a field with a single @ref Property.
        //! @param[in] category The category of this field.
        //! @param[in] prop Property that this field is based on.
        ECPRESENTATION_EXPORT ECPropertiesField(Category category, Property const& prop);

        //! Constructor.
        ECPropertiesField(Category category, Utf8String uniqueName, Property const& prop)
            : ECPropertiesField(category, prop)
            {
            SetUniqueName(uniqueName);
            }

        //! Copy constructor.
        ECPropertiesField(ECPropertiesField const& other) : Field(other), m_properties(other.m_properties) {}

        //! Move constructor.
        ECPropertiesField(ECPropertiesField&& other) : Field(std::move(other)), m_properties(std::move(other.m_properties)) {}

        //! Is this field equal to the supplied one.
        bool operator==(ECPropertiesField const& other) const {return Field::operator==(other) && m_properties == other.m_properties;}

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
    };

    struct CompositeContentField;
    struct RelatedContentField;
    //===================================================================================
    //! Describes a single content field which by itself describes content. Creating content
    //! for this field requires executing a separate query. Examples of such content could be getting
    //! content for related instances or composite structures like arrays or structs.
    // @bsiclass                                    Grigas.Petraitis            07/2017
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
        ECPRESENTATION_EXPORT virtual rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
        ECPRESENTATION_EXPORT virtual Utf8String _CreateName() const override;
        virtual ECClassCR _GetContentClass() const = 0;
        virtual Utf8CP _GetContentClassAlias() const = 0;
        virtual CompositeContentField* _AsCompositeContentField() {return nullptr;}
        virtual RelatedContentField* _AsRelatedContentField() {return nullptr;}

        //! Constructor.
        //! @param[in] category The category of this field.
        //! @param[in] label The label of this field.
        //! @param[in] fields A list of fields which this field consists from.
        //! @param[in] autoExpand Flag specifying if this field should be expanded.
        //! @param[in] priority Priority of the field
        NestedContentField(Category category, Utf8String label, bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : Field(category, label), m_fields(fields), m_priority(priority), m_autoExpand(autoExpand)
            {}

        //! Constructor.
        NestedContentField(Category category, Utf8String uniqueName, Utf8String label, bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : Field(category, uniqueName, label), m_fields(fields), m_priority(priority), m_autoExpand(autoExpand)
            {}

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
        ~NestedContentField() {for (Field const* field : m_fields) {DELETE_AND_CLEAR(field);}}

        CompositeContentField* AsCompositeContentField() {return _AsCompositeContentField();}
        CompositeContentField const* AsCompositeContentField() const {return const_cast<NestedContentField*>(this)->AsCompositeContentField();}
        RelatedContentField* AsRelatedContentField() {return _AsRelatedContentField();}
        RelatedContentField const* AsRelatedContentField() const {return const_cast<NestedContentField*>(this)->AsRelatedContentField();}

        //! Get the content class whose content is returned by this field.
        ECClassCR GetContentClass() const {return _GetContentClass();}

        //! Get alias of the content class
        Utf8CP GetContentClassAlias() const {return _GetContentClassAlias();}

        //! A list of fields which this field consists from.
        bvector<Field*> const& GetFields() const {return m_fields;}
        bvector<Field*>& GetFields() {return m_fields;}

        //! Should this field be automatically expanded.
        bool ShouldAutoExpand() const {return m_autoExpand;}
        };

    //===================================================================================
    //! Describes a single content field which contains composite (struct, array) content.
    // @bsiclass                                    Grigas.Petraitis            12/2019
    //===================================================================================
    struct CompositeContentField : NestedContentField
    {
    private:
        ECClassCR m_contentClass;
        Utf8String m_contentClassAlias;
    protected:
        ECClassCR _GetContentClass() const override {return m_contentClass;}
        Utf8CP _GetContentClassAlias() const override {return m_contentClassAlias.c_str();}
        virtual CompositeContentField* _AsCompositeContentField() override {return this;}
        Field* _Clone() const override {return new CompositeContentField(*this);}
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    public:
        CompositeContentField(Category category, Utf8String label, ECClassCR contentClass, Utf8String contentClassAlias,
            bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : NestedContentField(category, label, fields, autoExpand, priority), m_contentClass(contentClass),
            m_contentClassAlias(contentClassAlias)
            {}
        CompositeContentField(Category category, Utf8String uniqueName, Utf8String label, ECClassCR contentClass, Utf8String contentClassAlias,
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
    // @bsiclass                                    Grigas.Petraitis            12/2019
    //===================================================================================
    struct RelatedContentField : NestedContentField
    {
    private:
        RelatedClassPath m_pathFromSelectClassToContentClass;
    protected:
        ECClassCR _GetContentClass() const override {return m_pathFromSelectClassToContentClass.back().GetTargetClass().GetClass();}
        Utf8CP _GetContentClassAlias() const override {return m_pathFromSelectClassToContentClass.back().GetTargetClassAlias();}
        virtual RelatedContentField* _AsRelatedContentField() override {return this;}
        Field* _Clone() const override {return new RelatedContentField(*this);}
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT Utf8String _CreateName() const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    public:
        RelatedContentField(Category category, Utf8String label, RelatedClassPath pathFromSelectClassToContentClass,
            bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : NestedContentField(category, label, fields, autoExpand, priority), m_pathFromSelectClassToContentClass(pathFromSelectClassToContentClass)
            {}
        RelatedContentField(Category category, Utf8String uniqueName, Utf8String label, RelatedClassPath pathFromSelectClassToContentClass,
            bvector<Field*> fields = bvector<Field*>(), bool autoExpand = false, int priority = Property::DEFAULT_PRIORITY)
            : NestedContentField(category, uniqueName, label, fields, autoExpand, priority), m_pathFromSelectClassToContentClass(pathFromSelectClassToContentClass)
            {}
        RelatedContentField(RelatedContentField const& other)
            : NestedContentField(other), m_pathFromSelectClassToContentClass(other.m_pathFromSelectClassToContentClass)
            {}
        RelatedContentField(RelatedContentField&& other)
            : NestedContentField(std::move(other)), m_pathFromSelectClassToContentClass(std::move(other.m_pathFromSelectClassToContentClass))
            {}

        //! Path from the select class to content class.
        RelatedClassPath& GetPathFromSelectToContentClass() {return m_pathFromSelectClassToContentClass;}
        RelatedClassPath const& GetPathFromSelectToContentClass() const {return m_pathFromSelectClassToContentClass;}
        void SetPathFromSelectToContentClass(RelatedClassPath path) {m_pathFromSelectClassToContentClass = path;}

        Utf8CP GetSelectClassAlias() const {return "related";}
    };

    struct ECInstanceKeyField;
    struct ECNavigationInstanceIdField;
    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            06/2017
    //===================================================================================
    struct SystemField : Field
    {
    protected:
        SystemField* _AsSystemField() override {return this;}
        SystemField const* _AsSystemField() const override {return this;}
        int _GetPriority() const override {return Property::DEFAULT_PRIORITY;}
        bool _IsReadOnly() const override {return true;}
        bool _IsVisible() const override {return false;}
    protected:
        SystemField() : Field(Category(), "") {}
        virtual ECInstanceKeyField* _AsECInstanceKeyField() {return nullptr;}
        virtual ECInstanceKeyField const* _AsECInstanceKeyField() const {return nullptr;}
        virtual ECNavigationInstanceIdField* _AsECNavigationInstanceIdField() {return nullptr;}
        virtual ECNavigationInstanceIdField const* _AsECNavigationInstanceIdField() const {return nullptr;}
    public:
        bool IsECInstanceKeyField() const {return nullptr != _AsECInstanceKeyField();}
        ECInstanceKeyField* AsECInstanceKeyField() {return _AsECInstanceKeyField();}
        ECInstanceKeyField const* AsECInstanceKeyField() const {return _AsECInstanceKeyField();}
        bool IsECNavigationInstanceIdField() const {return nullptr != _AsECNavigationInstanceIdField();}
        ECNavigationInstanceIdField* AsECNavigationInstanceIdField() {return _AsECNavigationInstanceIdField();}
        ECNavigationInstanceIdField const* AsECNavigationInstanceIdField() const {return _AsECNavigationInstanceIdField();}
    };

    //===================================================================================
    // @bsiclass                                    Grigas.Petraitis            06/2017
    //===================================================================================
    struct ECInstanceKeyField : SystemField
    {
    private:
        bvector<ContentDescriptor::ECPropertiesField const*> m_keyFields;
    protected:
        ECInstanceKeyField* _AsECInstanceKeyField() override {return this;}
        ECInstanceKeyField const* _AsECInstanceKeyField() const override {return this;}
        Field* _Clone() const override {return new ECInstanceKeyField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        ECPRESENTATION_EXPORT void _OnFieldsCloned(bmap<Field const*, Field const*> const& fieldsRemapInfo) override;
        ECPRESENTATION_EXPORT bool _OnFieldRemoved(Field const&) override;
        ECPRESENTATION_EXPORT Utf8String _CreateName() const override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    public:
        ECInstanceKeyField() : SystemField() {}
        bvector<ContentDescriptor::ECPropertiesField const*> const& GetKeyFields() const {return m_keyFields;}
        void AddKeyField(ContentDescriptor::ECPropertiesField const& field) {m_keyFields.push_back(&field);}
    };

    //===================================================================================
    // @bsiclass                                    Saulius.Skliutas            08/2017
    //===================================================================================
    struct ECNavigationInstanceIdField : SystemField
    {
    private:
        ECPropertiesField const* m_propertyField;
    protected:
        ECNavigationInstanceIdField* _AsECNavigationInstanceIdField() override {return this;}
        ECNavigationInstanceIdField const* _AsECNavigationInstanceIdField() const override {return this;}
        Field* _Clone() const override {return new ECNavigationInstanceIdField(*this);}
        ECPRESENTATION_EXPORT TypeDescriptionPtr _CreateTypeDescription() const override;
        ECPRESENTATION_EXPORT bool _Equals(Field const& other) const override;
        ECPRESENTATION_EXPORT Utf8String _CreateName() const override;
        ECPRESENTATION_EXPORT void _OnFieldsCloned(bmap<Field const*, Field const*> const&) override;
        ECPRESENTATION_EXPORT bool _OnFieldRemoved(Field const&) override;
        ECPRESENTATION_EXPORT rapidjson::Document _AsJson(rapidjson::Document::AllocatorType* allocator) const override;
    public:
        ECNavigationInstanceIdField(ECPropertiesField const& propertiesField)
            : SystemField(), m_propertyField(&propertiesField)
            {}
        ECNavigationInstanceIdField(ECNavigationInstanceIdField const& other)
            : SystemField(other), m_propertyField(other.m_propertyField)
            {}
        ECPropertiesField const& GetPropertiesField() const {return *m_propertyField;}
    };

    //===================================================================================
    // @bsiclass                                    Saulius.Skliutas            11/2017
    //===================================================================================
    struct ECPropertiesFieldKey
    {
    private:
        ECPropertyCP m_property;
        ECClassCP m_propertyClass;
        RelatedClassPath m_pathFromSelectToPropertyClass;
        ValueKind m_valueKind;
        Utf8String m_fieldLabel;
        Utf8String m_type;
        ContentFieldEditor const* m_fieldEditor;
        KindOfQuantityCP m_koq;
        Utf8String m_categoryName;
    public:
        ECPropertiesFieldKey()
            : m_property(nullptr), m_propertyClass(nullptr), m_fieldEditor(nullptr), m_valueKind(VALUEKIND_Uninitialized), m_koq(nullptr)
            {}
        ECPropertiesFieldKey(ECPropertyCR property, ECClassCR propertyClass, RelatedClassPathCR path, Utf8String fieldLabel, Utf8String categoryName, ContentFieldEditor const* editor)
            : m_property(&property), m_propertyClass(&propertyClass), m_pathFromSelectToPropertyClass(path), m_fieldLabel(fieldLabel), m_categoryName(categoryName), m_fieldEditor(editor)
            {
            m_valueKind = m_property->GetIsPrimitive() ? VALUEKIND_Primitive
                : m_property->GetIsNavigation() ? VALUEKIND_Navigation
                : m_property->GetIsArray() ? VALUEKIND_Array
                : m_property->GetIsStruct() ? VALUEKIND_Struct
                : VALUEKIND_Uninitialized;
            m_type = m_property->GetTypeName();
            m_koq = m_property->GetKindOfQuantity();
            }
        ECPropertiesFieldKey(Property const& prop, Utf8String fieldLabel, Utf8String categoryName, ContentFieldEditor const* editor)
            : ECPropertiesFieldKey(prop.GetProperty(), prop.GetPropertyClass(), prop.GetPathFromSelectToPropertyClass(), fieldLabel, categoryName, editor)
            {}
        bool operator<(ECPropertiesFieldKey const& rhs) const;
        ValueKind GetValueKind() const {return m_valueKind;}
        Utf8CP GetName() const {return m_property->GetName().c_str();}
        Utf8CP GetLabel() const {return m_fieldLabel.c_str();}
        Utf8CP GetType() const {return m_type.c_str();}
        KindOfQuantityCP GetKoq() const {return m_koq;}
        bool IsRelated() const {return !m_pathFromSelectToPropertyClass.empty();}
        ECClassCP GetClass() const {return m_propertyClass;}
        Utf8StringCR GetCategoryName() const {return m_categoryName;}
        ContentFieldEditor const* GetEditor() const {return m_fieldEditor;}
    };

private:
    Utf8String m_preferredDisplayType;
    bvector<SelectClassInfo> m_classes;
    bvector<Field*> m_fields;
    bmap<ECPropertiesFieldKey, ECPropertiesField*> m_fieldsMap;
    bvector<ECInstanceKeyField*> m_keyFields;
    int m_sortingFieldIndex;
    SortDirection m_sortDirection;
    int m_contentFlags;
    Utf8String m_filterExpression;
    Utf8String m_connectionId;
    INavNodeKeysContainerCPtr m_inputKeys;
    SelectionInfoCPtr m_selectionInfo;
    Json::Value m_options;

private:
    ECPRESENTATION_EXPORT ContentDescriptor(IConnectionCR connection, JsonValueCR options, INavNodeKeysContainerCR input, Utf8String preferredDisplayType);
    ECPRESENTATION_EXPORT ContentDescriptor(ContentDescriptorCR other);
    ECPRESENTATION_EXPORT int GetFieldIndex(Utf8CP name) const;
    void OnFlagAdded(ContentFlags flag);
    void OnFlagRemoved(ContentFlags flag);
    void OnECPropertiesFieldRemoved(ECPropertiesField const& field);
    void OnECPropertiesFieldAdded(ECPropertiesField& field);

protected:
    ECPRESENTATION_EXPORT ~ContentDescriptor();

public:
    //! Creates a content descriptor.
    //! @param[in] connection The connection to create the descriptor for.
    //! @param[in] options The options to create the descriptor with.
    //! @param[in] input The input to create the descriptor for.
    //! @param[in] preferredDisplayType The display type to create the descriptor for.
    static ContentDescriptorPtr Create(IConnectionCR connection, JsonValueCR options, INavNodeKeysContainerCR input, Utf8CP preferredDisplayType = ContentDisplayType::Undefined)
        {return new ContentDescriptor(connection, options, input, preferredDisplayType);}

    //! Copies the supplied content descriptor.
    //! @param[in] other The descriptor to copy.
    static ContentDescriptorPtr Create(ContentDescriptorCR other) {return new ContentDescriptor(other);}

    //! Serializes this descriptor to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;

    //! Is this desciptor equal to the supplied one.
    ECPRESENTATION_EXPORT bool Equals(ContentDescriptorCR other) const;
    //! Is this desciptor equal to the supplied one.
    bool operator==(ContentDescriptorCR other) const {return Equals(other);}
    //! Is this desciptor not equal to the supplied one.
    bool operator!=(ContentDescriptorCR other) const {return !Equals(other);}

    //! Merge this descriptor with the supplied one.
    ECPRESENTATION_EXPORT void MergeWith(ContentDescriptorCR other);

    //! Get the preferred display type which this descriptor is created for.
    Utf8StringCR GetPreferredDisplayType() const {return m_preferredDisplayType;}
    //! Get node keys which this descriptor is created for.
    INavNodeKeysContainerCR GetInputNodeKeys() const {return *m_inputKeys;}
    //! Get connection ID which this descriptor is created for.
    Utf8StringCR GetConnectionId() const {return m_connectionId;}
    //! Get content options which this descriptor is created using.
    JsonValueCR GetOptions() const {return m_options;}
    //! Get selection info which this descriptor is created with. (returns nullptr if no selection info was provided)
    SelectionInfo const* GetSelectionInfo() const {return m_selectionInfo.get();}
    //! Set selection info which this descriptor is created with.
    void SetSelectionInfo(SelectionInfo const& selectionInfo) {m_selectionInfo = &selectionInfo;}

    //! Get information about ECClasses which the descriptor consists from.
    bvector<SelectClassInfo> const& GetSelectClasses() const {return m_classes;}
    //! Get information about ECClasses which the descriptor consists from.
    bvector<SelectClassInfo>& GetSelectClasses() {return m_classes;}

    //! Get the fields in this descriptor (excluding system ones).
    ECPRESENTATION_EXPORT bvector<Field*> GetVisibleFields() const;
    //! Get the fields in this descriptor (including system ones).
    bvector<Field*> const& GetAllFields() const {return m_fields;}

    //! Returns a field if descriptor has DistinctValues flag set, has a single field and that field qualifies to be used with DistinctValues flag. Returns nullptr otherwise.
    Field const* GetDistinctField() const;

    //! Get the ECInstance key fields in this descriptor.
    bvector<ECInstanceKeyField*> const& GetECInstanceKeyFields() const { return m_keyFields; }
    //! Add field to this descriptor.
    ECPRESENTATION_EXPORT void AddField(Field* field);
    //! Find ECProperties field in this descriptor by property.
    ECPropertiesField* FindECPropertiesField(Property const& prop, Utf8StringCR fieldLabel, Category const& category, ContentFieldEditor const* editor) {return FindECPropertiesField(prop.GetProperty(), prop.GetPropertyClass(), prop.GetPathFromSelectToPropertyClass(), fieldLabel, category, editor);}
    //! Find ECProperties field in this descriptor by property.
    ECPropertiesField* FindECPropertiesField(ECPropertyCR prop, ECClassCR propClass, RelatedClassPathCR pathFromSelectToPropertyClass, Utf8StringCR fieldLabel, Category const& category, ContentFieldEditor const* editor);

    //! Remove a field from this descriptor.
    ECPRESENTATION_EXPORT void RemoveField(Field const& field);
    //! Remove a field from this descriptor by index.
    void RemoveField(size_t index) {RemoveField(*m_fields[index]);}
    //! Remove a field from this descriptor by name.
    ECPRESENTATION_EXPORT void RemoveField(Utf8CP name);

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

    //! Set filtering ECExpression.
    void SetFilterExpression(Utf8String expression) {m_filterExpression = expression;}
    //! Get filtering ECExpression.
    Utf8StringCR GetFilterExpression() const {return m_filterExpression;}

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
    bool ShowImages() const {return HasContentFlag(ContentFlags::ShowImages);}
    //! Should content include display labels.
    bool ShowLabels() const {return HasContentFlag(ContentFlags::ShowLabels);}
    //! Should the content be merged into a single record.
    bool MergeResults() const {return HasContentFlag(ContentFlags::MergeResults);}
    //! Should only distinct values be returned
    bool OnlyDistinctValues() const {return HasContentFlag(ContentFlags::DistinctValues);}
};

//=======================================================================================
//! A struct that represents a single content record.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
struct ContentSetItem : RefCountedBase, RapidJsonExtendedDataHolder<>
{
    //===================================================================================
    //! Describes ContentSetItem parts which may be serialized.
    // @bsiclass                                    Grigas.Petraitis            07/2017
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
        SERIALIZE_All           = ((1 << 9) - 1),
        };

    //===================================================================================
    //! Data structure containing a pair of ECPropertiesField and an index of a property
    //! in that field.
    // @bsiclass                                    Grigas.Petraitis            06/2017
    //===================================================================================
    struct FieldProperty
    {
    private:
        ContentDescriptor::ECPropertiesField const* m_field;
        size_t m_propertyIndex;
    public:
        FieldProperty() : m_field(nullptr), m_propertyIndex(0) {}
        FieldProperty(ContentDescriptor::ECPropertiesField const& field, size_t propertyIndex)
            : m_field(&field), m_propertyIndex(propertyIndex)
            {}
        bool operator<(FieldProperty const& other) const
            {
            if (m_propertyIndex != other.m_propertyIndex)
                return m_propertyIndex < other.m_propertyIndex;
            return m_field->GetUniqueName().CompareTo(other.m_field->GetUniqueName()) < 0;
            }
        ContentDescriptor::ECPropertiesField const& GetField() const {return *m_field;}
        ContentDescriptor::Property const& GetProperty() const {return m_field->GetProperties()[m_propertyIndex];}
        size_t GetPropertyIndex() const {return m_propertyIndex;}
    };
    typedef bmap<FieldProperty, bvector<ECClassInstanceKey>> FieldPropertyInstanceKeyMap;

private:
    ECClassCP m_class;
    bvector<ECClassInstanceKey> m_keys;
    LabelDefinitionCPtr m_displayLabelDefinition;
    Utf8String m_imageId;
    rapidjson::Document m_values;
    rapidjson::Document m_displayValues;
    rapidjson::Document m_extendedData;
    FieldPropertyInstanceKeyMap m_fieldPropertyInstanceKeys;
    bvector<Utf8String> m_mergedFieldNames;

private:
    ContentSetItem(bvector<ECClassInstanceKey> keys, LabelDefinitionCR displayLabelDefinition, Utf8String imageId, rapidjson::Document&& values,
        rapidjson::Document&& displayValues, bvector<Utf8String> mergedFieldNames, FieldPropertyInstanceKeyMap&& fieldPropertyInstanceKeys)
        : m_class(nullptr), m_keys(keys), m_displayLabelDefinition(&displayLabelDefinition), m_imageId(imageId),
        m_values(std::move(values)), m_displayValues(std::move(displayValues)), m_extendedData(rapidjson::kObjectType),
        m_mergedFieldNames(mergedFieldNames), m_fieldPropertyInstanceKeys(std::move(fieldPropertyInstanceKeys))
        {}
//__PUBLISH_SECTION_END__
public:
    rapidjson::Document& GetValues() {return m_values;}
    rapidjson::Document const& GetValues() const { return m_values; }
    rapidjson::Document& GetDisplayValues() {return m_displayValues;}
    rapidjson::Document const& GetDisplayValues() const { return m_displayValues; }
    rapidjson::Document& GetExtendedData() { return m_extendedData; }
    bvector<Utf8String>& GetMergedFieldNames() {return m_mergedFieldNames;}
    bvector<ECClassInstanceKey>& GetKeys() {return m_keys;}
    FieldPropertyInstanceKeyMap const& GetFieldInstanceKeys() const {return m_fieldPropertyInstanceKeys;}
//__PUBLISH_SECTION_START__

public:
    //! Creates a @ref ContentSetItem.
    //! @param[in] keys The keys which describe whose values this item contains.
    //! @param[in] displayLabelDefinition The definition of this content item label.
    //! @param[in] imageId The image ID for this item.
    //! @param[in] values The values map.
    //! @param[in] displayValues The display values map.
    //! @param[in] mergedFieldNames Names of merged fields in this record.
    //! @param[in] fieldPropertyInstanceKeys ECClassInstanceKeys of related instances for each field in this record.
    static ContentSetItemPtr Create(bvector<ECClassInstanceKey> keys, LabelDefinitionCR displayLabelDefinition, Utf8String imageId,
        rapidjson::Document&& values, rapidjson::Document&& displayValues, bvector<Utf8String> mergedFieldNames,
        FieldPropertyInstanceKeyMap&& fieldPropertyInstanceKeys)
        {
        return new ContentSetItem(keys, displayLabelDefinition, imageId, std::move(values), std::move(displayValues),
            mergedFieldNames, std::move(fieldPropertyInstanceKeys));
        }

    //! Serialize this item to JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(int flags = SERIALIZE_All, rapidjson::Document::AllocatorType* allocator = nullptr) const;

    //! Get user's extended data map
    RapidJsonAccessor GetUsersExtendedData() const {return RapidJsonAccessor(m_extendedData);}
    ECPRESENTATION_EXPORT void AddUsersExtendedData(Utf8CP key, ECValueCR value);

    //! Get keys of ECInstances whose values this item contains.
    bvector<ECClassInstanceKey> const& GetKeys() const {return m_keys;}

    //! Get names of merged fields in this record.
    bvector<Utf8String> const& GetMergedFieldNames() const {return m_mergedFieldNames;}

    //! Are the values of field with the specified name merged in this record.
    ECPRESENTATION_EXPORT bool IsMerged(Utf8StringCR fieldName) const;

    //! Get the ECInstance keys whose values are contained in the field with the specified name.
    ECPRESENTATION_EXPORT bvector<ECClassInstanceKey> const& GetPropertyValueKeys(FieldProperty const&) const;

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
// @bsiclass                                    Grigas.Petraitis                04/2016
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
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! Holds information about a single changed instance.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                06/2017
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
// @bsiclass                                    Grigas.Petraitis                06/2017
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
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! An interface for a ECProperty value formatter.
//! @note Formatter methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                09/2016
//=======================================================================================
struct IECPropertyFormatter
{
protected:
    //! @see GetFormattedPropertyValue
    virtual BentleyStatus _GetFormattedPropertyValue(Utf8StringR, ECPropertyCR, ECValueCR) const = 0;
    virtual BentleyStatus _GetFormattedPropertyLabel(Utf8StringR, ECPropertyCR, ECClassCR, RelatedClassPath const&, RelationshipMeaning) const = 0;

public:
    //! Virtual destructor.
    virtual ~IECPropertyFormatter() {}

    //! Formats the supplied ECValue.
    //! @param[out] formattedValue The formatted value.
    //! @param[in] ecProperty The property whose value is being formatted.
    //! @param[in] ecValue The value to format.
    //! @return SUCCESS if the value was successfully formatted.
    BentleyStatus GetFormattedPropertyValue(Utf8StringR formattedValue, ECPropertyCR ecProperty, ECValueCR ecValue) const
        {
        return _GetFormattedPropertyValue(formattedValue, ecProperty, ecValue);
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
};

//=======================================================================================
//! An interface for a ECProperty value formatter.
//! @note Formatter methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                06/2017
//=======================================================================================
struct DefaultPropertyFormatter : IECPropertyFormatter
{
protected:
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyEnumFormatting(Utf8StringR, ECPropertyCR, ECValueCR) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyKoqFormatting(Utf8StringR, ECPropertyCR, ECValueCR) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyPoint3dFormatting(Utf8StringR, DPoint3dCR) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyPoint2dFormatting(Utf8StringR, DPoint2dCR) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyDoubleFormatting(Utf8StringR, double) const;
    ECPRESENTATION_EXPORT virtual BentleyStatus _ApplyDateTimeFormatting(Utf8StringR, DateTimeCR) const;
protected:
    ECPRESENTATION_EXPORT virtual BentleyStatus _GetFormattedPropertyValue(Utf8StringR, ECPropertyCR, ECValueCR) const override;
    ECPRESENTATION_EXPORT virtual BentleyStatus _GetFormattedPropertyLabel(Utf8StringR, ECPropertyCR, ECClassCR, RelatedClassPath const&, RelationshipMeaning) const override;
public:
    DefaultPropertyFormatter() {}
};

//=======================================================================================
//! Interface for a category supplier which is used to determine categories for
//! @ref ContentDescriptor::Field.
//! @note Supplied methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                10/2016
//=======================================================================================
struct IPropertyCategorySupplier
{
protected:
    virtual ContentDescriptor::Category _GetECClassCategory(ECClassCR) const = 0;
    virtual ContentDescriptor::Category _GetRelatedECClassCategory(ECClassCR, RelationshipMeaning) const = 0;
    virtual ContentDescriptor::Category _GetPropertyCategory(ECPropertyCR) const = 0;
    virtual ContentDescriptor::Category _CreateCategory(ECClassCR, ECPropertyCR, RelationshipMeaning) const = 0;

public:
    //! Virtual destructor.
    virtual ~IPropertyCategorySupplier() {}

    //! Called to create a category based on specific ECClass
    ContentDescriptor::Category GetECClassCategory(ECClassCR ecClass) const {return _GetECClassCategory(ecClass);}

    //! Called to create a category based on specific ECClass
    ContentDescriptor::Category GetRelatedECClassCategory(ECClassCR ecClass, RelationshipMeaning meaning) const {return _GetRelatedECClassCategory(ecClass, meaning);}

    //! Called to create a category based on ECProperty's category. Should return invalid category if property is not categorized.
    ContentDescriptor::Category GetPropertyCategory(ECPropertyCR ecProperty) const {return _GetPropertyCategory(ecProperty);}

    //! Called to create a category for a property based on exact ECClass it's used on and the specified relationship meaning
    ContentDescriptor::Category CreateCategory(ECClassCR actualClass, ECPropertyCR ecProperty, RelationshipMeaning meaning) const {return _CreateCategory(actualClass, ecProperty, meaning);}
};

//=======================================================================================
//! Default property category supplier that uses the "Category" custom attribute to determine
//! the ECProperty category. If no such custom attribute exists, the "Miscellaneous" category
//! is returned.
//! @ingroup GROUP_Presentation_Content
// @bsiclass                                    Grigas.Petraitis                10/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DefaultCategorySupplier : IPropertyCategorySupplier
{
protected:
    ECPRESENTATION_EXPORT virtual ContentDescriptor::Category _GetECClassCategory(ECClassCR) const override;
    ECPRESENTATION_EXPORT virtual ContentDescriptor::Category _GetRelatedECClassCategory(ECClassCR, RelationshipMeaning) const override;
    ECPRESENTATION_EXPORT virtual ContentDescriptor::Category _GetPropertyCategory(ECPropertyCR) const override;
    ECPRESENTATION_EXPORT virtual ContentDescriptor::Category _CreateCategory(ECClassCR, ECPropertyCR, RelationshipMeaning) const override;
public:
    DefaultCategorySupplier() {}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
